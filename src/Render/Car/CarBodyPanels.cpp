/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Car body panel and fixture drawing
*/

#include "Render/Car/CarBodyDraw.hpp"
#include "Render/CarVisual.hpp"

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
} // namespace

void CarBodyDraw::drawChassisAssembly(const CarBodyPalette &palette)
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

void CarBodyDraw::drawHeadlightFixtures(const CarVisual &vis)
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

void CarBodyDraw::drawBrakeFixtures(const CarVisual &vis)
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

void CarBodyDraw::drawOverlayEffects(
    const Car &car, const CarVisual &vis, float time)
{
    drawHeadlightFixtures(vis);
    drawBrakeFixtures(vis);
    drawRainLight(vis, time);
    drawBrakeGlow(vis);
    drawRainGlow(vis, time);
    // Les effets translucides ne doivent pas ecrire la profondeur : dessines
    // entre la camera et la voiture, leurs valeurs de depth rejetaient les
    // fragments de la carrosserie (voiture blanchie/fantome sous nitro).
    // Flush du batch avant/apres : glDepthMask s'applique immediatement
    // alors que les sommets rlgl partent au prochain flush.
    rlDrawRenderBatchActive();
    rlDisableDepthMask();
    if (vis.headlights)
        drawHeadlightGlow(vis);
    if (vis.nitro)
        drawNitroFlames(car, time);
    drawWindshield();
    rlDrawRenderBatchActive();
    rlEnableDepthMask();
}

void CarBodyDraw::drawRainLight(const CarVisual &vis, float time)
{
    const bool rainOn = vis.drifting && std::fmod(time, 0.5f) < 0.30f;
    const Color rainColor = rainOn
        ? Color{255, 70, 70, 255}
        : Color{96, 22, 22, 255};

    DrawCube(
        Vector3{0.0f, 0.33f, -2.215f},
        0.16f, 0.16f, 0.05f, rainColor);
}

void CarBodyDraw::drawBrakeGlow(const CarVisual &vis)
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

void CarBodyDraw::drawRainGlow(const CarVisual &vis, float time)
{
    const bool rainOn = vis.drifting && std::fmod(time, 0.5f) < 0.30f;

    if (!rainOn)
        return;
    DrawCube(
        Vector3{0.0f, 0.33f, -2.26f},
        0.24f, 0.24f, 0.05f, Fade(Color{255, 70, 70, 255}, 0.35f));
}

void CarBodyDraw::drawHeadlightGlow(const CarVisual &vis)
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

void CarBodyDraw::drawNitroFlames(const Car &car, float time)
{
    const float phase = car.position().x * 3.7f + car.position().z * 2.9f;

    CarBodyDraw::drawExhaustFlame(-kExhaustX, time, phase);
    CarBodyDraw::drawExhaustFlame(kExhaustX, time, phase + 2.1f);
}

void CarBodyDraw::drawExhaustFlame(float x, float time, float phase)
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

void CarBodyDraw::drawWindshield()
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

} // namespace racer

