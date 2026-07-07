/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Particle VFX system implementation
*/

#include "render/vfx.hpp"

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
constexpr float kGroundY = 0.0f;
constexpr float kRainRadius = 40.0f;
constexpr float kRainFallSpeed = -22.0f;
constexpr int kMaxImpactsParFrame = 64;

enum class PType : std::uint8_t {
    DRIFT_SMOKE,
    OFFROAD_DUST,
    NITRO_FLAME,
    SPARK,
    CONFETTI,
    RAIN,
    SPLASH
};

struct Particle {
    PType type = PType::DRIFT_SMOKE;
    Vector3 pos{0.0f, 0.0f, 0.0f};
    Vector3 vel{0.0f, 0.0f, 0.0f};
    float life = 0.0f;
    float maxLife = 1.0f;
    float size = 1.0f;
    float sizeGrowth = 0.0f;
    float rot = 0.0f;
    float rotVel = 0.0f;
    float phase = 0.0f;
    Color colorStart{255, 255, 255, 255};
    Color colorEnd{255, 255, 255, 0};
};

struct CamBasis {
    Vector3 right;
    Vector3 up;
    Vector3 fwd;
};

} // namespace

class VfxInternals {
public:
    static constexpr Color rgba(int r, int g, int b, int a);
    static float frand(float lo, float hi);
    static unsigned char lerpChannel(
        unsigned char ca, unsigned char cb, float t);
    static Color lerpColor(Color a, Color b, float t);
    static void finishTexture(Texture2D &tex);
    static Texture2D makePuffTexture();
    static Color streakPixelColor(int x, int y);
    static Texture2D makeStreakTexture();
    static Texture2D makeSquareTexture();
    static CamBasis makeCamBasis(const Camera3D &cam);
    static void emitQuad(
        const Texture2D &tex, Vector3 c, Vector3 hr, Vector3 hu, Color tint);
    static void computeBillboardAxes(
        const Particle &p, const CamBasis &cb, Vector3 &hr, Vector3 &hu);
    static void computeConfettiAxes(
        const Particle &p, const CamBasis &cb, Vector3 &hr, Vector3 &hu);
    static void computeStreakAxes(
        const Particle &p, const CamBasis &cb, Vector3 &hr, Vector3 &hu);
    static void computeAxes(
        const Particle &p, const CamBasis &cb, Vector3 &hr, Vector3 &hu);
    static void drawParticlesOfType(
        const Particle *pool, int count, PType type,
        const Texture2D &tex, const CamBasis &cb);
};

constexpr Color VfxInternals::rgba(int r, int g, int b, int a)
{
    return Color{
        static_cast<unsigned char>(r),
        static_cast<unsigned char>(g),
        static_cast<unsigned char>(b),
        static_cast<unsigned char>(a)};
}

float VfxInternals::frand(float lo, float hi)
{
    float t = static_cast<float>(GetRandomValue(0, 16383)) / 16383.0f;

    return lo + (hi - lo) * t;
}

unsigned char VfxInternals::lerpChannel(
    unsigned char ca, unsigned char cb, float t)
{
    float v = static_cast<float>(ca)
        + (static_cast<float>(cb) - static_cast<float>(ca)) * t;

    return static_cast<unsigned char>(std::clamp(v, 0.0f, 255.0f));
}

Color VfxInternals::lerpColor(Color a, Color b, float t)
{
    return Color{
        lerpChannel(a.r, b.r, t),
        lerpChannel(a.g, b.g, t),
        lerpChannel(a.b, b.b, t),
        lerpChannel(a.a, b.a, t)};
}

void VfxInternals::finishTexture(Texture2D &tex)
{
    GenTextureMipmaps(&tex);
    SetTextureFilter(tex, TEXTURE_FILTER_TRILINEAR);
    SetTextureWrap(tex, TEXTURE_WRAP_CLAMP);
}

Texture2D VfxInternals::makePuffTexture()
{
    Image img = GenImageGradientRadial(
        64, 64, 0.25f,
        rgba(255, 255, 255, 255),
        rgba(255, 255, 255, 0));
    Texture2D tex = LoadTextureFromImage(img);

    UnloadImage(img);
    finishTexture(tex);
    return tex;
}

