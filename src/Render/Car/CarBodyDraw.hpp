/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Car body and lighting draw pass
*/

#ifndef CAR_BODY_DRAW_HPP_
#define CAR_BODY_DRAW_HPP_

#include "raylib.h"

#include "Render/CarVisual.hpp"
#include "Vehicle/Car.hpp"

namespace racer {

struct CarBodyPalette {
    Color body;
    Color stripe;
    Color carbon;
    Color carbonLight;
};

struct CarBodyDraw {
    static CarBodyPalette makeBodyPalette(Color bodyColor);
    static Color shade(Color color, float factor);
    static void pushChassisPose(const Car &car, const CarVisual &vis);
    static void popChassisPose();
    static void drawBodyShell(const CarBodyPalette &palette);
    static void drawWheelArches(const CarBodyPalette &palette);
    static void drawAeroTrim(const CarBodyPalette &palette);
    static void drawLivery(const CarBodyPalette &palette);
    static void drawRacePlates();
    static void drawFrontDetails(const CarBodyPalette &palette);
    static void drawSideIntakes();
    static void drawMirrors(const CarBodyPalette &palette);
    static void drawAntenna();
    static void drawRollCage(const CarBodyPalette &palette);
    static void drawDriver();
    static void drawRearWing(const CarBodyPalette &palette);
    static void drawExhaustPipes();
    static void drawChassisAssembly(const CarBodyPalette &palette);
    static void drawHeadlightFixtures(const CarVisual &vis);
    static void drawBrakeFixtures(const CarVisual &vis);
    static void drawOverlayEffects(
        const Car &car, const CarVisual &vis, float time);
    static void drawRainLight(const CarVisual &vis, float time);
    static void drawBrakeGlow(const CarVisual &vis);
    static void drawRainGlow(const CarVisual &vis, float time);
    static void drawHeadlightGlow(const CarVisual &vis);
    static void drawNitroFlames(const Car &car, float time);
    static void drawExhaustFlame(float x, float time, float phase);
    static void drawWindshield();
};

} // namespace racer

#endif /* !CAR_BODY_DRAW_HPP_ */
