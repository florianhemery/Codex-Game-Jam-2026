/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Car body and lighting draw pass
*/

#include "Render/Car/CarBodyDraw.hpp"

#include <algorithm>
#include <cmath>

#include "Render/Car/CarDraw.hpp"
#include "rlgl.h"

#include "Render/Car/CarPrimitives.hpp"

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

} // namespace

CarBodyPalette CarBodyDraw::makeBodyPalette(Color bodyColor)
{
    const float lum = 0.299f * bodyColor.r + 0.587f * bodyColor.g
        + 0.114f * bodyColor.b;
    const Color stripe = (lum > 150.0f)
        ? Color{30, 30, 36, 255}
        : Color{245, 245, 242, 255};

    return CarBodyPalette{
        bodyColor,
        stripe,
        Color{36, 36, 42, 255},
        Color{52, 52, 58, 255},
    };
}

Color CarBodyDraw::shade(Color color, float factor)
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
void CarBodyDraw::pushChassisPose(
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

void CarBodyDraw::popChassisPose()
{
    rlPopMatrix();
}

void CarBodyDraw::drawBodyShell(const CarBodyPalette &palette)
{
    carDraw::cube(
        Vector3{0.0f, 0.20f, -0.05f},
        1.70f, 0.22f, 4.10f, shade(palette.body, 0.32f));
    carDraw::cube(
        Vector3{0.0f, 0.44f, 0.10f},
        1.78f, 0.30f, 3.60f, palette.body);
    carDraw::cube(
        Vector3{0.0f, 0.42f, 1.95f},
        1.44f, 0.22f, 0.85f, shade(palette.body, 1.06f));
    carDraw::cube(
        Vector3{0.0f, 0.36f, 2.42f},
        1.02f, 0.16f, 0.34f, shade(palette.body, 1.08f));
    carDraw::cube(
        Vector3{0.0f, 0.52f, -1.35f},
        1.72f, 0.34f, 0.95f, shade(palette.body, 0.90f));
    carDraw::cube(
        Vector3{0.0f, 0.47f, -1.98f},
        1.70f, 0.40f, 0.42f, shade(palette.body, 0.72f));
}

void CarBodyDraw::drawWheelArches(const CarBodyPalette &palette)
{
    carDraw::cube(
        Vector3{-0.82f, 0.63f, 1.45f},
        0.40f, 0.10f, 0.98f, shade(palette.body, 1.04f));
    carDraw::cube(
        Vector3{0.82f, 0.63f, 1.45f},
        0.40f, 0.10f, 0.98f, shade(palette.body, 1.04f));
    carDraw::cube(
        Vector3{-0.82f, 0.63f, -1.42f},
        0.40f, 0.10f, 0.98f, shade(palette.body, 0.96f));
    carDraw::cube(
        Vector3{0.82f, 0.63f, -1.42f},
        0.40f, 0.10f, 0.98f, shade(palette.body, 0.96f));
}

void CarBodyDraw::drawAeroTrim(const CarBodyPalette &palette)
{
    carDraw::cube(
        Vector3{0.0f, 0.15f, 2.35f},
        1.58f, 0.09f, 0.35f, palette.carbon);
    carDraw::cube(
        Vector3{0.0f, 0.16f, -2.22f},
        1.46f, 0.13f, 0.24f, palette.carbon);
    carDraw::cube(
        Vector3{-0.42f, 0.20f, -2.22f},
        0.045f, 0.20f, 0.22f, palette.carbonLight);
    carDraw::cube(
        Vector3{0.0f, 0.20f, -2.22f},
        0.045f, 0.20f, 0.22f, palette.carbonLight);
    carDraw::cube(
        Vector3{0.42f, 0.20f, -2.22f},
        0.045f, 0.20f, 0.22f, palette.carbonLight);
}

void CarBodyDraw::drawLivery(const CarBodyPalette &palette)
{
    carDraw::cube(
        Vector3{-0.17f, 0.598f, 0.10f},
        0.15f, 0.016f, 3.55f, palette.stripe);
    carDraw::cube(
        Vector3{0.17f, 0.598f, 0.10f},
        0.15f, 0.016f, 3.55f, palette.stripe);
    carDraw::cube(
        Vector3{-0.17f, 0.538f, 1.95f},
        0.15f, 0.016f, 0.80f, palette.stripe);
    carDraw::cube(
        Vector3{0.17f, 0.538f, 1.95f},
        0.15f, 0.016f, 0.80f, palette.stripe);
    carDraw::cube(
        Vector3{-0.17f, 0.698f, -1.35f},
        0.15f, 0.016f, 0.90f, palette.stripe);
    carDraw::cube(
        Vector3{0.17f, 0.698f, -1.35f},
        0.15f, 0.016f, 0.90f, palette.stripe);
}

void CarBodyDraw::drawRacePlates()
{
    carDraw::cube(
        Vector3{-0.90f, 0.47f, 0.25f},
        0.025f, 0.30f, 0.44f, Color{242, 242, 238, 255});
    carDraw::cube(
        Vector3{0.90f, 0.47f, 0.25f},
        0.025f, 0.30f, 0.44f, Color{242, 242, 238, 255});
    carDraw::cube(
        Vector3{-0.918f, 0.47f, 0.25f},
        0.012f, 0.17f, 0.09f, Color{30, 30, 34, 255});
    carDraw::cube(
        Vector3{0.918f, 0.47f, 0.25f},
        0.012f, 0.17f, 0.09f, Color{30, 30, 34, 255});
}

void CarBodyDraw::drawFrontDetails(const CarBodyPalette &palette)
{
    carDraw::cube(
        Vector3{0.0f, 0.64f, 0.85f},
        0.34f, 0.11f, 0.44f, palette.carbonLight);
    carDraw::cube(
        Vector3{0.0f, 0.645f, 1.08f},
        0.24f, 0.07f, 0.02f, Color{12, 12, 14, 255});
    carDraw::cube(
        Vector3{0.0f, 0.33f, 2.575f},
        0.62f, 0.10f, 0.05f, Color{14, 14, 16, 255});
}

void CarBodyDraw::drawSideIntakes()
{
    carDraw::cube(
        Vector3{-0.905f, 0.44f, -0.62f},
        0.05f, 0.18f, 0.46f, Color{14, 14, 16, 255});
    carDraw::cube(
        Vector3{0.905f, 0.44f, -0.62f},
        0.05f, 0.18f, 0.46f, Color{14, 14, 16, 255});
}

void CarBodyDraw::drawMirrors(const CarBodyPalette &palette)
{
    carDraw::cube(
        Vector3{-0.78f, 0.74f, 0.22f},
        0.14f, 0.035f, 0.035f, palette.carbon);
    carDraw::cube(
        Vector3{0.78f, 0.74f, 0.22f},
        0.14f, 0.035f, 0.035f, palette.carbon);
    carDraw::cube(
        Vector3{-0.92f, 0.78f, 0.22f},
        0.16f, 0.09f, 0.05f, palette.carbonLight);
    carDraw::cube(
        Vector3{0.92f, 0.78f, 0.22f},
        0.16f, 0.09f, 0.05f, palette.carbonLight);
}

void CarBodyDraw::drawAntenna()
{
    DrawCylinderExLit(
        Vector3{0.32f, 0.69f, -1.30f},
        Vector3{0.40f, 1.02f, -1.42f},
        0.012f, 0.012f, 6, Color{25, 25, 28, 255});
    DrawSphereLit(
        Vector3{0.40f, 1.02f, -1.42f}, 0.024f, 8, 8, Color{25, 25, 28, 255});
}

void CarBodyDraw::drawRollCage(const CarBodyPalette &palette)
{
    carDraw::cube(
        Vector3{0.0f, 1.00f, -0.90f},
        0.72f, 0.07f, 0.07f, palette.carbonLight);
    carDraw::cube(
        Vector3{-0.325f, 0.90f, -0.90f},
        0.07f, 0.24f, 0.07f, palette.carbonLight);
    carDraw::cube(
        Vector3{0.325f, 0.90f, -0.90f},
        0.07f, 0.24f, 0.07f, palette.carbonLight);
}

void CarBodyDraw::drawDriver()
{
    carDraw::cube(
        Vector3{0.0f, 0.70f, -0.62f},
        0.46f, 0.16f, 0.26f, Color{40, 40, 45, 255});
    DrawSphereLit(
        Vector3{0.0f, 0.82f, -0.55f}, 0.13f, 8, 8, Color{252, 216, 80, 255});
}

void CarBodyDraw::drawRearWing(const CarBodyPalette &palette)
{
    carDraw::cube(
        Vector3{-0.48f, 0.80f, -2.02f},
        0.09f, 0.26f, 0.13f, palette.carbonLight);
    carDraw::cube(
        Vector3{0.48f, 0.80f, -2.02f},
        0.09f, 0.26f, 0.13f, palette.carbonLight);
    carDraw::cube(
        Vector3{0.0f, 0.97f, -2.05f},
        1.76f, 0.07f, 0.52f, shade(palette.body, 0.55f));
    carDraw::cube(
        Vector3{-0.885f, 0.95f, -2.05f},
        0.05f, 0.24f, 0.58f, palette.carbon);
    carDraw::cube(
        Vector3{0.885f, 0.95f, -2.05f},
        0.05f, 0.24f, 0.58f, palette.carbon);
}

void CarBodyDraw::drawExhaustPipes()
{
    DrawCylinderExLit(
        Vector3{-kExhaustX, kExhaustY, -2.05f},
        Vector3{-kExhaustX, kExhaustY, kExhaustZ},
        0.055f, 0.055f, 8, Color{35, 35, 38, 255});
    DrawCylinderExLit(
        Vector3{kExhaustX, kExhaustY, -2.05f},
        Vector3{kExhaustX, kExhaustY, kExhaustZ},
        0.055f, 0.055f, 8, Color{35, 35, 38, 255});
}

} // namespace racer

