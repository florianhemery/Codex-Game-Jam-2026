/*
** EPITECH PROJECT, 2026
** racer
** File description:
** PlayerDriveSystem implementation
*/

#include "World/Sim/PlayerDriveSystem.hpp"

#include <algorithm>
#include <cmath>

#include "Vehicle/Car.hpp"

namespace racer::world {

void PlayerDriveSystem::reset(Car &car, float worldX, float worldZ,
    float heading)
{
    car.position() = Vector3{worldX, 0.06f, worldZ};
    car.heading() = heading;
    car.velocityHeading() = heading;
    car.speed() = 0.0f;
}

void PlayerDriveSystem::applySurface(Car &car, SurfaceKind surface)
{
    switch (surface) {
    case SurfaceKind::ASPHALT:
        car.surfaceGrip() = 1.0f;
        car.surfaceDrag() = 1.0f;
        break;
    case SurfaceKind::GRASS:
        car.surfaceGrip() = 0.72f;
        car.surfaceDrag() = 1.8f;
        break;
    case SurfaceKind::SAND:
        car.surfaceGrip() = 0.65f;
        car.surfaceDrag() = 2.0f;
        break;
    case SurfaceKind::ROCK:
        car.surfaceGrip() = 0.85f;
        car.surfaceDrag() = 1.4f;
        break;
    case SurfaceKind::GRAVEL:
        car.surfaceGrip() = 0.78f;
        car.surfaceDrag() = 1.6f;
        break;
    }
}

void PlayerDriveSystem::update(Car &car, const CarInput &input,
    float steerSmoothed, float dt, ChunkStreamer &streamer,
    float &wheelSpinOut)
{
    float wx = car.position().x;
    float wz = car.position().z;
    SurfaceKind surface = streamer.sampleSurface(wx, wz);
    applySurface(car, surface);
    car.update(input, dt);

    wx = car.position().x;
    wz = car.position().z;
    float groundY = streamer.sampleHeight(wx, wz);
    car.position().y = groundY + 0.06f;

    wheelSpinOut += car.speed() * dt / kWheelRadius;
    (void)steerSmoothed;
}

} // namespace racer::world
