#include "render/vfx.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <memory>

#include "raymath.h"
#include "rlgl.h"

namespace racer {
namespace {

constexpr int kMaxParticles = 4096;
constexpr float kGroundY = 0.0f;          // sol suppose plan a y=0 (rebonds, impacts pluie)
constexpr float kRainRadius = 40.0f;      // rayon du cylindre de spawn autour du focus
constexpr float kRainFallSpeed = -22.0f;
constexpr int kMaxImpactsParFrame = 64;   // impacts de pluie traites par frame (borne fixe)

// Types internes : chaque type choisit texture, passe de blend et comportement.
enum class PType : std::uint8_t { DriftSmoke, OffroadDust, NitroFlame, Spark, Confetti, Rain, Splash };

struct Particle {
    PType type = PType::DriftSmoke;
    Vector3 pos{0.0f, 0.0f, 0.0f};
    Vector3 vel{0.0f, 0.0f, 0.0f};
    float life = 0.0f;       // secondes restantes
    float maxLife = 1.0f;
    float size = 1.0f;       // largeur monde (billboards) ou largeur du streak
    float sizeGrowth = 0.0f; // d(size)/dt
    float rot = 0.0f;        // degres, rotation dans le plan du billboard
    float rotVel = 0.0f;
    float phase = 0.0f;      // dephasage sinus (balancement/retournement confetti)
    Color colorStart{255, 255, 255, 255};
    Color colorEnd{255, 255, 255, 0};
};

constexpr Color Rgba(int r, int g, int b, int a) {
    return Color{static_cast<unsigned char>(r), static_cast<unsigned char>(g),
                 static_cast<unsigned char>(b), static_cast<unsigned char>(a)};
}

float Frand(float lo, float hi) {
    return lo + (hi - lo) * (static_cast<float>(GetRandomValue(0, 16383)) / 16383.0f);
}

Color LerpColor(Color a, Color b, float t) {
    auto ch = [t](unsigned char ca, unsigned char cb) {
        float v = static_cast<float>(ca) + (static_cast<float>(cb) - static_cast<float>(ca)) * t;
        return static_cast<unsigned char>(std::clamp(v, 0.0f, 255.0f));
    };
    return Color{ch(a.r, b.r), ch(a.g, b.g), ch(a.b, b.b), ch(a.a, b.a)};
}

// -- Textures procedurales (aucun fichier sur disque) ------------------------

void FinishTexture(Texture2D& tex) {
    GenTextureMipmaps(&tex);
    SetTextureFilter(tex, TEXTURE_FILTER_TRILINEAR);
    SetTextureWrap(tex, TEXTURE_WRAP_CLAMP);
}

// Puff radial doux blanc -> transparent (fumee, poussiere, flammes).
Texture2D MakePuffTexture() {
    Image img = GenImageGradientRadial(64, 64, 0.25f, Rgba(255, 255, 255, 255), Rgba(255, 255, 255, 0));
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    FinishTexture(tex);
    return tex;
}

// Streak vertical fin a bords doux (pluie, etincelles, splash).
Texture2D MakeStreakTexture() {
    Image img = GenImageColor(16, 64, BLANK);
    for (int y = 0; y < 64; ++y) {
        for (int x = 0; x < 16; ++x) {
            float u = (static_cast<float>(x) + 0.5f) / 16.0f * 2.0f - 1.0f; // -1..1 horizontal
            float v = (static_cast<float>(y) + 0.5f) / 64.0f;               // 0..1 vertical
            float bell = std::max(0.0f, 1.0f - std::fabs(u));
            bell *= bell;                                                    // cloche fine
            float ends = std::clamp(std::min(v, 1.0f - v) * 8.0f, 0.0f, 1.0f); // fondu aux extremites
            ImageDrawPixel(&img, x, y, Rgba(255, 255, 255, static_cast<int>(bell * ends * 255.0f)));
        }
    }
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    FinishTexture(tex);
    return tex;
}

// Petit carre plein (confettis).
Texture2D MakeSquareTexture() {
    Image img = GenImageColor(8, 8, Rgba(255, 255, 255, 255));
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    FinishTexture(tex);
    return tex;
}

// -- Rendu ---------------------------------------------------------------

struct CamBasis {
    Vector3 right;
    Vector3 up;
    Vector3 fwd;
};

CamBasis MakeCamBasis(const Camera3D& cam) {
    Matrix v = MatrixLookAt(cam.position, cam.target, cam.up);
    CamBasis b;
    b.right = Vector3{v.m0, v.m4, v.m8};
    b.up = Vector3{v.m1, v.m5, v.m9};
    b.fwd = Vector3Normalize(Vector3Subtract(cam.target, cam.position));
    return b;
}

// Quad texture centre en c, demi-axes hr/hu (batch rlgl, culling desactive).
void EmitQuad(const Texture2D& tex, Vector3 c, Vector3 hr, Vector3 hu, Color tint) {
    rlCheckRenderBatchLimit(4);
    rlSetTexture(tex.id);
    rlBegin(RL_QUADS);
    rlColor4ub(tint.r, tint.g, tint.b, tint.a);
    rlTexCoord2f(0.0f, 1.0f);
    rlVertex3f(c.x - hr.x - hu.x, c.y - hr.y - hu.y, c.z - hr.z - hu.z);
    rlTexCoord2f(1.0f, 1.0f);
    rlVertex3f(c.x + hr.x - hu.x, c.y + hr.y - hu.y, c.z + hr.z - hu.z);
    rlTexCoord2f(1.0f, 0.0f);
    rlVertex3f(c.x + hr.x + hu.x, c.y + hr.y + hu.y, c.z + hr.z + hu.z);
    rlTexCoord2f(0.0f, 0.0f);
    rlVertex3f(c.x - hr.x + hu.x, c.y - hr.y + hu.y, c.z - hr.z + hu.z);
    rlEnd();
}

// Dessine toutes les particules d'un type donne (groupe = 1 texture, peu de
// switchs dans le batch rlgl). Couleur = lerp(start, end, 1 - life/maxLife).
void DrawParticlesOfType(const Particle* pool, int count, PType type, const Texture2D& tex, const CamBasis& cb) {
    for (int i = 0; i < count; ++i) {
        const Particle& p = pool[i];
        if (p.type != type) continue;
        float t = 1.0f - p.life / p.maxLife;
        Color col = LerpColor(p.colorStart, p.colorEnd, t);

        Vector3 hr{0.0f, 0.0f, 0.0f};
        Vector3 hu{0.0f, 0.0f, 0.0f};
        switch (p.type) {
            case PType::DriftSmoke:
            case PType::OffroadDust:
            case PType::NitroFlame: {
                // Billboard plein face camera, tourne de rot autour de la normale.
                float half = p.size * 0.5f;
                float cr = std::cos(p.rot * DEG2RAD);
                float sr = std::sin(p.rot * DEG2RAD);
                hr = Vector3Add(Vector3Scale(cb.right, cr * half), Vector3Scale(cb.up, sr * half));
                hu = Vector3Add(Vector3Scale(cb.right, -sr * half), Vector3Scale(cb.up, cr * half));
            } break;
            case PType::Confetti: {
                // Face camera + rotation, largeur oscillante = retournement du papier.
                float age = p.maxLife - p.life;
                float flip = std::sin(p.phase + age * 7.0f);
                float halfW = p.size * 0.5f * flip;
                float halfH = p.size * 0.5f;
                float cr = std::cos(p.rot * DEG2RAD);
                float sr = std::sin(p.rot * DEG2RAD);
                hr = Vector3Add(Vector3Scale(cb.right, cr * halfW), Vector3Scale(cb.up, sr * halfW));
                hu = Vector3Add(Vector3Scale(cb.right, -sr * halfH), Vector3Scale(cb.up, cr * halfH));
            } break;
            case PType::Spark:
            case PType::Rain:
            case PType::Splash: {
                // Streak etire le long de la vitesse.
                Vector3 dir = p.vel;
                float speed = Vector3Length(dir);
                dir = (speed > 0.001f) ? Vector3Scale(dir, 1.0f / speed) : Vector3{0.0f, 1.0f, 0.0f};
                float halfLen = 0.7f; // pluie
                if (p.type == PType::Spark) halfLen = std::clamp(speed * 0.035f, 0.08f, 0.35f);
                else if (p.type == PType::Splash) halfLen = 0.09f;
                hu = Vector3Scale(dir, halfLen);
                Vector3 side = Vector3CrossProduct(dir, cb.fwd);
                float sideLen = Vector3Length(side);
                side = (sideLen > 0.001f) ? Vector3Scale(side, 1.0f / sideLen) : cb.right;
                hr = Vector3Scale(side, p.size * 0.5f);
            } break;
        }
        EmitQuad(tex, p.pos, hr, hu, col);
    }
}

} // namespace

// -- Etat interne -------------------------------------------------------------

struct VfxSystem::Impl {
    std::array<Particle, kMaxParticles> pool{};
    int count = 0;

