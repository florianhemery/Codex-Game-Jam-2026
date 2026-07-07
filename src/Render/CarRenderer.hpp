/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Car visual rendering and light anchor points
*/

#ifndef CAR_RENDERER_HPP_
#define CAR_RENDERER_HPP_

#include "raylib.h"
#include "Vehicle/Car.hpp"

namespace racer {

inline constexpr float kWheelRadius = 0.35f;

struct CarVisual {
    float steer = 0.0f;
    float wheelSpin = 0.0f;
    bool braking = false;
    bool nitro = false;
    bool headlights = false;
    bool drifting = false;
};

void DrawCar(const Car &car, Color bodyColor);
void DrawCarEx(const Car &car, const CarVisual &vis, Color bodyColor);

struct CarLightPoints {
    Vector3 headL;
    Vector3 headR;
    Vector3 brakeL;
    Vector3 brakeR;
    Vector3 exhaust;
};

CarLightPoints GetCarLightPoints(const Car &car);

} // namespace racer

#endif /* !CAR_RENDERER_HPP_ */