Color VfxInternals::streakPixelColor(int x, int y)
{
    float u = (static_cast<float>(x) + 0.5f) / 16.0f * 2.0f - 1.0f;
    float v = (static_cast<float>(y) + 0.5f) / 64.0f;
    float bell = std::max(0.0f, 1.0f - std::fabs(u));

    bell *= bell;
    float ends = std::clamp(std::min(v, 1.0f - v) * 8.0f, 0.0f, 1.0f);

    return rgba(255, 255, 255, static_cast<int>(bell * ends * 255.0f));
}

Texture2D VfxInternals::makeStreakTexture()
{
    Image img = GenImageColor(16, 64, BLANK);

    for (int y = 0; y < 64; ++y) {
        for (int x = 0; x < 16; ++x)
            ImageDrawPixel(&img, x, y, streakPixelColor(x, y));
    }
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    finishTexture(tex);
    return tex;
}

Texture2D VfxInternals::makeSquareTexture()
{
    Image img = GenImageColor(8, 8, rgba(255, 255, 255, 255));
    Texture2D tex = LoadTextureFromImage(img);

    UnloadImage(img);
    finishTexture(tex);
    return tex;
}

CamBasis VfxInternals::makeCamBasis(const Camera3D &cam)
{
    Matrix v = MatrixLookAt(cam.position, cam.target, cam.up);
    CamBasis b;

    b.right = Vector3{v.m0, v.m4, v.m8};
    b.up = Vector3{v.m1, v.m5, v.m9};
    b.fwd = Vector3Normalize(Vector3Subtract(cam.target, cam.position));
    return b;
}

