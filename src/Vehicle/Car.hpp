/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Arcade vehicle model, inputs and tuning
*/

#ifndef CAR_HPP_
#define CAR_HPP_

#include "raylib.h"

namespace racer {

struct CarInput {
    float throttle = 0.0f;
    float steer = 0.0f;
    bool handbrake = false;
    bool nitro = false;
};

struct CarTuning {
    float maxSpeed = 28.0f;
    float acceleration = 14.0f;
    float braking = 24.0f;
    float turnRate = 2.6f;
    float gripNormal = 6.0f;
    float gripDrift = 0.8f;
    float dragCoeff = 0.35f;
    float nitroBoost = 12.0f;
    float nitroMaxSpeedBonus = 10.0f;
    float nitroCapacity = 3.0f;
    float nitroRegenPerSecond = 0.4f;
};

class Car {
public:
    Vector3 position{0.0f, 0.0f, 0.0f};
    float heading = 0.0f;
    float speed = 0.0f;
    float velocityHeading = 0.0f;
    float nitroRemaining = 3.0f;
    bool isDrifting = false;
    float surfaceGrip = 1.0f;
    float surfaceDrag = 1.0f;
    CarTuning tuning{};

    void update(const CarInput &input, float dt);
    Vector3 forward() const;
    Vector3 velocity() const;
};

} // namespace racer

#endif /* !CAR_HPP_ */
