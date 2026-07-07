/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Car body panel and fixture drawing
*/

#include "Render/Car/CarBodyDraw.hpp"
#include "Render/Car/CarVisual.hpp"
#include "rlgl.h"

namespace racer {

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
    if (vis.headlights)
        drawHeadlightGlow(vis);
    if (vis.nitro)
        drawNitroFlames(car, time);
    drawWindshield();
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

    drawExhaustFlame(-kExhaustX, time, phase);
    drawExhaustFlame(kExhaustX, time, phase + 2.1f);
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

} // namespace racer