    Texture2D texPuff{};
    Texture2D texStreak{};
    Texture2D texSquare{};

    float rainIntensity = 0.0f; // 0..1, lissee
    float rainTarget = 0.0f;
    float rainAccum = 0.0f;     // accumulateur fractionnaire de spawn

    Particle* Alloc() {
        if (count >= kMaxParticles) return nullptr; // pool plein : on ignore
        return &pool[static_cast<std::size_t>(count++)];
    }

    void SpawnRainDrop(Vector3 focus) {
        Particle* p = Alloc();
        if (p == nullptr) return;
        float ang = Frand(0.0f, 2.0f * PI);
        float rad = kRainRadius * std::sqrt(Frand(0.0f, 1.0f)); // uniforme sur le disque
        p->type = PType::Rain;
        p->pos = Vector3{focus.x + std::cos(ang) * rad, focus.y + Frand(12.0f, 18.0f), focus.z + std::sin(ang) * rad};
        p->vel = Vector3{Frand(-1.8f, 1.8f), kRainFallSpeed + Frand(-2.0f, 2.0f), Frand(-1.8f, 1.8f)};
        p->maxLife = p->life = 1.4f; // tuee avant par l'impact sol
        p->size = Frand(0.1f, 0.14f);
        p->sizeGrowth = 0.0f;
        p->rot = 0.0f;
        p->rotVel = 0.0f;
        p->phase = 0.0f;
        p->colorStart = Rgba(190, 212, 240, 205);
        p->colorEnd = Rgba(195, 216, 242, 170);
    }

