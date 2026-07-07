/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Car visual rendering implementation
*/

#include "Render/CarRenderer.hpp"

#include <algorithm>
#include <cmath>

#include "rlgl.h"

namespace racer {

namespace {

constexpr float kHeadX = 0.48f;
constexpr float kHeadY = 0.44f;
constexpr float kHeadZ = 2.37f;
constexpr float kBrakeX = 0.55f;
constexpr float kBrakeY = 0.55f;
constexpr float kBrakeZ = -2.20f;
constexpr float kExhaustX = 0.17f;
constexpr float kExhaustY = 0.24f;
constexpr float kExhaustZ = -2.26f;

struct BodyPalette {
    Color body;
    Color stripe;
    Color carbon;
    Color carbonLight;
};

} // namespace

class CarRenderHelpers {
public:
    static BodyPalette makeBodyPalette(Color bodyColor);
    static Color shade(Color color, float factor);
    static float wrapAngle(float angle);
    static void drawWheelSpokes();
    static void drawWheel(Vector3 center, float steerDeg, float spinDeg);
    static void drawShadowDiscs();
    static void drawShadow(const Car &car, bool drifting);
    static void drawExhaustFlame(float x, float time, float phase);
    static void drawWheels(const CarVisual &vis);
    static void drawHeadlightGroundGlow();
    static void drawNitroGroundGlow();
    static void drawGroundEffects(const CarVisual &vis);
    static void pushChassisPose(const Car &car, const CarVisual &vis);
    static void popChassisPose();
    static void drawBodyShell(const BodyPalette &palette);
    static void drawWheelArches(const BodyPalette &palette);
    static void drawAeroTrim(const BodyPalette &palette);
    static void drawLivery(const BodyPalette &palette);
    static void drawRacePlates();
    static void drawFrontDetails(const BodyPalette &palette);
    static void drawSideIntakes();
    static void drawMirrors(const BodyPalette &palette);
    static void drawAntenna();
    static void drawRollCage(const BodyPalette &palette);
    static void drawDriver();
    static void drawRearWing(const BodyPalette &palette);
    static void drawExhaustPipes();
    static void drawChassisAssembly(const BodyPalette &palette);
    static void drawHeadlightFixtures(const CarVisual &vis);
    static void drawBrakeFixtures(const CarVisual &vis);
    static void drawOverlayEffects(
        const Car &car, const CarVisual &vis, float time);
    static void drawRainLight(const CarVisual &vis, float time);
    static void drawBrakeGlow(const CarVisual &vis);
    static void drawRainGlow(const CarVisual &vis, float time);
    static void drawHeadlightGlow(const CarVisual &vis);
    static void drawNitroFlames(const Car &car, float time);
    static void drawWindshield();
};

BodyPalette CarRenderHelpers::makeBodyPalette(Color bodyColor)
{
    const float lum = 0.299f * bodyColor.r + 0.587f * bodyColor.g
        + 0.114f * bodyColor.b;
    const Color stripe = (lum > 150.0f)
        ? Color{30, 30, 36, 255}
        : Color{245, 245, 242, 255};

    return BodyPalette{
        bodyColor,
        stripe,
        Color{36, 36, 42, 255},
        Color{52, 52, 58, 255},
    };
}

Color CarRenderHelpers::shade(Color color, float factor)
{
    auto channel = [factor](unsigned char value) {
        float scaled = static_cast<float>(value) * factor;

        scaled = std::clamp(scaled, 0.0f, 255.0f);
        return static_cast<unsigned char>(scaled);
    };

    return Color{
        channel(color.r),
        channel(color.g),
        channel(color.b),
        color.a,
    };
}

float CarRenderHelpers::wrapAngle(float angle)
{
    while (angle > PI)
        angle -= 2.0f * PI;
    while (angle < -PI)
        angle += 2.0f * PI;
    return angle;
}

void CarRenderHelpers::drawWheelSpokes()
{
    DrawCube(
        Vector3{0.0f, 0.0f, 0.0f},
        0.385f, 0.38f, 0.08f, Color{45, 45, 52, 255});
    DrawCube(
        Vector3{0.0f, 0.0f, 0.0f},
        0.385f, 0.08f, 0.38f, Color{45, 45, 52, 255});
    DrawCylinderEx(
        Vector3{-0.20f, 0.0f, 0.0f},
        Vector3{0.20f, 0.0f, 0.0f},
        0.055f, 0.055f, 8, Color{225, 190, 90, 255});
}

void CarRenderHelpers::drawWheel(
    Vector3 center, float steerDeg, float spinDeg)
{
    rlPushMatrix();
    rlTranslatef(center.x, center.y, center.z);
    rlRotatef(steerDeg, 0.0f, 1.0f, 0.0f);
    DrawCylinderEx(
        Vector3{-0.17f, 0.0f, 0.0f},
        Vector3{0.17f, 0.0f, 0.0f},
        kWheelRadius, kWheelRadius, 14, Color{23, 23, 26, 255});
    DrawCylinderEx(
        Vector3{-0.18f, 0.0f, 0.0f},
        Vector3{0.18f, 0.0f, 0.0f},
        0.225f, 0.225f, 12, Color{198, 200, 205, 255});
    rlRotatef(spinDeg, 1.0f, 0.0f, 0.0f);
    drawWheelSpokes();
    rlPopMatrix();
}

void CarRenderHelpers::drawShadowDiscs()
{
    DrawCylinder(
        Vector3{0.0f, 0.010f, 0.0f},
        2.45f, 2.45f, 0.006f, 22, Fade(BLACK, 0.09f));
    DrawCylinder(
        Vector3{0.0f, 0.018f, 0.0f},
        2.05f, 2.05f, 0.006f, 22, Fade(BLACK, 0.12f));
    DrawCylinder(
        Vector3{0.0f, 0.026f, 0.0f},
        1.60f, 1.60f, 0.006f, 22, Fade(BLACK, 0.16f));
}

void CarRenderHelpers::drawShadow(const Car &car, bool drifting)
{
    const float speedNorm = std::clamp(
        std::fabs(car.speed()) / 30.0f, 0.0f, 1.0f);
    const float stretch = 1.0f + 0.40f * speedNorm;
    const float slip = wrapAngle(car.velocityHeading() - car.heading());
    const float sideOffset = drifting
        ? std::clamp(-slip * 0.9f, -0.5f, 0.5f)
        : 0.0f;

    rlPushMatrix();
    rlTranslatef(car.position().x, 0.0f, car.position().z);
    rlRotatef(car.velocityHeading() * RAD2DEG, 0.0f, 1.0f, 0.0f);
    rlTranslatef(sideOffset, 0.0f, 0.0f);
    rlScalef(0.62f, 1.0f, stretch);
    drawShadowDiscs();
    rlPopMatrix();
}

void CarRenderHelpers::drawExhaustFlame(
    float x, float time, float phase)
{
    const float flicker = 0.30f * std::sin(time * 40.0f + phase)
        + 0.14f * std::sin(time * 23.7f + phase * 1.7f);
    const float len = std::clamp(1.05f + flicker, 0.55f, 1.5f);
    const Vector3 base{x, kExhaustY, kExhaustZ + 0.02f};
    const Vector3 tipOuter{x, kExhaustY + 0.06f, kExhaustZ - len};
    const Vector3 tipMid{
        x, kExhaustY + 0.03f, kExhaustZ - len * 0.78f};
    const Vector3 tipCore{
        x, kExhaustY + 0.01f, kExhaustZ - len * 0.50f};

    DrawCylinderEx(
        base, tipCore, 0.048f, 0.010f, 8,
        Fade(Color{190, 230, 255, 255}, 0.95f));
    DrawCylinderEx(
        base, tipMid, 0.075f, 0.012f, 8,
        Fade(Color{255, 210, 90, 255}, 0.60f));
    DrawCylinderEx(
        base, tipOuter, 0.105f, 0.014f, 8,
        Fade(Color{255, 120, 30, 255}, 0.42f));
}

void CarRenderHelpers::drawWheels(const CarVisual &vis)
{
    const float steerDeg = std::clamp(vis.steer, -1.0f, 1.0f) * 28.0f;
    const float spinDeg = vis.wheelSpin * RAD2DEG;

    drawWheel(Vector3{-0.88f, kWheelRadius, 1.45f}, steerDeg, spinDeg);
    drawWheel(Vector3{0.88f, kWheelRadius, 1.45f}, steerDeg, spinDeg);
    drawWheel(Vector3{-0.88f, kWheelRadius, -1.42f}, 0.0f, spinDeg);
    drawWheel(Vector3{0.88f, kWheelRadius, -1.42f}, 0.0f, spinDeg);
}

void CarRenderHelpers::drawHeadlightGroundGlow()
{
    rlPushMatrix();
    rlTranslatef(0.0f, 0.02f, 5.0f);
    rlScalef(1.5f, 1.0f, 2.3f);
    DrawCylinder(
        Vector3{0.0f, 0.0f, 0.0f},
        0.62f, 0.62f, 0.005f, 18,
        Fade(Color{255, 240, 180, 255}, 0.10f));
    DrawCylinder(
        Vector3{0.0f, 0.006f, 0.0f},
        0.36f, 0.36f, 0.005f, 18,
        Fade(Color{255, 246, 205, 255}, 0.10f));
    rlPopMatrix();
}

void CarRenderHelpers::drawNitroGroundGlow()
{
    rlPushMatrix();
    rlTranslatef(0.0f, 0.02f, -2.95f);
    rlScalef(1.35f, 1.0f, 1.8f);
    DrawCylinder(
        Vector3{0.0f, 0.0f, 0.0f},
        0.55f, 0.55f, 0.005f, 16,
        Fade(Color{255, 150, 60, 255}, 0.16f));
    rlPopMatrix();
}

void CarRenderHelpers::drawGroundEffects(const CarVisual &vis)
{
    if (vis.headlights)
        drawHeadlightGroundGlow();
    if (vis.nitro)
        drawNitroGroundGlow();
}

void CarRenderHelpers::pushChassisPose(
    const Car &car, const CarVisual &vis)
{
    const float speedFactor = std::clamp(
        std::fabs(car.speed()) / 12.0f, 0.0f, 1.0f);
    const float rollDeg = std::clamp(vis.steer, -1.0f, 1.0f)
        * (vis.drifting ? 5.2f : 3.4f) * speedFactor;
    const float pitchDeg = (vis.braking ? 2.1f : 0.0f)
        + (vis.nitro ? -1.4f : 0.0f);

    rlPushMatrix();
    rlTranslatef(0.0f, 0.34f, 0.0f);
    rlRotatef(rollDeg, 0.0f, 0.0f, 1.0f);
    rlRotatef(pitchDeg, 1.0f, 0.0f, 0.0f);
    rlTranslatef(0.0f, -0.34f, 0.0f);
}

void CarRenderHelpers::popChassisPose()
{
    rlPopMatrix();
}

void CarRenderHelpers::drawBodyShell(const BodyPalette &palette)
{
    DrawCube(
        Vector3{0.0f, 0.20f, -0.05f},
        1.70f, 0.22f, 4.10f, shade(palette.body, 0.32f));
    DrawCube(
        Vector3{0.0f, 0.44f, 0.10f},
        1.78f, 0.30f, 3.60f, palette.body);
    DrawCube(
        Vector3{0.0f, 0.42f, 1.95f},
        1.44f, 0.22f, 0.85f, shade(palette.body, 1.10f));
    DrawCube(
        Vector3{0.0f, 0.36f, 2.42f},
        1.02f, 0.16f, 0.34f, shade(palette.body, 1.16f));
    DrawCube(
        Vector3{0.0f, 0.52f, -1.35f},
        1.72f, 0.34f, 0.95f, shade(palette.body, 0.90f));
    DrawCube(
        Vector3{0.0f, 0.47f, -1.98f},
        1.70f, 0.40f, 0.42f, shade(palette.body, 0.72f));
}

void CarRenderHelpers::drawWheelArches(const BodyPalette &palette)
{
    DrawCube(
        Vector3{-0.82f, 0.63f, 1.45f},
        0.40f, 0.10f, 0.98f, shade(palette.body, 1.04f));
    DrawCube(
        Vector3{0.82f, 0.63f, 1.45f},
        0.40f, 0.10f, 0.98f, shade(palette.body, 1.04f));
    DrawCube(
        Vector3{-0.82f, 0.63f, -1.42f},
        0.40f, 0.10f, 0.98f, shade(palette.body, 0.96f));
    DrawCube(
        Vector3{0.82f, 0.63f, -1.42f},
        0.40f, 0.10f, 0.98f, shade(palette.body, 0.96f));
}

void CarRenderHelpers::drawAeroTrim(const BodyPalette &palette)
{
    DrawCube(
        Vector3{0.0f, 0.15f, 2.35f},
        1.58f, 0.09f, 0.35f, palette.carbon);
    DrawCube(
        Vector3{0.0f, 0.16f, -2.22f},
        1.46f, 0.13f, 0.24f, palette.carbon);
    DrawCube(
        Vector3{-0.42f, 0.20f, -2.22f},
        0.045f, 0.20f, 0.22f, palette.carbonLight);
    DrawCube(
        Vector3{0.0f, 0.20f, -2.22f},
        0.045f, 0.20f, 0.22f, palette.carbonLight);
    DrawCube(
        Vector3{0.42f, 0.20f, -2.22f},
        0.045f, 0.20f, 0.22f, palette.carbonLight);
}

void CarRenderHelpers::drawLivery(const BodyPalette &palette)
{
    DrawCube(
        Vector3{-0.17f, 0.598f, 0.10f},
        0.15f, 0.016f, 3.55f, palette.stripe);
    DrawCube(
        Vector3{0.17f, 0.598f, 0.10f},
        0.15f, 0.016f, 3.55f, palette.stripe);
    DrawCube(
        Vector3{-0.17f, 0.538f, 1.95f},
        0.15f, 0.016f, 0.80f, palette.stripe);
    DrawCube(
        Vector3{0.17f, 0.538f, 1.95f},
        0.15f, 0.016f, 0.80f, palette.stripe);
    DrawCube(
        Vector3{-0.17f, 0.698f, -1.35f},
        0.15f, 0.016f, 0.90f, palette.stripe);
    DrawCube(
        Vector3{0.17f, 0.698f, -1.35f},
        0.15f, 0.016f, 0.90f, palette.stripe);
}

void CarRenderHelpers::drawRacePlates()
{
    DrawCube(
        Vector3{-0.90f, 0.47f, 0.25f},
        0.025f, 0.30f, 0.44f, Color{242, 242, 238, 255});
    DrawCube(
        Vector3{0.90f, 0.47f, 0.25f},
        0.025f, 0.30f, 0.44f, Color{242, 242, 238, 255});
    DrawCube(
        Vector3{-0.918f, 0.47f, 0.25f},
        0.012f, 0.17f, 0.09f, Color{30, 30, 34, 255});
    DrawCube(
        Vector3{0.918f, 0.47f, 0.25f},
        0.012f, 0.17f, 0.09f, Color{30, 30, 34, 255});
}

void CarRenderHelpers::drawFrontDetails(const BodyPalette &palette)
{
    DrawCube(
        Vector3{0.0f, 0.64f, 0.85f},
        0.34f, 0.11f, 0.44f, palette.carbonLight);
    DrawCube(
        Vector3{0.0f, 0.645f, 1.08f},
        0.24f, 0.07f, 0.02f, Color{12, 12, 14, 255});
    DrawCube(
        Vector3{0.0f, 0.33f, 2.575f},
        0.62f, 0.10f, 0.05f, Color{14, 14, 16, 255});
}

void CarRenderHelpers::drawSideIntakes()
{
    DrawCube(
        Vector3{-0.905f, 0.44f, -0.62f},
        0.05f, 0.18f, 0.46f, Color{14, 14, 16, 255});
    DrawCube(
        Vector3{0.905f, 0.44f, -0.62f},
        0.05f, 0.18f, 0.46f, Color{14, 14, 16, 255});
}

void CarRenderHelpers::drawMirrors(const BodyPalette &palette)
{
    DrawCube(
        Vector3{-0.78f, 0.74f, 0.22f},
        0.14f, 0.035f, 0.035f, palette.carbon);
    DrawCube(
        Vector3{0.78f, 0.74f, 0.22f},
        0.14f, 0.035f, 0.035f, palette.carbon);
    DrawCube(
        Vector3{-0.92f, 0.78f, 0.22f},
        0.16f, 0.09f, 0.05f, palette.carbonLight);
    DrawCube(
        Vector3{0.92f, 0.78f, 0.22f},
        0.16f, 0.09f, 0.05f, palette.carbonLight);
}

void CarRenderHelpers::drawAntenna()
{
    DrawCylinderEx(
        Vector3{0.32f, 0.69f, -1.30f},
        Vector3{0.40f, 1.02f, -1.42f},
        0.012f, 0.012f, 6, Color{25, 25, 28, 255});
    DrawSphere(
        Vector3{0.40f, 1.02f, -1.42f}, 0.024f, Color{25, 25, 28, 255});
}

void CarRenderHelpers::drawRollCage(const BodyPalette &palette)
{
    DrawCube(
        Vector3{0.0f, 1.00f, -0.90f},
        0.72f, 0.07f, 0.07f, palette.carbonLight);
    DrawCube(
        Vector3{-0.325f, 0.90f, -0.90f},
        0.07f, 0.24f, 0.07f, palette.carbonLight);
    DrawCube(
        Vector3{0.325f, 0.90f, -0.90f},
        0.07f, 0.24f, 0.07f, palette.carbonLight);
}

void CarRenderHelpers::drawDriver()
{
    DrawCube(
        Vector3{0.0f, 0.70f, -0.62f},
        0.46f, 0.16f, 0.26f, Color{40, 40, 45, 255});
    DrawSphere(
        Vector3{0.0f, 0.82f, -0.55f}, 0.13f, Color{252, 216, 80, 255});
}

void CarRenderHelpers::drawRearWing(const BodyPalette &palette)
{
    DrawCube(
        Vector3{-0.48f, 0.80f, -2.02f},
        0.09f, 0.26f, 0.13f, palette.carbonLight);
    DrawCube(
        Vector3{0.48f, 0.80f, -2.02f},
        0.09f, 0.26f, 0.13f, palette.carbonLight);
    DrawCube(
        Vector3{0.0f, 0.97f, -2.05f},
        1.76f, 0.07f, 0.52f, shade(palette.body, 0.55f));
    DrawCube(
        Vector3{-0.885f, 0.95f, -2.05f},
        0.05f, 0.24f, 0.58f, palette.carbon);
    DrawCube(
        Vector3{0.885f, 0.95f, -2.05f},
        0.05f, 0.24f, 0.58f, palette.carbon);
}

void CarRenderHelpers::drawExhaustPipes()
{
    DrawCylinderEx(
        Vector3{-kExhaustX, kExhaustY, -2.05f},
        Vector3{-kExhaustX, kExhaustY, kExhaustZ},
        0.055f, 0.055f, 8, Color{35, 35, 38, 255});
    DrawCylinderEx(
        Vector3{kExhaustX, kExhaustY, -2.05f},
        Vector3{kExhaustX, kExhaustY, kExhaustZ},
        0.055f, 0.055f, 8, Color{35, 35, 38, 255});
}

void CarRenderHelpers::drawChassisAssembly(const BodyPalette &palette)
{
    drawBodyShell(palette);
    drawWheelArches(palette);
    drawAeroTrim(palette);
    drawLivery(palette);
    drawRacePlates();
    drawFrontDetails(palette);
    drawSideIntakes();
    drawMirrors(palette);
    drawAntenna();
    drawRollCage(palette);
    drawDriver();
    drawRearWing(palette);
    drawExhaustPipes();
}

void CarRenderHelpers::drawHeadlightFixtures(const CarVisual &vis)
{
    DrawCube(
        Vector3{-kHeadX, kHeadY, kHeadZ - 0.012f},
        0.38f, 0.16f, 0.05f, Color{20, 20, 22, 255});
    DrawCube(
        Vector3{kHeadX, kHeadY, kHeadZ - 0.012f},
        0.38f, 0.16f, 0.05f, Color{20, 20, 22, 255});
    const Color headColor = vis.headlights
        ? Color{255, 252, 235, 255}
        : Color{248, 238, 205, 255};

    DrawCube(
        Vector3{-kHeadX, kHeadY, kHeadZ},
        0.30f, 0.11f, 0.06f, headColor);
    DrawCube(
        Vector3{kHeadX, kHeadY, kHeadZ},
        0.30f, 0.11f, 0.06f, headColor);
}

void CarRenderHelpers::drawBrakeFixtures(const CarVisual &vis)
{
    DrawCube(
        Vector3{0.0f, kBrakeY, kBrakeZ - 0.005f},
        1.58f, 0.20f, 0.04f, Color{18, 18, 20, 255});
    const Color brakeColor = vis.braking
        ? Color{255, 46, 36, 255}
        : Color{110, 18, 18, 255};

    DrawCube(
        Vector3{-kBrakeX, kBrakeY, kBrakeZ},
        0.36f, 0.13f, 0.05f, brakeColor);
    DrawCube(
        Vector3{kBrakeX, kBrakeY, kBrakeZ},
        0.36f, 0.13f, 0.05f, brakeColor);
}

void CarRenderHelpers::drawOverlayEffects(
    const Car &car, const CarVisual &vis, float time)
{
    drawHeadlightFixtures(vis);
    drawBrakeFixtures(vis);
    drawRainLight(vis, time);
    drawBrakeGlow(vis);
    drawRainGlow(vis, time);
    if (vis.headlights)
        drawHeadlightGlow(vis);
    if (vis.nitro)
        drawNitroFlames(car, time);
    drawWindshield();
}

void CarRenderHelpers::drawRainLight(const CarVisual &vis, float time)
{
    const bool rainOn = vis.drifting && std::fmod(time, 0.5f) < 0.30f;
    const Color rainColor = rainOn
        ? Color{255, 70, 70, 255}
        : Color{96, 22, 22, 255};

    DrawCube(
        Vector3{0.0f, 0.33f, -2.215f},
        0.16f, 0.16f, 0.05f, rainColor);
}

void CarRenderHelpers::drawBrakeGlow(const CarVisual &vis)
{
    if (!vis.braking)
        return;
    DrawCube(
        Vector3{-kBrakeX, kBrakeY, -2.24f},
        0.44f, 0.20f, 0.05f, Fade(Color{255, 45, 35, 255}, 0.30f));
    DrawCube(
        Vector3{kBrakeX, kBrakeY, -2.24f},
        0.44f, 0.20f, 0.05f, Fade(Color{255, 45, 35, 255}, 0.30f));
}

void CarRenderHelpers::drawRainGlow(const CarVisual &vis, float time)
{
    const bool rainOn = vis.drifting && std::fmod(time, 0.5f) < 0.30f;

    if (!rainOn)
        return;
    DrawCube(
        Vector3{0.0f, 0.33f, -2.26f},
        0.24f, 0.24f, 0.05f, Fade(Color{255, 70, 70, 255}, 0.35f));
}

void CarRenderHelpers::drawHeadlightGlow(const CarVisual &vis)
{
    if (!vis.headlights)
        return;
    DrawSphere(
        Vector3{-kHeadX, kHeadY, kHeadZ + 0.02f},
        0.085f, Fade(Color{255, 244, 200, 255}, 0.45f));
    DrawSphere(
        Vector3{kHeadX, kHeadY, kHeadZ + 0.02f},
        0.085f, Fade(Color{255, 244, 200, 255}, 0.45f));
    DrawCylinderEx(
        Vector3{-kHeadX, kHeadY, kHeadZ + 0.03f},
        Vector3{-0.78f, -0.12f, 5.6f},
        0.05f, 0.48f, 10, Fade(Color{255, 238, 170, 255}, 0.13f));
    DrawCylinderEx(
        Vector3{kHeadX, kHeadY, kHeadZ + 0.03f},
        Vector3{0.78f, -0.12f, 5.6f},
        0.05f, 0.48f, 10, Fade(Color{255, 238, 170, 255}, 0.13f));
}

void CarRenderHelpers::drawNitroFlames(const Car &car, float time)
{
    const float phase = car.position().x * 3.7f + car.position().z * 2.9f;

    drawExhaustFlame(-kExhaustX, time, phase);
    drawExhaustFlame(kExhaustX, time, phase + 2.1f);
}

void CarRenderHelpers::drawWindshield()
{
    DrawCube(
        Vector3{0.0f, 0.80f, -0.42f},
        1.06f, 0.30f, 1.15f, Fade(Color{40, 58, 82, 255}, 0.55f));
    rlPushMatrix();
    rlTranslatef(0.0f, 0.76f, 0.28f);
    rlRotatef(-34.0f, 1.0f, 0.0f, 0.0f);
    DrawCube(
        Vector3{0.0f, 0.0f, 0.0f},
        1.02f, 0.44f, 0.05f, Fade(Color{50, 70, 96, 255}, 0.50f));
    rlPopMatrix();
}

CarLightPoints CarRenderer::getCarLightPoints(const Car &car)
{
    const float c = std::cos(car.heading());
    const float s = std::sin(car.heading());
    auto toWorld = [&](float lx, float ly, float lz) {
        return Vector3{
            car.position().x + lx * c + lz * s,
            car.position().y + ly,
            car.position().z - lx * s + lz * c,
        };
    };
    CarLightPoints points{};

    points.headL = toWorld(-kHeadX, kHeadY, kHeadZ);
    points.headR = toWorld(kHeadX, kHeadY, kHeadZ);
    points.brakeL = toWorld(-kBrakeX, kBrakeY, kBrakeZ);
    points.brakeR = toWorld(kBrakeX, kBrakeY, kBrakeZ);
    points.exhaust = toWorld(0.0f, kExhaustY, kExhaustZ);
    return points;
}

void CarRenderer::drawCar(const Car &car, Color bodyColor)
{
    drawCarEx(car, CarVisual{}, bodyColor);
}

void CarRenderer::drawCarEx(const Car &car, const CarVisual &vis,
    Color bodyColor)
{
    const float time = static_cast<float>(GetTime());
    const BodyPalette palette = CarRenderHelpers::makeBodyPalette(bodyColor);

    CarRenderHelpers::drawShadow(car, vis.drifting);
    rlPushMatrix();
    rlTranslatef(car.position().x, car.position().y, car.position().z);
    rlRotatef(car.heading() * RAD2DEG, 0.0f, 1.0f, 0.0f);
    CarRenderHelpers::drawWheels(vis);
    CarRenderHelpers::drawGroundEffects(vis);
    CarRenderHelpers::pushChassisPose(car, vis);
    CarRenderHelpers::drawChassisAssembly(palette);
    CarRenderHelpers::drawOverlayEffects(car, vis, time);
    CarRenderHelpers::popChassisPose();
    rlPopMatrix();
}

} // namespace racer
