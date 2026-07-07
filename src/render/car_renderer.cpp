#include "render/car_renderer.h"

#include <cmath>

#include "rlgl.h"

namespace racer {

namespace {

// Eclaircit/assombrit une couleur (facteur multiplicatif, clamp 0..255).
Color Shade(Color c, float f) {
    auto ch = [f](unsigned char v) {
        float s = static_cast<float>(v) * f;
        if (s < 0.0f) s = 0.0f;
        if (s > 255.0f) s = 255.0f;
        return static_cast<unsigned char>(s);
    };
    return Color{ch(c.r), ch(c.g), ch(c.b), c.a};
}

float WrapAngle(float a) {
    while (a > PI) a -= 2.0f * PI;
    while (a < -PI) a += 2.0f * PI;
    return a;
}

float Clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

// Geometrie locale partagee entre le dessin et GetCarLightPoints.
// Repere local : +Z avant, +Y haut, sol a y=0.
constexpr float kHeadX = 0.48f, kHeadY = 0.44f, kHeadZ = 2.37f;
constexpr float kBrakeX = 0.55f, kBrakeY = 0.55f, kBrakeZ = -2.20f;
constexpr float kExhaustX = 0.17f, kExhaustY = 0.24f, kExhaustZ = -2.26f;

// Roue cylindrique sur axe X local : pneu sombre + flasque de jante claire +
// croix sombre qui tourne avec wheelSpin (rend la rotation lisible).
void DrawWheel(Vector3 center, float steerDeg, float spinDeg) {
    rlPushMatrix();
    rlTranslatef(center.x, center.y, center.z);
    rlRotatef(steerDeg, 0.0f, 1.0f, 0.0f);

    DrawCylinderEx(Vector3{-0.17f, 0.0f, 0.0f}, Vector3{0.17f, 0.0f, 0.0f},
                   kWheelRadius, kWheelRadius, 14, Color{23, 23, 26, 255});
    DrawCylinderEx(Vector3{-0.18f, 0.0f, 0.0f}, Vector3{0.18f, 0.0f, 0.0f},
                   0.225f, 0.225f, 12, Color{198, 200, 205, 255});

    rlRotatef(spinDeg, 1.0f, 0.0f, 0.0f);
    DrawCube(Vector3{0.0f, 0.0f, 0.0f}, 0.385f, 0.38f, 0.08f, Color{45, 45, 52, 255});
    DrawCube(Vector3{0.0f, 0.0f, 0.0f}, 0.385f, 0.08f, 0.38f, Color{45, 45, 52, 255});

    // Ecrou central (depasse legerement des flasques).
    DrawCylinderEx(Vector3{-0.20f, 0.0f, 0.0f}, Vector3{0.20f, 0.0f, 0.0f},
                   0.055f, 0.055f, 8, Color{225, 190, 90, 255});

    rlPopMatrix();
}

// Ombre douce : disques superposes d'alpha decroissant, etires selon la
// vitesse le long de la trajectoire reelle, decales lateralement en drift.
void DrawShadow(const Car& car, bool drifting) {
    float speedNorm = Clampf(std::fabs(car.speed) / 30.0f, 0.0f, 1.0f);
    float stretch = 1.0f + 0.40f * speedNorm;
    float slip = WrapAngle(car.velocityHeading - car.heading);
    float sideOffset = drifting ? Clampf(-slip * 0.9f, -0.5f, 0.5f) : 0.0f;

    rlPushMatrix();
    rlTranslatef(car.position.x, 0.0f, car.position.z);
    rlRotatef(car.velocityHeading * RAD2DEG, 0.0f, 1.0f, 0.0f);
    rlTranslatef(sideOffset, 0.0f, 0.0f);
    rlScalef(0.62f, 1.0f, stretch); // ellipse allongee (normales ~verticales, sans consequence)
    DrawCylinder(Vector3{0.0f, 0.010f, 0.0f}, 2.45f, 2.45f, 0.006f, 22, Fade(BLACK, 0.09f));
    DrawCylinder(Vector3{0.0f, 0.018f, 0.0f}, 2.05f, 2.05f, 0.006f, 22, Fade(BLACK, 0.12f));
    DrawCylinder(Vector3{0.0f, 0.026f, 0.0f}, 1.60f, 1.60f, 0.006f, 22, Fade(BLACK, 0.16f));
    rlPopMatrix();
}

// Flamme d'echappement a trois couches (enveloppe orange, corps jaune,
// coeur bleu-blanc), longueur vacillante pour un rendu vivant.
void DrawExhaustFlame(float x, float t, float phase) {
    float flicker = 0.30f * std::sin(t * 40.0f + phase) + 0.14f * std::sin(t * 23.7f + phase * 1.7f);
    float len = Clampf(1.05f + flicker, 0.55f, 1.5f);

    Vector3 base{x, kExhaustY, kExhaustZ + 0.02f};
    Vector3 tipOuter{x, kExhaustY + 0.06f, kExhaustZ - len};
    Vector3 tipMid{x, kExhaustY + 0.03f, kExhaustZ - len * 0.78f};
    Vector3 tipCore{x, kExhaustY + 0.01f, kExhaustZ - len * 0.50f};

    // Interieur vers exterieur : les primitives translucides ecrivent le
    // depth buffer, le coeur doit donc etre trace avant l'enveloppe.
    DrawCylinderEx(base, tipCore, 0.048f, 0.010f, 8, Fade(Color{190, 230, 255, 255}, 0.95f));
    DrawCylinderEx(base, tipMid, 0.075f, 0.012f, 8, Fade(Color{255, 210, 90, 255}, 0.60f));
    DrawCylinderEx(base, tipOuter, 0.105f, 0.014f, 8, Fade(Color{255, 120, 30, 255}, 0.42f));
}

} // namespace

CarLightPoints GetCarLightPoints(const Car& car) {
    float c = std::cos(car.heading);
    float s = std::sin(car.heading);
    auto toWorld = [&](float lx, float ly, float lz) {
        return Vector3{car.position.x + lx * c + lz * s,
                       car.position.y + ly,
                       car.position.z - lx * s + lz * c};
    };
    CarLightPoints pts{};
    pts.headL = toWorld(-kHeadX, kHeadY, kHeadZ);
    pts.headR = toWorld(kHeadX, kHeadY, kHeadZ);
    pts.brakeL = toWorld(-kBrakeX, kBrakeY, kBrakeZ);
    pts.brakeR = toWorld(kBrakeX, kBrakeY, kBrakeZ);
    pts.exhaust = toWorld(0.0f, kExhaustY, kExhaustZ);
    return pts;
}

void DrawCar(const Car& car, Color bodyColor) {
    DrawCarEx(car, CarVisual{}, bodyColor);
}

void DrawCarEx(const Car& car, const CarVisual& vis, Color bodyColor) {
    const float t = static_cast<float>(GetTime());

    // Couleurs derivees de la livree.
    const float lum = 0.299f * bodyColor.r + 0.587f * bodyColor.g + 0.114f * bodyColor.b;
    const Color stripe = (lum > 150.0f) ? Color{30, 30, 36, 255} : Color{245, 245, 242, 255};
    const Color carbon{36, 36, 42, 255};
    const Color carbonLight{52, 52, 58, 255};

    DrawShadow(car, vis.drifting);

    rlPushMatrix();
    rlTranslatef(car.position.x, car.position.y, car.position.z);
    rlRotatef(car.heading * RAD2DEG, 0.0f, 1.0f, 0.0f);

    // --- Roues : hors roulis (elles restent posees au sol). ---
    const float steerDeg = Clampf(vis.steer, -1.0f, 1.0f) * 28.0f;
    const float spinDeg = vis.wheelSpin * RAD2DEG;
    DrawWheel(Vector3{-0.88f, kWheelRadius, 1.45f}, steerDeg, spinDeg);
    DrawWheel(Vector3{0.88f, kWheelRadius, 1.45f}, steerDeg, spinDeg);
    DrawWheel(Vector3{-0.88f, kWheelRadius, -1.42f}, 0.0f, spinDeg);
    DrawWheel(Vector3{0.88f, kWheelRadius, -1.42f}, 0.0f, spinDeg);

    // --- Effets au sol (non affectes par le roulis du chassis). ---
    if (vis.headlights) {
        rlPushMatrix();
        rlTranslatef(0.0f, 0.02f, 5.0f);
        rlScalef(1.5f, 1.0f, 2.3f);
        DrawCylinder(Vector3{0.0f, 0.0f, 0.0f}, 0.62f, 0.62f, 0.005f, 18, Fade(Color{255, 240, 180, 255}, 0.10f));
        DrawCylinder(Vector3{0.0f, 0.006f, 0.0f}, 0.36f, 0.36f, 0.005f, 18, Fade(Color{255, 246, 205, 255}, 0.10f));
        rlPopMatrix();
    }
    if (vis.nitro) {
        rlPushMatrix();
        rlTranslatef(0.0f, 0.02f, -2.95f);
        rlScalef(1.35f, 1.0f, 1.8f);
        DrawCylinder(Vector3{0.0f, 0.0f, 0.0f}, 0.55f, 0.55f, 0.005f, 16, Fade(Color{255, 150, 60, 255}, 0.16f));
        rlPopMatrix();
    }

    // --- Pose dynamique du chassis : roulis / plongee / squat. ---
    float speedFactor = Clampf(std::fabs(car.speed) / 12.0f, 0.0f, 1.0f);
    float rollDeg = Clampf(vis.steer, -1.0f, 1.0f) * (vis.drifting ? 5.2f : 3.4f) * speedFactor;
    float pitchDeg = (vis.braking ? 2.1f : 0.0f) + (vis.nitro ? -1.4f : 0.0f);

    rlPushMatrix();
    rlTranslatef(0.0f, 0.34f, 0.0f);
    rlRotatef(rollDeg, 0.0f, 0.0f, 1.0f);
    rlRotatef(pitchDeg, 1.0f, 0.0f, 0.0f);
    rlTranslatef(0.0f, -0.34f, 0.0f);

    // --- Carrosserie empilee : bas de caisse, corps, capot plongeant, nez, arriere tronque. ---
    DrawCube(Vector3{0.0f, 0.20f, -0.05f}, 1.70f, 0.22f, 4.10f, Shade(bodyColor, 0.32f));
    DrawCube(Vector3{0.0f, 0.44f, 0.10f}, 1.78f, 0.30f, 3.60f, bodyColor);
    DrawCube(Vector3{0.0f, 0.42f, 1.95f}, 1.44f, 0.22f, 0.85f, Shade(bodyColor, 1.10f));
    DrawCube(Vector3{0.0f, 0.36f, 2.42f}, 1.02f, 0.16f, 0.34f, Shade(bodyColor, 1.16f));
    DrawCube(Vector3{0.0f, 0.52f, -1.35f}, 1.72f, 0.34f, 0.95f, Shade(bodyColor, 0.90f));
    DrawCube(Vector3{0.0f, 0.47f, -1.98f}, 1.70f, 0.40f, 0.42f, Shade(bodyColor, 0.72f));

    // Passages de roues (le haut des pneus disparait dessous).
    DrawCube(Vector3{-0.82f, 0.63f, 1.45f}, 0.40f, 0.10f, 0.98f, Shade(bodyColor, 1.04f));
    DrawCube(Vector3{0.82f, 0.63f, 1.45f}, 0.40f, 0.10f, 0.98f, Shade(bodyColor, 1.04f));
    DrawCube(Vector3{-0.82f, 0.63f, -1.42f}, 0.40f, 0.10f, 0.98f, Shade(bodyColor, 0.96f));
    DrawCube(Vector3{0.82f, 0.63f, -1.42f}, 0.40f, 0.10f, 0.98f, Shade(bodyColor, 0.96f));

    // Splitter avant + diffuseur arriere (carbone).
    DrawCube(Vector3{0.0f, 0.15f, 2.35f}, 1.58f, 0.09f, 0.35f, carbon);
    DrawCube(Vector3{0.0f, 0.16f, -2.22f}, 1.46f, 0.13f, 0.24f, carbon);
    DrawCube(Vector3{-0.42f, 0.20f, -2.22f}, 0.045f, 0.20f, 0.22f, carbonLight);
    DrawCube(Vector3{0.0f, 0.20f, -2.22f}, 0.045f, 0.20f, 0.22f, carbonLight);
    DrawCube(Vector3{0.42f, 0.20f, -2.22f}, 0.045f, 0.20f, 0.22f, carbonLight);

    // Livree : double bande contrastee capot -> toit -> arriere.
    DrawCube(Vector3{-0.17f, 0.598f, 0.10f}, 0.15f, 0.016f, 3.55f, stripe);
    DrawCube(Vector3{0.17f, 0.598f, 0.10f}, 0.15f, 0.016f, 3.55f, stripe);
    DrawCube(Vector3{-0.17f, 0.538f, 1.95f}, 0.15f, 0.016f, 0.80f, stripe);
    DrawCube(Vector3{0.17f, 0.538f, 1.95f}, 0.15f, 0.016f, 0.80f, stripe);
    DrawCube(Vector3{-0.17f, 0.698f, -1.35f}, 0.15f, 0.016f, 0.90f, stripe);
    DrawCube(Vector3{0.17f, 0.698f, -1.35f}, 0.15f, 0.016f, 0.90f, stripe);

    // Numero de course : plaque blanche + "chiffre" sombre sur chaque porte.
    DrawCube(Vector3{-0.90f, 0.47f, 0.25f}, 0.025f, 0.30f, 0.44f, Color{242, 242, 238, 255});
    DrawCube(Vector3{0.90f, 0.47f, 0.25f}, 0.025f, 0.30f, 0.44f, Color{242, 242, 238, 255});
    DrawCube(Vector3{-0.918f, 0.47f, 0.25f}, 0.012f, 0.17f, 0.09f, Color{30, 30, 34, 255});
    DrawCube(Vector3{0.918f, 0.47f, 0.25f}, 0.012f, 0.17f, 0.09f, Color{30, 30, 34, 255});

    // Entree d'air sur le capot.
    DrawCube(Vector3{0.0f, 0.64f, 0.85f}, 0.34f, 0.11f, 0.44f, carbonLight);
    DrawCube(Vector3{0.0f, 0.645f, 1.08f}, 0.24f, 0.07f, 0.02f, Color{12, 12, 14, 255});

    // Calandre basse sombre sur le nez (donne un "visage" a l'avant).
    DrawCube(Vector3{0.0f, 0.33f, 2.575f}, 0.62f, 0.10f, 0.05f, Color{14, 14, 16, 255});

    // Prises d'air laterales devant les roues arriere.
    DrawCube(Vector3{-0.905f, 0.44f, -0.62f}, 0.05f, 0.18f, 0.46f, Color{14, 14, 16, 255});
    DrawCube(Vector3{0.905f, 0.44f, -0.62f}, 0.05f, 0.18f, 0.46f, Color{14, 14, 16, 255});

    // Retroviseurs sur tiges.
    DrawCube(Vector3{-0.78f, 0.74f, 0.22f}, 0.14f, 0.035f, 0.035f, carbon);
    DrawCube(Vector3{0.78f, 0.74f, 0.22f}, 0.14f, 0.035f, 0.035f, carbon);
    DrawCube(Vector3{-0.92f, 0.78f, 0.22f}, 0.16f, 0.09f, 0.05f, carbonLight);
    DrawCube(Vector3{0.92f, 0.78f, 0.22f}, 0.16f, 0.09f, 0.05f, carbonLight);

    // Antenne inclinee + boule.
    DrawCylinderEx(Vector3{0.32f, 0.69f, -1.30f}, Vector3{0.40f, 1.02f, -1.42f}, 0.012f, 0.012f, 6, Color{25, 25, 28, 255});
    DrawSphere(Vector3{0.40f, 1.02f, -1.42f}, 0.024f, Color{25, 25, 28, 255});

    // Arceau de securite au-dessus du pilote.
    DrawCube(Vector3{0.0f, 1.00f, -0.90f}, 0.72f, 0.07f, 0.07f, carbonLight);
    DrawCube(Vector3{-0.325f, 0.90f, -0.90f}, 0.07f, 0.24f, 0.07f, carbonLight);
    DrawCube(Vector3{0.325f, 0.90f, -0.90f}, 0.07f, 0.24f, 0.07f, carbonLight);

    // Pilote : epaules + casque colore, visibles sous la verriere.
    DrawCube(Vector3{0.0f, 0.70f, -0.62f}, 0.46f, 0.16f, 0.26f, Color{40, 40, 45, 255});
    DrawSphere(Vector3{0.0f, 0.82f, -0.55f}, 0.13f, Color{252, 216, 80, 255});

    // Aileron arriere sur pylones + derives laterales.
    DrawCube(Vector3{-0.48f, 0.80f, -2.02f}, 0.09f, 0.26f, 0.13f, carbonLight);
    DrawCube(Vector3{0.48f, 0.80f, -2.02f}, 0.09f, 0.26f, 0.13f, carbonLight);
    DrawCube(Vector3{0.0f, 0.97f, -2.05f}, 1.76f, 0.07f, 0.52f, Shade(bodyColor, 0.55f));
    DrawCube(Vector3{-0.885f, 0.95f, -2.05f}, 0.05f, 0.24f, 0.58f, carbon);
    DrawCube(Vector3{0.885f, 0.95f, -2.05f}, 0.05f, 0.24f, 0.58f, carbon);

    // Sorties d'echappement.
    DrawCylinderEx(Vector3{-kExhaustX, kExhaustY, -2.05f}, Vector3{-kExhaustX, kExhaustY, kExhaustZ}, 0.055f, 0.055f, 8, Color{35, 35, 38, 255});
    DrawCylinderEx(Vector3{kExhaustX, kExhaustY, -2.05f}, Vector3{kExhaustX, kExhaustY, kExhaustZ}, 0.055f, 0.055f, 8, Color{35, 35, 38, 255});

    // Phares avant (emissifs blancs-chauds, plus vifs si allumes) sur bezel sombre.
    DrawCube(Vector3{-kHeadX, kHeadY, kHeadZ - 0.012f}, 0.38f, 0.16f, 0.05f, Color{20, 20, 22, 255});
    DrawCube(Vector3{kHeadX, kHeadY, kHeadZ - 0.012f}, 0.38f, 0.16f, 0.05f, Color{20, 20, 22, 255});
    const Color headColor = vis.headlights ? Color{255, 252, 235, 255} : Color{248, 238, 205, 255};
    DrawCube(Vector3{-kHeadX, kHeadY, kHeadZ}, 0.30f, 0.11f, 0.06f, headColor);
    DrawCube(Vector3{kHeadX, kHeadY, kHeadZ}, 0.30f, 0.11f, 0.06f, headColor);

    // Feux stop (rouge vif au freinage, rouge sombre sinon) sur bandeau noir.
    DrawCube(Vector3{0.0f, kBrakeY, kBrakeZ - 0.005f}, 1.58f, 0.20f, 0.04f, Color{18, 18, 20, 255});
    const Color brakeColor = vis.braking ? Color{255, 46, 36, 255} : Color{110, 18, 18, 255};
    DrawCube(Vector3{-kBrakeX, kBrakeY, kBrakeZ}, 0.36f, 0.13f, 0.05f, brakeColor);
    DrawCube(Vector3{kBrakeX, kBrakeY, kBrakeZ}, 0.36f, 0.13f, 0.05f, brakeColor);

    // Feu de pluie central : clignote façon F1 pendant un drift.
    bool rainOn = vis.drifting && std::fmod(t, 0.5f) < 0.30f;
    DrawCube(Vector3{0.0f, 0.33f, -2.215f}, 0.16f, 0.16f, 0.05f, rainOn ? Color{255, 70, 70, 255} : Color{96, 22, 22, 255});

    // --- Elements translucides (dessines en dernier pour le blending). ---
    if (vis.braking) {
        DrawCube(Vector3{-kBrakeX, kBrakeY, -2.24f}, 0.44f, 0.20f, 0.05f, Fade(Color{255, 45, 35, 255}, 0.30f));
        DrawCube(Vector3{kBrakeX, kBrakeY, -2.24f}, 0.44f, 0.20f, 0.05f, Fade(Color{255, 45, 35, 255}, 0.30f));
    }
    if (rainOn) {
        DrawCube(Vector3{0.0f, 0.33f, -2.26f}, 0.24f, 0.24f, 0.05f, Fade(Color{255, 70, 70, 255}, 0.35f));
    }
    if (vis.headlights) {
        DrawSphere(Vector3{-kHeadX, kHeadY, kHeadZ + 0.02f}, 0.085f, Fade(Color{255, 244, 200, 255}, 0.45f));
        DrawSphere(Vector3{kHeadX, kHeadY, kHeadZ + 0.02f}, 0.085f, Fade(Color{255, 244, 200, 255}, 0.45f));
        // Pointe des cones legerement sous le sol : le faisceau parait
        // absorbe par la piste au lieu de laisser un disque flottant.
        DrawCylinderEx(Vector3{-kHeadX, kHeadY, kHeadZ + 0.03f}, Vector3{-0.78f, -0.12f, 5.6f}, 0.05f, 0.48f, 10, Fade(Color{255, 238, 170, 255}, 0.13f));
        DrawCylinderEx(Vector3{kHeadX, kHeadY, kHeadZ + 0.03f}, Vector3{0.78f, -0.12f, 5.6f}, 0.05f, 0.48f, 10, Fade(Color{255, 238, 170, 255}, 0.13f));
    }
    if (vis.nitro) {
        float phase = car.position.x * 3.7f + car.position.z * 2.9f;
        DrawExhaustFlame(-kExhaustX, t, phase);
        DrawExhaustFlame(kExhaustX, t, phase + 2.1f);
    }

    // Verriere teintee (habitacle) + pare-brise incline, en tout dernier.
    DrawCube(Vector3{0.0f, 0.80f, -0.42f}, 1.06f, 0.30f, 1.15f, Fade(Color{40, 58, 82, 255}, 0.55f));
    rlPushMatrix();
    rlTranslatef(0.0f, 0.76f, 0.28f);
    rlRotatef(-34.0f, 1.0f, 0.0f, 0.0f);
    DrawCube(Vector3{0.0f, 0.0f, 0.0f}, 1.02f, 0.44f, 0.05f, Fade(Color{50, 70, 96, 255}, 0.50f));
    rlPopMatrix();

    rlPopMatrix(); // roulis / tangage
    rlPopMatrix(); // position / cap
}

} // namespace racer