void VfxInternals::emitQuad(
    const Texture2D &tex, Vector3 c, Vector3 hr, Vector3 hu, Color tint)
{
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

void VfxInternals::computeBillboardAxes(
    const Particle &p, const CamBasis &cb, Vector3 &hr, Vector3 &hu)
{
    float half = p.size * 0.5f;
    float cr = std::cos(p.rot * DEG2RAD);
    float sr = std::sin(p.rot * DEG2RAD);

    hr = Vector3Add(
        Vector3Scale(cb.right, cr * half),
        Vector3Scale(cb.up, sr * half));
    hu = Vector3Add(
        Vector3Scale(cb.right, -sr * half),
        Vector3Scale(cb.up, cr * half));
}

void VfxInternals::computeConfettiAxes(
    const Particle &p, const CamBasis &cb, Vector3 &hr, Vector3 &hu)
{
    float age = p.maxLife - p.life;
    float flip = std::sin(p.phase + age * 7.0f);
    float halfW = p.size * 0.5f * flip;
    float halfH = p.size * 0.5f;
    float cr = std::cos(p.rot * DEG2RAD);
    float sr = std::sin(p.rot * DEG2RAD);

    hr = Vector3Add(
        Vector3Scale(cb.right, cr * halfW),
        Vector3Scale(cb.up, sr * halfW));
    hu = Vector3Add(
        Vector3Scale(cb.right, -sr * halfH),
        Vector3Scale(cb.up, cr * halfH));
}

void VfxInternals::computeStreakAxes(
    const Particle &p, const CamBasis &cb, Vector3 &hr, Vector3 &hu)
{
    Vector3 dir = p.vel;
    float speed = Vector3Length(dir);
    float halfLen = 0.7f;

    dir = (speed > 0.001f)
        ? Vector3Scale(dir, 1.0f / speed)
        : Vector3{0.0f, 1.0f, 0.0f};
    if (p.type == PType::SPARK)
        halfLen = std::clamp(speed * 0.035f, 0.08f, 0.35f);
    else if (p.type == PType::SPLASH)
        halfLen = 0.09f;
    hu = Vector3Scale(dir, halfLen);
    Vector3 side = Vector3CrossProduct(dir, cb.fwd);
    float sideLen = Vector3Length(side);

    side = (sideLen > 0.001f)
        ? Vector3Scale(side, 1.0f / sideLen)
        : cb.right;
    hr = Vector3Scale(side, p.size * 0.5f);
}

void VfxInternals::computeAxes(
    const Particle &p, const CamBasis &cb, Vector3 &hr, Vector3 &hu)
{
    switch (p.type) {
        case PType::DRIFT_SMOKE:
        case PType::OFFROAD_DUST:
        case PType::NITRO_FLAME:
            computeBillboardAxes(p, cb, hr, hu);
            break;
        case PType::CONFETTI:
            computeConfettiAxes(p, cb, hr, hu);
            break;
        case PType::SPARK:
        case PType::RAIN:
        case PType::SPLASH:
            computeStreakAxes(p, cb, hr, hu);
            break;
    }
}

void VfxInternals::drawParticlesOfType(
    const Particle *pool, int count, PType type,
    const Texture2D &tex, const CamBasis &cb)
{
    for (int i = 0; i < count; ++i) {
        const Particle &p = pool[i];

        if (p.type != type)
            continue;
        float t = 1.0f - p.life / p.maxLife;
        Color col = lerpColor(p.colorStart, p.colorEnd, t);
        Vector3 hr{0.0f, 0.0f, 0.0f};
        Vector3 hu{0.0f, 0.0f, 0.0f};

        computeAxes(p, cb, hr, hu);
        emitQuad(tex, p.pos, hr, hu, col);
    }
}

struct VfxSystem::Impl {
    std::array<Particle, kMaxParticles> pool_{};
    int count_ = 0;

    Texture2D texPuff_{};
    Texture2D texStreak_{};
    Texture2D texSquare_{};

    float rainIntensity_ = 0.0f;
    float rainTarget_ = 0.0f;
    float rainAccum_ = 0.0f;

    Particle *alloc();
    void resetScalars(Particle *p);
    void initRainDrop(Particle *p, Vector3 focus);
    void initSplashDrop(Particle *p, Vector3 at);
    void spawnRainDrop(Vector3 focus);
    void spawnSplash(Vector3 at);
    void initDriftSmoke(Particle *p, Vector3 pos, Vector3 carVel);
    void initOffroadDust(Particle *p, Vector3 pos, Vector3 carVel);
    void initNitroFlame(
        Particle *p, Vector3 pos, Vector3 backDir, Vector3 carVel);
    void initSpark(Particle *p, Vector3 pos, Vector3 dir, float sp);
    void initConfetti(Particle *p, Vector3 pos);
    void setConfettiColors(Particle *p);
    bool tickParticle(
        int index, float dt,
        std::array<Vector3, kMaxImpactsParFrame> &impacts,
        int &impactCount);
    void updateRainIntensity(float dt);
    void updateRainSpawn(float dt, Vector3 focus);
    void applyDriftSmoke(Particle &p, float dt);
    void applyOffroadDust(Particle &p, float dt);
    void applyNitroFlame(Particle &p, float dt);
    void applySpark(Particle &p, float dt);
    void applyConfetti(Particle &p, float dt);
    void applyTypePhysics(Particle &p, float dt);
    void integrateMotion(Particle &p, float dt);
    bool resolveCollision(
        Particle &p,
        std::array<Vector3, kMaxImpactsParFrame> &impacts,
        int &impactCount);
    void integrateAll(float dt);
};

Particle *VfxSystem::Impl::alloc()
{
    Particle *slot = nullptr;

    if (count_ < kMaxParticles)
        slot = &pool_[static_cast<std::size_t>(count_++)];
    return slot;
}

void VfxSystem::Impl::resetScalars(Particle *p)
{
    p->sizeGrowth = 0.0f;
    p->rot = 0.0f;
    p->rotVel = 0.0f;
    p->phase = 0.0f;
}

void VfxSystem::Impl::initRainDrop(Particle *p, Vector3 focus)
{
    float ang = VfxInternals::frand(0.0f, 2.0f * PI);
    float rad = kRainRadius * std::sqrt(VfxInternals::frand(0.0f, 1.0f));

    p->type = PType::RAIN;
    p->pos = Vector3{
        focus.x + std::cos(ang) * rad,
        focus.y + VfxInternals::frand(12.0f, 18.0f),
        focus.z + std::sin(ang) * rad};
    p->vel = Vector3{
        VfxInternals::frand(-1.8f, 1.8f),
        kRainFallSpeed + VfxInternals::frand(-2.0f, 2.0f),
        VfxInternals::frand(-1.8f, 1.8f)};
    p->maxLife = p->life = 1.4f;
    p->size = VfxInternals::frand(0.1f, 0.14f);
    resetScalars(p);
    p->colorStart = VfxInternals::rgba(190, 212, 240, 205);
    p->colorEnd = VfxInternals::rgba(195, 216, 242, 170);
}

void VfxSystem::Impl::spawnRainDrop(Vector3 focus)
{
    Particle *p = alloc();

    if (p == nullptr)
        return;
    initRainDrop(p, focus);
}

void VfxSystem::Impl::initSplashDrop(Particle *p, Vector3 at)
{
    float ang = VfxInternals::frand(0.0f, 2.0f * PI);
    float sp = VfxInternals::frand(0.9f, 1.9f);

    p->type = PType::SPLASH;
    p->pos = Vector3{at.x, kGroundY + 0.03f, at.z};
    p->vel = Vector3{
        std::cos(ang) * sp,
        VfxInternals::frand(0.15f, 0.4f),
        std::sin(ang) * sp};
    p->maxLife = p->life = 0.1f;
    p->size = 0.05f;
    resetScalars(p);
    p->colorStart = VfxInternals::rgba(190, 210, 235, 150);
    p->colorEnd = VfxInternals::rgba(195, 215, 240, 0);
}

void VfxSystem::Impl::spawnSplash(Vector3 at)
{
    for (int i = 0; i < 2; ++i) {
        Particle *p = alloc();

        if (p == nullptr)
            return;
        initSplashDrop(p, at);
    }
}

void VfxSystem::Impl::updateRainIntensity(float dt)
{
    float step = 0.5f * dt;

    if (rainIntensity_ < rainTarget_)
        rainIntensity_ = std::min(rainTarget_, rainIntensity_ + step);
    else if (rainIntensity_ > rainTarget_)
        rainIntensity_ = std::max(rainTarget_, rainIntensity_ - step);
}

void VfxSystem::Impl::updateRainSpawn(float dt, Vector3 focus)
{
    if (rainIntensity_ > 0.001f) {
        rainAccum_ += 300.0f * rainIntensity_ * dt;
        int n = std::min(static_cast<int>(rainAccum_), 64);

        rainAccum_ -= static_cast<float>(n);
        for (int i = 0; i < n; ++i)
            spawnRainDrop(focus);
    } else {
        rainAccum_ = 0.0f;
    }
}

void VfxSystem::Impl::applyDriftSmoke(Particle &p, float dt)
{
    float d = std::max(0.0f, 1.0f - 1.7f * dt);

    p.vel.x *= d;
    p.vel.z *= d;
    p.vel.y += 0.5f * dt;
}

void VfxSystem::Impl::applyOffroadDust(Particle &p, float dt)
{
    float d = std::max(0.0f, 1.0f - 2.4f * dt);

    p.vel.x *= d;
    p.vel.z *= d;
    p.vel.y -= 2.5f * dt;
}

void VfxSystem::Impl::applyNitroFlame(Particle &p, float dt)
{
    float d = std::max(0.0f, 1.0f - 5.0f * dt);

    p.vel.x *= d;
    p.vel.y *= d;
    p.vel.z *= d;
}

void VfxSystem::Impl::applySpark(Particle &p, float dt)
{
    p.vel.y -= 25.0f * dt;
}

void VfxSystem::Impl::applyConfetti(Particle &p, float dt)
{
    float age = p.maxLife - p.life;
    float d = std::max(0.0f, 1.0f - 0.8f * dt);

    p.vel.y -= 3.5f * dt;
    if (p.vel.y < -1.5f)
        p.vel.y = -1.5f;
    p.vel.x = p.vel.x * d
        + std::cos(age * 2.6f + p.phase) * 2.2f * dt;
    p.vel.z = p.vel.z * d
        + std::sin(age * 2.1f + p.phase * 1.7f) * 2.2f * dt;
}

void VfxSystem::Impl::applyTypePhysics(Particle &p, float dt)
{
    switch (p.type) {
    case PType::DRIFT_SMOKE: applyDriftSmoke(p, dt); break;
    case PType::OFFROAD_DUST: applyOffroadDust(p, dt); break;
    case PType::NITRO_FLAME: applyNitroFlame(p, dt); break;
    case PType::SPARK: applySpark(p, dt); break;
    case PType::CONFETTI: applyConfetti(p, dt); break;
    case PType::RAIN:
    case PType::SPLASH:
        break;
    }
}

void VfxSystem::Impl::initDriftSmoke(
    Particle *p, Vector3 pos, Vector3 carVel)
{
    p->type = PType::DRIFT_SMOKE;
    p->pos = Vector3{
        pos.x + VfxInternals::frand(-0.18f, 0.18f),
        std::max(0.08f, pos.y + VfxInternals::frand(-0.05f, 0.12f)),
        pos.z + VfxInternals::frand(-0.18f, 0.18f)};
    p->vel = Vector3{
        carVel.x * 0.25f + VfxInternals::frand(-0.9f, 0.9f),
        carVel.y * 0.25f + VfxInternals::frand(0.6f, 1.5f),
        carVel.z * 0.25f + VfxInternals::frand(-0.9f, 0.9f)};
    p->maxLife = p->life = VfxInternals::frand(0.8f, 1.4f);
    p->size = VfxInternals::frand(0.4f, 0.6f);
    p->sizeGrowth = VfxInternals::frand(1.3f, 2.1f);
    p->rot = VfxInternals::frand(0.0f, 360.0f);
    p->rotVel = VfxInternals::frand(-80.0f, 80.0f);
    p->phase = 0.0f;
    p->colorStart = VfxInternals::rgba(192, 192, 198, 110);
    p->colorEnd = VfxInternals::rgba(225, 225, 232, 0);
}

void VfxSystem::Impl::initOffroadDust(
    Particle *p, Vector3 pos, Vector3 carVel)
{
    p->type = PType::OFFROAD_DUST;
    p->pos = Vector3{
        pos.x + VfxInternals::frand(-0.25f, 0.25f),
        std::max(0.06f, pos.y),
        pos.z + VfxInternals::frand(-0.25f, 0.25f)};
    p->vel = Vector3{
        carVel.x * 0.2f + VfxInternals::frand(-1.2f, 1.2f),
        VfxInternals::frand(0.15f, 0.6f),
        carVel.z * 0.2f + VfxInternals::frand(-1.2f, 1.2f)};
    p->maxLife = p->life = VfxInternals::frand(0.35f, 0.7f);
    p->size = VfxInternals::frand(0.3f, 0.5f);
    p->sizeGrowth = VfxInternals::frand(1.0f, 1.6f);
    p->rot = VfxInternals::frand(0.0f, 360.0f);
    p->rotVel = VfxInternals::frand(-60.0f, 60.0f);
    p->phase = 0.0f;
    p->colorStart = VfxInternals::rgba(206, 186, 140, 82);
    p->colorEnd = VfxInternals::rgba(214, 198, 164, 0);
}

void VfxSystem::Impl::initNitroFlame(
    Particle *p, Vector3 pos, Vector3 backDir, Vector3 carVel)
{
    float eject = VfxInternals::frand(8.0f, 14.0f);

    p->type = PType::NITRO_FLAME;
    p->pos = Vector3{
        pos.x + backDir.x * VfxInternals::frand(0.0f, 0.15f),
        pos.y + backDir.y * VfxInternals::frand(0.0f, 0.15f),
        pos.z + backDir.z * VfxInternals::frand(0.0f, 0.15f)};
    p->vel = Vector3{
        backDir.x * eject + carVel.x + VfxInternals::frand(-1.0f, 1.0f),
        backDir.y * eject + carVel.y + VfxInternals::frand(-1.0f, 1.0f),
        backDir.z * eject + carVel.z + VfxInternals::frand(-1.0f, 1.0f)};
    p->maxLife = p->life = VfxInternals::frand(0.15f, 0.3f);
    p->size = VfxInternals::frand(0.42f, 0.6f);
    p->sizeGrowth = VfxInternals::frand(-1.3f, -0.9f);
    p->rot = VfxInternals::frand(0.0f, 360.0f);
    p->rotVel = VfxInternals::frand(-200.0f, 200.0f);
    p->phase = 0.0f;
    p->colorStart = VfxInternals::rgba(185, 210, 255, 255);
    p->colorEnd = VfxInternals::rgba(255, 110, 15, 0);
}

void VfxSystem::Impl::initSpark(
    Particle *p, Vector3 pos, Vector3 dir, float sp)
{
    p->type = PType::SPARK;
    p->pos = pos;
    p->vel = Vector3{
        dir.x * sp + VfxInternals::frand(-2.5f, 2.5f),
        dir.y * sp + VfxInternals::frand(-1.0f, 3.0f),
        dir.z * sp + VfxInternals::frand(-2.5f, 2.5f)};
    p->maxLife = p->life = VfxInternals::frand(0.4f, 0.8f);
    p->size = VfxInternals::frand(0.05f, 0.09f);
    resetScalars(p);
    p->colorStart = VfxInternals::rgba(255, 244, 190, 255);
    p->colorEnd = VfxInternals::rgba(255, 120, 30, 0);
}

void VfxSystem::Impl::setConfettiColors(Particle *p)
{
    Color vive = ColorFromHSV(
        VfxInternals::frand(0.0f, 360.0f),
        VfxInternals::frand(0.75f, 0.95f),
        1.0f);

    p->colorStart = Color{vive.r, vive.g, vive.b, 255};
    p->colorEnd = Color{vive.r, vive.g, vive.b, 0};
}

void VfxSystem::Impl::initConfetti(Particle *p, Vector3 pos)
{
    p->type = PType::CONFETTI;
    p->pos = Vector3{
        pos.x + VfxInternals::frand(-0.4f, 0.4f),
        pos.y + VfxInternals::frand(-0.2f, 0.4f),
        pos.z + VfxInternals::frand(-0.4f, 0.4f)};
    p->vel = Vector3{
        VfxInternals::frand(-3.0f, 3.0f),
        VfxInternals::frand(1.5f, 5.5f),
        VfxInternals::frand(-3.0f, 3.0f)};
    p->maxLife = p->life = VfxInternals::frand(3.0f, 5.0f);
    p->size = VfxInternals::frand(0.12f, 0.2f);
    p->sizeGrowth = 0.0f;
    p->rot = VfxInternals::frand(0.0f, 360.0f);
    p->rotVel = VfxInternals::frand(-540.0f, 540.0f);
    p->phase = VfxInternals::frand(0.0f, 6.2831f);
    setConfettiColors(p);
}

void VfxSystem::Impl::integrateMotion(Particle &p, float dt)
{
    p.pos.x += p.vel.x * dt;
    p.pos.y += p.vel.y * dt;
    p.pos.z += p.vel.z * dt;
}

bool VfxSystem::Impl::resolveCollision(
    Particle &p,
    std::array<Vector3, kMaxImpactsParFrame> &impacts,
    int &impactCount)
{
    bool kill = false;

    if (p.type == PType::SPARK && p.pos.y < kGroundY) {
        p.pos.y = kGroundY;
        p.vel.y *= -0.4f;
        p.vel.x *= 0.75f;
        p.vel.z *= 0.75f;
        p.life *= 0.6f;
    } else if (p.type == PType::OFFROAD_DUST && p.pos.y < 0.05f) {
        p.pos.y = 0.05f;
        if (p.vel.y < 0.0f)
            p.vel.y = 0.0f;
    } else if (p.type == PType::CONFETTI && p.pos.y < 0.03f) {
        p.pos.y = 0.03f;
        p.vel = Vector3{0.0f, 0.0f, 0.0f};
    } else if (p.type == PType::RAIN && p.pos.y <= kGroundY) {
        if (impactCount < kMaxImpactsParFrame)
            impacts[static_cast<std::size_t>(impactCount++)] = p.pos;
        kill = true;
    }
    return kill;
}

bool VfxSystem::Impl::tickParticle(
    int index, float dt,
    std::array<Vector3, kMaxImpactsParFrame> &impacts,
    int &impactCount)
{
    Particle &p = pool_[static_cast<std::size_t>(index)];
    bool kill = false;

    p.life -= dt;
    kill = p.life <= 0.0f;
    if (!kill) {
        applyTypePhysics(p, dt);
        integrateMotion(p, dt);
        kill = resolveCollision(p, impacts, impactCount);
        p.rot += p.rotVel * dt;
        p.size = std::max(0.02f, p.size + p.sizeGrowth * dt);
    }
    if (kill) {
        pool_[static_cast<std::size_t>(index)]
            = pool_[static_cast<std::size_t>(--count_)];
        return true;
    }
    return false;
}

void VfxSystem::Impl::integrateAll(float dt)
{
    std::array<Vector3, kMaxImpactsParFrame> impacts{};
    int impactCount = 0;

    for (int i = 0; i < count_;) {
        if (tickParticle(i, dt, impacts, impactCount))
            continue;
        ++i;
    }
    for (int i = 0; i < impactCount; ++i) {
        const Vector3 &hit = impacts[static_cast<std::size_t>(i)];

        spawnSplash(Vector3{hit.x, kGroundY, hit.z});
    }
}

VfxSystem::VfxSystem() : impl_(std::make_unique<Impl>())
{
    impl_->texPuff_ = VfxInternals::makePuffTexture();
    impl_->texStreak_ = VfxInternals::makeStreakTexture();
    impl_->texSquare_ = VfxInternals::makeSquareTexture();
}

VfxSystem::~VfxSystem()
{
    UnloadTexture(impl_->texPuff_);
    UnloadTexture(impl_->texStreak_);
    UnloadTexture(impl_->texSquare_);
}

void VfxSystem::Update(float dt, Vector3 focus)
{
    if (dt <= 0.0f)
        return;
    dt = std::min(dt, 0.1f);
    impl_->updateRainIntensity(dt);
    impl_->updateRainSpawn(dt, focus);
    impl_->integrateAll(dt);
}

void VfxSystem::Draw(const Camera3D &camera) const
{
    const Impl &s = *impl_;

    if (s.count_ == 0)
        return;
    CamBasis cb = VfxInternals::makeCamBasis(camera);

    rlDrawRenderBatchActive();
    rlDisableDepthMask();
    rlDisableBackfaceCulling();
    BeginBlendMode(BLEND_ALPHA);
    VfxInternals::drawParticlesOfType(
        s.pool_.data(), s.count_, PType::DRIFT_SMOKE, s.texPuff_, cb);
    VfxInternals::drawParticlesOfType(
        s.pool_.data(), s.count_, PType::OFFROAD_DUST, s.texPuff_, cb);
    VfxInternals::drawParticlesOfType(
        s.pool_.data(), s.count_, PType::CONFETTI, s.texSquare_, cb);
    VfxInternals::drawParticlesOfType(
        s.pool_.data(), s.count_, PType::SPLASH, s.texStreak_, cb);
    VfxInternals::drawParticlesOfType(
        s.pool_.data(), s.count_, PType::RAIN, s.texStreak_, cb);
    BeginBlendMode(BLEND_ADDITIVE);
    VfxInternals::drawParticlesOfType(
        s.pool_.data(), s.count_, PType::NITRO_FLAME, s.texPuff_, cb);
    VfxInternals::drawParticlesOfType(
        s.pool_.data(), s.count_, PType::SPARK, s.texStreak_, cb);
    EndBlendMode();
    rlDrawRenderBatchActive();
    rlEnableBackfaceCulling();
    rlEnableDepthMask();
}

void VfxSystem::EmitDriftSmoke(Vector3 pos, Vector3 carVel)
{
    int n = GetRandomValue(2, 3);

    for (int i = 0; i < n; ++i) {
        Particle *p = impl_->alloc();

        if (p == nullptr)
            break;
        impl_->initDriftSmoke(p, pos, carVel);
    }
}

void VfxSystem::EmitOffroadDust(Vector3 pos, Vector3 carVel)
{
    int n = GetRandomValue(1, 2);

    for (int i = 0; i < n; ++i) {
        Particle *p = impl_->alloc();

        if (p == nullptr)
            break;
        impl_->initOffroadDust(p, pos, carVel);
    }
}

void VfxSystem::EmitNitroFlame(Vector3 pos, Vector3 backDir, Vector3 carVel)
{
    int n = GetRandomValue(4, 5);

    for (int i = 0; i < n; ++i) {
        Particle *p = impl_->alloc();

        if (p == nullptr)
            break;
        impl_->initNitroFlame(p, pos, backDir, carVel);
    }
}

void VfxSystem::EmitSparks(Vector3 pos, Vector3 dir)
{
    float dl = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
    Vector3 d = (dl > 0.001f)
        ? Vector3{dir.x / dl, dir.y / dl, dir.z / dl}
        : Vector3{0.0f, 1.0f, 0.0f};
    int n = GetRandomValue(10, 16);

    for (int i = 0; i < n; ++i) {
        Particle *p = impl_->alloc();

        if (p == nullptr)
            break;
        impl_->initSpark(p, pos, d, VfxInternals::frand(5.0f, 13.0f));
    }
}

void VfxSystem::EmitConfetti(Vector3 pos)
{
    int n = GetRandomValue(45, 70);

    for (int i = 0; i < n; ++i) {
        Particle *p = impl_->alloc();

        if (p == nullptr)
            break;
        impl_->initConfetti(p, pos);
    }
}

void VfxSystem::SetRain(bool enabled)
{
    impl_->rainTarget_ = enabled ? 1.0f : 0.0f;
}

int VfxSystem::ActiveCount() const
{
    return impl_->count_;
}

void VfxSystem::Clear()
{
    impl_->count_ = 0;
    impl_->rainAccum_ = 0.0f;
}

} // namespace racer
