/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Car visual rendering facade
*/

#include "Render/Car/CarRenderer.hpp"

#include <cmath>

#include "rlgl.h"

#include "Render/Car/CarBodyDraw.hpp"
#include "Render/Car/CarWheelDraw.hpp"

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
    const CarBodyPalette palette = CarBodyDraw::makeBodyPalette(bodyColor);

    CarWheelDraw::drawShadow(car, vis.drifting);
    rlPushMatrix();
    rlTranslatef(car.position().x, car.position().y, car.position().z);
    rlRotatef(car.heading() * RAD2DEG, 0.0f, 1.0f, 0.0f);
    CarWheelDraw::drawWheels(vis);
    CarWheelDraw::drawGroundEffects(vis);
    CarBodyDraw::pushChassisPose(car, vis);
    CarBodyDraw::drawChassisAssembly(palette);
    CarBodyDraw::drawOverlayEffects(car, vis, time);
    CarBodyDraw::popChassisPose();
    rlPopMatrix();
}

} // namespace racer
