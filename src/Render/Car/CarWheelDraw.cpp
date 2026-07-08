/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Car wheel and shadow drawing
*/

#include "Render/Car/CarWheelDraw.hpp"

#include <algorithm>
#include <cmath>

#include "rlgl.h"

#include "Render/Car/CarPrimitives.hpp"

namespace racer {

float CarWheelDraw::wrapAngle(float angle)
{
    while (angle > PI)
        angle -= 2.0f * PI;
    while (angle < -PI)
        angle += 2.0f * PI;
    return angle;
}

void CarWheelDraw::drawWheelSpokes()
{
    DrawCube(
        Vector3{0.0f, 0.0f, 0.0f},
        0.385f, 0.38f, 0.08f, Color{45, 45, 52, 255});
    DrawCube(
        Vector3{0.0f, 0.0f, 0.0f},
        0.385f, 0.08f, 0.38f, Color{45, 45, 52, 255});
    DrawCylinderExLit(
        Vector3{-0.20f, 0.0f, 0.0f},
        Vector3{0.20f, 0.0f, 0.0f},
        0.055f, 0.055f, 8, Color{225, 190, 90, 255});
}

void CarWheelDraw::drawWheel(
    Vector3 center, float steerDeg, float spinDeg)
{
    rlPushMatrix();
    rlTranslatef(center.x, center.y, center.z);
    rlRotatef(steerDeg, 0.0f, 1.0f, 0.0f);
    DrawCylinderExLit(
        Vector3{-0.17f, 0.0f, 0.0f},
        Vector3{0.17f, 0.0f, 0.0f},
        kWheelRadius, kWheelRadius, 14, Color{23, 23, 26, 255});
    DrawCylinderExLit(
        Vector3{-0.18f, 0.0f, 0.0f},
        Vector3{0.18f, 0.0f, 0.0f},
        0.225f, 0.225f, 12, Color{198, 200, 205, 255});
    rlRotatef(spinDeg, 1.0f, 0.0f, 0.0f);
    drawWheelSpokes();
    rlPopMatrix();
}

void CarWheelDraw::drawShadowDiscs()
{
    DrawCylinder(
        Vector3{0.0f, 0.040f, 0.0f},
        2.45f, 2.45f, 0.006f, 22, Fade(BLACK, 0.09f));
    DrawCylinder(
        Vector3{0.0f, 0.048f, 0.0f},
        2.05f, 2.05f, 0.006f, 22, Fade(BLACK, 0.12f));
    DrawCylinder(
        Vector3{0.0f, 0.056f, 0.0f},
        1.60f, 1.60f, 0.006f, 22, Fade(BLACK, 0.16f));
}

void CarWheelDraw::drawShadow(const Car &car, bool drifting)
{
    const float speedNorm = std::clamp(
        std::fabs(car.speed()) / 30.0f, 0.0f, 1.0f);
    const float stretch = 1.0f + 0.40f * speedNorm;
    const float slip = wrapAngle(car.velocityHeading() - car.heading());
    const float sideOffset = drifting
        ? std::clamp(-slip * 0.9f, -0.5f, 0.5f)
        : 0.0f;

    rlPushMatrix();
    rlTranslatef(car.position().x, car.position().y, car.position().z);
    rlRotatef(car.velocityHeading() * RAD2DEG, 0.0f, 1.0f, 0.0f);
    rlTranslatef(sideOffset, 0.0f, 0.0f);
    rlScalef(0.62f, 1.0f, stretch);
    // Decalque translucide au sol : pas d'ecriture de profondeur (meme
    // risque de rejet de la carrosserie que pour les halos).
    rlDrawRenderBatchActive();
    rlDisableDepthMask();
    drawShadowDiscs();
    rlDrawRenderBatchActive();
    rlEnableDepthMask();
    rlPopMatrix();
}

void CarWheelDraw::drawWheels(const CarVisual &vis)
{
    const float steerDeg = std::clamp(vis.steer, -1.0f, 1.0f) * 28.0f;
    const float spinDeg = vis.wheelSpin * RAD2DEG;

    drawWheel(Vector3{-0.88f, kWheelRadius, 1.45f}, steerDeg, spinDeg);
    drawWheel(Vector3{0.88f, kWheelRadius, 1.45f}, steerDeg, spinDeg);
    drawWheel(Vector3{-0.88f, kWheelRadius, -1.42f}, 0.0f, spinDeg);
    drawWheel(Vector3{0.88f, kWheelRadius, -1.42f}, 0.0f, spinDeg);
}

void CarWheelDraw::drawHeadlightGroundGlow()
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

void CarWheelDraw::drawNitroGroundGlow()
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

void CarWheelDraw::drawGroundEffects(const CarVisual &vis)
{
    // Halos au sol : decalques translucides, jamais d'ecriture de
    // profondeur (sinon leur depth rejette la carrosserie dessinee apres
    // et la voiture parait blanchie/fantome vue de derriere).
    // Flush du batch avant/apres : glDepthMask s'applique immediatement
    // alors que les sommets rlgl partent au prochain flush.
    rlDrawRenderBatchActive();
    rlDisableDepthMask();
    if (vis.headlights)
        drawHeadlightGroundGlow();
    if (vis.nitro)
        drawNitroGroundGlow();
    rlDrawRenderBatchActive();
    rlEnableDepthMask();
}

} // namespace racer
