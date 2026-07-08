/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Arcade vehicle physics implementation
*/

#include "Vehicle/Car.hpp"

#include <algorithm>
#include <cmath>

namespace racer {

namespace {

float computeEngineAccel(Car &car, const CarInput &input, bool nitroActive)
{
    float engineAccel = 0.0f;

    if (input.throttle > 0.0f) {
        engineAccel = input.throttle * car.tuning().acceleration;
        if (nitroActive)
            engineAccel += car.tuning().nitroBoost;
        engineAccel *= 0.55f + 0.45f * car.surfaceGrip();
    } else if (input.throttle < 0.0f) {
        if (car.speed() > 0.5f) {
            engineAccel = input.throttle * car.tuning().braking;
        } else {
            engineAccel = input.throttle * car.tuning().acceleration * 0.6f;
        }
    }
    return engineAccel;
}

void applyDragAndClamp(Car &car, float currentMaxSpeed, float dt)
{
    float maxReverseSpeed = car.tuning().maxSpeed * 0.4f;

    car.speed() -= car.tuning().dragCoeff * car.surfaceDrag() * car.speed()
        * dt;
    car.speed() = std::clamp(
        car.speed(), -maxReverseSpeed, currentMaxSpeed);
}

} // namespace

float Car::normalizeAngle(float angle)
{
    while (angle > PI)
        angle -= 2.0f * PI;
    while (angle < -PI)
        angle += 2.0f * PI;
    return angle;
}

float Car::sign(float value)
{
    return value > 0.0f ? 1.0f : (value < 0.0f ? -1.0f : 0.0f);
}

bool Car::updateNitro(Car &car, const CarInput &input, float dt)
{
    bool nitroActive = input.nitro && car.nitroRemaining() > 0.0f;

    if (nitroActive) {
        car.nitroRemaining() = std::max(0.0f, car.nitroRemaining() - dt);
    } else {
        car.nitroRemaining() = std::min(
            car.tuning().nitroCapacity,
            car.nitroRemaining() + car.tuning().nitroRegenPerSecond * dt);
    }
    return nitroActive;
}

void Car::applyEngineAndDrag(
    Car &car, const CarInput &input, float dt, bool nitroActive)
{
    float currentMaxSpeed = car.tuning().maxSpeed;
    float engineAccel = computeEngineAccel(car, input, nitroActive);

    if (car.startBoostTimer() > 0.0f) {
        engineAccel *= car.startBoostAccelMul();
        currentMaxSpeed += car.startBoostSpeedBonus();
        car.startBoostTimer() = std::max(0.0f, car.startBoostTimer() - dt);
        if (car.startBoostTimer() <= 0.0f) {
            car.startBoostAccelMul() = 1.0f;
            car.startBoostSpeedBonus() = 0.0f;
        }
    }
    if (nitroActive)
        currentMaxSpeed += car.tuning().nitroMaxSpeedBonus;
    car.speed() += engineAccel * dt;
    applyDragAndClamp(car, currentMaxSpeed, dt);
}

void Car::updateHeading(Car &car, const CarInput &input, float dt)
{
    float driftTurnBoost = 1.0f;
    float speedFactor = std::min(1.0f, std::fabs(car.speed()) / 3.0f);
    float speedSign = 1.0f;

    car.isDrifting() = input.handbrake && std::fabs(car.speed()) > 4.0f;
    if (car.isDrifting())
        driftTurnBoost = 1.5f;
    if (car.speed() != 0.0f)
        speedSign = sign(car.speed());
    car.heading() += input.steer * car.tuning().turnRate * driftTurnBoost
        * speedFactor * speedSign * dt;
    car.heading() = normalizeAngle(car.heading());
}

void Car::updateVelocityHeading(Car &car, float dt)
{
    float grip = car.tuning().gripNormal * car.surfaceGrip();
    float headingDiff = 0.0f;

    if (car.isDrifting())
        grip = car.tuning().gripDrift * car.surfaceGrip();
    headingDiff = normalizeAngle(car.heading() - car.velocityHeading());
    car.velocityHeading() += headingDiff * std::min(1.0f, grip * dt);
    car.velocityHeading() = normalizeAngle(car.velocityHeading());
}

void Car::integratePosition(Car &car, float dt)
{
    Vector3 vel = car.velocity();

    car.position().x += vel.x * dt;
    car.position().z += vel.z * dt;
}

Vector3 Car::forward() const
{
    return Vector3{
        std::sin(heading()), 0.0f, std::cos(heading())};
}

Vector3 Car::velocity() const
{
    return Vector3{
        std::sin(velocityHeading()) * speed(),
        0.0f,
        std::cos(velocityHeading()) * speed()};
}

void Car::update(const CarInput &input, float dt)
{
    bool nitroActive = updateNitro(*this, input, dt);

    applyEngineAndDrag(*this, input, dt, nitroActive);
    updateHeading(*this, input, dt);
    updateVelocityHeading(*this, dt);
    integratePosition(*this, dt);
}

} // namespace racer