    void SpawnSplash(Vector3 at) {
        // Deux micro-streaks quasi horizontaux, tres courts.
        for (int i = 0; i < 2; ++i) {
            Particle* p = Alloc();
            if (p == nullptr) return;
            float ang = Frand(0.0f, 2.0f * PI);
            float sp = Frand(0.9f, 1.9f);
            p->type = PType::Splash;
            p->pos = Vector3{at.x, kGroundY + 0.03f, at.z};
            p->vel = Vector3{std::cos(ang) * sp, Frand(0.15f, 0.4f), std::sin(ang) * sp};
            p->maxLife = p->life = 0.1f;
            p->size = 0.05f;
            p->sizeGrowth = 0.0f;
            p->rot = 0.0f;
            p->rotVel = 0.0f;
            p->phase = 0.0f;
            p->colorStart = Rgba(190, 210, 235, 150);
            p->colorEnd = Rgba(195, 215, 240, 0);
        }
    }
};

// -- API publique ------------------------------------------------------------

VfxSystem::VfxSystem() : impl_(std::make_unique<Impl>()) {
    // Necessite un contexte OpenGL actif (a construire apres InitWindow).
    impl_->texPuff = MakePuffTexture();
    impl_->texStreak = MakeStreakTexture();
    impl_->texSquare = MakeSquareTexture();
}

VfxSystem::~VfxSystem() {
    UnloadTexture(impl_->texPuff);
    UnloadTexture(impl_->texStreak);
    UnloadTexture(impl_->texSquare);
}

void VfxSystem::Update(float dt, Vector3 focus) {
    Impl& s = *impl_;
    if (dt <= 0.0f) return;
    dt = std::min(dt, 0.1f);

    // Intensite de pluie lissee vers la cible (~2 s pour 0 -> 1).
    float step = 0.5f * dt;
    if (s.rainIntensity < s.rainTarget) s.rainIntensity = std::min(s.rainTarget, s.rainIntensity + step);
    else if (s.rainIntensity > s.rainTarget) s.rainIntensity = std::max(s.rainTarget, s.rainIntensity - step);

    // Spawn continu de gouttes (~300 * intensite / s) autour du focus.
    if (s.rainIntensity > 0.001f) {
        s.rainAccum += 300.0f * s.rainIntensity * dt;
        int n = std::min(static_cast<int>(s.rainAccum), 64);
        s.rainAccum -= static_cast<float>(n);
        for (int i = 0; i < n; ++i) s.SpawnRainDrop(focus);
    } else {
        s.rainAccum = 0.0f;
    }

    // Impacts de pluie collectes pendant l'integration, spawns differes (borne fixe).
    std::array<Vector3, kMaxImpactsParFrame> impacts{};
    int impactCount = 0;

    // O(n) : integre, tue et recycle par swap avec la fin (l'element permute
    // vient de la queue et n'a pas encore ete visite).
    for (int i = 0; i < s.count;) {
        Particle& p = s.pool[static_cast<std::size_t>(i)];
        p.life -= dt;
        bool kill = p.life <= 0.0f;

        if (!kill) {
            switch (p.type) {
                case PType::DriftSmoke: {
                    float d = std::max(0.0f, 1.0f - 1.7f * dt); // trainee d'air
                    p.vel.x *= d;
                    p.vel.z *= d;
                    p.vel.y += 0.5f * dt; // legere montee
                } break;
                case PType::OffroadDust: {
                    float d = std::max(0.0f, 1.0f - 2.4f * dt);
                    p.vel.x *= d;
                    p.vel.z *= d;
                    p.vel.y -= 2.5f * dt; // retombe, reste bas
                } break;
                case PType::NitroFlame: {
                    float d = std::max(0.0f, 1.0f - 5.0f * dt); // jet court et nerveux
                    p.vel.x *= d;
                    p.vel.y *= d;
                    p.vel.z *= d;
                } break;
                case PType::Spark:
                    p.vel.y -= 25.0f * dt; // gravite forte
                    break;
                case PType::Confetti: {
                    p.vel.y -= 3.5f * dt;
                    if (p.vel.y < -1.5f) p.vel.y = -1.5f; // chute lente terminale
                    float age = p.maxLife - p.life;
                    float d = std::max(0.0f, 1.0f - 0.8f * dt);
                    p.vel.x = p.vel.x * d + std::cos(age * 2.6f + p.phase) * 2.2f * dt; // balancement
                    p.vel.z = p.vel.z * d + std::sin(age * 2.1f + p.phase * 1.7f) * 2.2f * dt;
                } break;
                case PType::Rain:
                case PType::Splash:
                    break;
            }

            p.pos.x += p.vel.x * dt;
            p.pos.y += p.vel.y * dt;
            p.pos.z += p.vel.z * dt;

            if (p.type == PType::Spark && p.pos.y < kGroundY) {
                p.pos.y = kGroundY; // rebond simple
                p.vel.y *= -0.4f;
                p.vel.x *= 0.75f;
                p.vel.z *= 0.75f;
                p.life *= 0.6f; // vie reduite a chaque rebond
            } else if (p.type == PType::OffroadDust && p.pos.y < 0.05f) {
                p.pos.y = 0.05f;
                if (p.vel.y < 0.0f) p.vel.y = 0.0f;
            } else if (p.type == PType::Confetti && p.pos.y < 0.03f) {
                p.pos.y = 0.03f; // pose au sol, s'eteint par le fondu alpha
                p.vel = Vector3{0.0f, 0.0f, 0.0f};
            } else if (p.type == PType::Rain && p.pos.y <= kGroundY) {
                if (impactCount < kMaxImpactsParFrame) impacts[static_cast<std::size_t>(impactCount++)] = p.pos;
                kill = true;
            }

            p.rot += p.rotVel * dt;
            p.size = std::max(0.02f, p.size + p.sizeGrowth * dt);
        }

        if (kill) {
            s.pool[static_cast<std::size_t>(i)] = s.pool[static_cast<std::size_t>(--s.count)];
            continue;
        }
        ++i;
    }

    for (int i = 0; i < impactCount; ++i) {
        s.SpawnSplash(Vector3{impacts[static_cast<std::size_t>(i)].x, kGroundY, impacts[static_cast<std::size_t>(i)].z});
    }
}

void VfxSystem::Draw(const Camera3D& camera) const {
    const Impl& s = *impl_;
    if (s.count == 0) return;

    CamBasis cb = MakeCamBasis(camera);

    // Flush de la scene opaque avant de changer l'etat GL : profondeur en
    // lecture seule (pas d'ecriture) et culling off (quads orientes librement).
    rlDrawRenderBatchActive();
    rlDisableDepthMask();
    rlDisableBackfaceCulling();

    // Passe 1 : alpha classique (fumee, poussiere, confettis, pluie, splash).
    BeginBlendMode(BLEND_ALPHA);
    DrawParticlesOfType(s.pool.data(), s.count, PType::DriftSmoke, s.texPuff, cb);
    DrawParticlesOfType(s.pool.data(), s.count, PType::OffroadDust, s.texPuff, cb);
    DrawParticlesOfType(s.pool.data(), s.count, PType::Confetti, s.texSquare, cb);
    DrawParticlesOfType(s.pool.data(), s.count, PType::Splash, s.texStreak, cb);
    DrawParticlesOfType(s.pool.data(), s.count, PType::Rain, s.texStreak, cb);

    // Passe 2 : additif (flammes nitro, etincelles) -- le switch flush le batch.
    BeginBlendMode(BLEND_ADDITIVE);
    DrawParticlesOfType(s.pool.data(), s.count, PType::NitroFlame, s.texPuff, cb);
    DrawParticlesOfType(s.pool.data(), s.count, PType::Spark, s.texStreak, cb);
    EndBlendMode();

    // Restaure l'etat GL pour la suite de la scene.
    rlDrawRenderBatchActive();
    rlEnableBackfaceCulling();
    rlEnableDepthMask();
}

void VfxSystem::EmitDriftSmoke(Vector3 pos, Vector3 carVel) {
    int n = GetRandomValue(2, 3);
    for (int i = 0; i < n; ++i) {
        Particle* p = impl_->Alloc();
        if (p == nullptr) return;
        p->type = PType::DriftSmoke;
        p->pos = Vector3{pos.x + Frand(-0.18f, 0.18f), std::max(0.08f, pos.y + Frand(-0.05f, 0.12f)),
                         pos.z + Frand(-0.18f, 0.18f)};
        p->vel = Vector3{carVel.x * 0.25f + Frand(-0.9f, 0.9f), carVel.y * 0.25f + Frand(0.6f, 1.5f),
                         carVel.z * 0.25f + Frand(-0.9f, 0.9f)};
        p->maxLife = p->life = Frand(0.8f, 1.4f);
        p->size = Frand(0.4f, 0.6f);
        p->sizeGrowth = Frand(1.3f, 2.1f); // grossit fort
        p->rot = Frand(0.0f, 360.0f);
        p->rotVel = Frand(-80.0f, 80.0f);
        p->phase = 0.0f;
        p->colorStart = Rgba(192, 192, 198, 110); // gris clair, peu opaque (les puffs se cumulent)
        p->colorEnd = Rgba(225, 225, 232, 0);
    }
}

void VfxSystem::EmitOffroadDust(Vector3 pos, Vector3 carVel) {
    int n = GetRandomValue(1, 2);
    for (int i = 0; i < n; ++i) {
        Particle* p = impl_->Alloc();
        if (p == nullptr) return;
        p->type = PType::OffroadDust;
        p->pos = Vector3{pos.x + Frand(-0.25f, 0.25f), std::max(0.06f, pos.y), pos.z + Frand(-0.25f, 0.25f)};
        p->vel = Vector3{carVel.x * 0.2f + Frand(-1.2f, 1.2f), Frand(0.15f, 0.6f), carVel.z * 0.2f + Frand(-1.2f, 1.2f)};
        p->maxLife = p->life = Frand(0.35f, 0.7f); // vie courte
        p->size = Frand(0.3f, 0.5f);
        p->sizeGrowth = Frand(1.0f, 1.6f);
        p->rot = Frand(0.0f, 360.0f);
        p->rotVel = Frand(-60.0f, 60.0f);
        p->phase = 0.0f;
        p->colorStart = Rgba(206, 186, 140, 82); // sable, moins opaque que la fumee
        p->colorEnd = Rgba(214, 198, 164, 0);
    }
}

void VfxSystem::EmitNitroFlame(Vector3 pos, Vector3 backDir, Vector3 carVel) {
    int n = GetRandomValue(4, 5);
    for (int i = 0; i < n; ++i) {
        Particle* p = impl_->Alloc();
        if (p == nullptr) return;
        float eject = Frand(8.0f, 14.0f);
        p->type = PType::NitroFlame;
        p->pos = Vector3{pos.x + backDir.x * Frand(0.0f, 0.15f), pos.y + backDir.y * Frand(0.0f, 0.15f),
                         pos.z + backDir.z * Frand(0.0f, 0.15f)};
        p->vel = Vector3{backDir.x * eject + carVel.x + Frand(-1.0f, 1.0f),
                         backDir.y * eject + carVel.y + Frand(-1.0f, 1.0f),
                         backDir.z * eject + carVel.z + Frand(-1.0f, 1.0f)};
        p->maxLife = p->life = Frand(0.15f, 0.3f);
        p->size = Frand(0.42f, 0.6f);
        p->sizeGrowth = Frand(-1.3f, -0.9f); // taille decroissante
        p->rot = Frand(0.0f, 360.0f);
        p->rotVel = Frand(-200.0f, 200.0f);
        p->phase = 0.0f;
        p->colorStart = Rgba(185, 210, 255, 255); // coeur blanc-bleu (additif)
        p->colorEnd = Rgba(255, 110, 15, 0);      // vers orange puis transparent
    }
}

void VfxSystem::EmitSparks(Vector3 pos, Vector3 dir) {
    float dl = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
    Vector3 d = (dl > 0.001f) ? Vector3{dir.x / dl, dir.y / dl, dir.z / dl} : Vector3{0.0f, 1.0f, 0.0f};
    int n = GetRandomValue(10, 16);
    for (int i = 0; i < n; ++i) {
        Particle* p = impl_->Alloc();
        if (p == nullptr) return;
        float sp = Frand(5.0f, 13.0f);
        p->type = PType::Spark;
        p->pos = pos;
        p->vel = Vector3{d.x * sp + Frand(-2.5f, 2.5f), d.y * sp + Frand(-1.0f, 3.0f), d.z * sp + Frand(-2.5f, 2.5f)};
        p->maxLife = p->life = Frand(0.4f, 0.8f);
        p->size = Frand(0.05f, 0.09f);
        p->sizeGrowth = 0.0f;
        p->rot = 0.0f;
        p->rotVel = 0.0f;
        p->phase = 0.0f;
        p->colorStart = Rgba(255, 244, 190, 255); // jaune-blanc (additif)
        p->colorEnd = Rgba(255, 120, 30, 0);
    }
}

void VfxSystem::EmitConfetti(Vector3 pos) {
    int n = GetRandomValue(45, 70);
    for (int i = 0; i < n; ++i) {
        Particle* p = impl_->Alloc();
        if (p == nullptr) return;
        Color vive = ColorFromHSV(Frand(0.0f, 360.0f), Frand(0.75f, 0.95f), 1.0f);
        p->type = PType::Confetti;
        p->pos = Vector3{pos.x + Frand(-0.4f, 0.4f), pos.y + Frand(-0.2f, 0.4f), pos.z + Frand(-0.4f, 0.4f)};
        p->vel = Vector3{Frand(-3.0f, 3.0f), Frand(1.5f, 5.5f), Frand(-3.0f, 3.0f)};
        p->maxLife = p->life = Frand(3.0f, 5.0f);
        p->size = Frand(0.12f, 0.2f);
        p->sizeGrowth = 0.0f;
        p->rot = Frand(0.0f, 360.0f);
        p->rotVel = Frand(-540.0f, 540.0f);
        p->phase = Frand(0.0f, 6.2831f);
        p->colorStart = Color{vive.r, vive.g, vive.b, 255};
        p->colorEnd = Color{vive.r, vive.g, vive.b, 0};
    }
}

void VfxSystem::SetRain(bool enabled) {
    impl_->rainTarget = enabled ? 1.0f : 0.0f;
}

int VfxSystem::ActiveCount() const {
    return impl_->count;
}

void VfxSystem::Clear() {
    impl_->count = 0;
    impl_->rainAccum = 0.0f; // l'intensite/cible de pluie est conservee
}

} // namespace racer
