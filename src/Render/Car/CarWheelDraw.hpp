/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Car wheel and shadow drawing
*/

#ifndef CAR_WHEEL_DRAW_HPP_
#define CAR_WHEEL_DRAW_HPP_

#include "raylib.h"

#include "Render/CarVisual.hpp"
#include "Vehicle/Car.hpp"

namespace racer {

inline constexpr float kWheelRadius = 0.35f;

struct CarWheelDraw {
    static float wrapAngle(float angle);
    static void drawWheelSpokes();
    static void drawWheel(Vector3 center, float steerDeg, float spinDeg);
    static void drawShadowDiscs();
    static void drawShadow(const Car &car, bool drifting);
    static void drawWheels(const CarVisual &vis);
    static void drawHeadlightGroundGlow();
    static void drawNitroGroundGlow();
    static void drawGroundEffects(const CarVisual &vis);
};

} // namespace racer

#endif /* !CAR_WHEEL_DRAW_HPP_ */
