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
#include "Vehicle/CarTelemetry.hpp"
#include "World/Aurelia/AureliaBounds.hpp"

namespace racer::world {

namespace {

float normalizeAngleLocal(float angle)
{
    while (angle > PI)
        angle -= 2.0f * PI;
    while (angle < -PI)
        angle += 2.0f * PI;
    return angle;
}

// Lateral g-force is not tracked as physics state anywhere -- the car only
// stores speed and heading. It is derived here exactly as the task spec
// describes: the rate of change of the velocity heading (how fast the
// direction of travel is rotating) times speed gives lateral acceleration
// (a = v * omega for motion along a curved path), divided by g to express
// it in g-force the way telemetry tooling conventionally reports it.
float estimateLateralG(float prevVelocityHeading, float velocityHeading,
    float speed, float dt)
{
    if (dt <= 0.0f)
        return 0.0f;

    constexpr float kGravity = 9.81f;
    float omega = normalizeAngleLocal(velocityHeading - prevVelocityHeading)
        / dt;
    return (omega * speed) / kGravity;
}

} // namespace

void PlayerDriveSystem::reset(Car &car, float worldX, float worldZ,
    float heading)
{
    car.position() = Vector3{worldX, 0.06f, worldZ};
    car.heading() = heading;
    car.velocityHeading() = heading;
    car.speed() = 0.0f;
    // Re-sync the smoothed heading-grip state (see Car::updateVelocityHeading)
    // to the car's current tuning/surface so a respawn/teleport doesn't
    // carry over a stale blended grip value from wherever the car was
    // before.
    car.headingGrip() = car.tuning().gripNormal * car.surfaceGrip();
}

// Surface grip/drag target values sampled from the terrain under the car.
// These used to be written straight into car.surfaceGrip()/surfaceDrag()
// every frame, i.e. an instant step the moment the sampled surface changed
// (e.g. asphalt 1.0 -> grass 0.72 grip, 1.0 -> 1.8 drag, in a single 1/60s
// frame). A telemetry_session run (src/Tools/TelemetrySession.cpp) crossing
// on/off road repeatedly showed exactly that: surface_grip and
// surface_drag flipping between two values from one 0.1s sample to the
// next with no ramp. Blending toward the target over ~125ms
// (kSurfaceBlendRate) instead smooths engine-power and drag response at
// transitions (tires don't lose/gain purchase instantaneously) while
// converging to the same steady-state grip/drag once fully on the new
// surface, so it doesn't change the destination values in CarTuning-driven
// tests.
void PlayerDriveSystem::applySurface(Car &car, SurfaceKind surface, float dt)
{
    constexpr float kSurfaceBlendRate = 8.0f;
    float targetGrip = 1.0f;
    float targetDrag = 1.0f;

    switch (surface) {
    case SurfaceKind::ASPHALT:
        targetGrip = 1.0f;
        targetDrag = 1.0f;
        break;
    case SurfaceKind::GRASS:
        targetGrip = 0.72f;
        targetDrag = 1.8f;
        break;
    case SurfaceKind::SAND:
        targetGrip = 0.65f;
        targetDrag = 2.0f;
        break;
    case SurfaceKind::ROCK:
        targetGrip = 0.85f;
        targetDrag = 1.4f;
        break;
    case SurfaceKind::GRAVEL:
        targetGrip = 0.78f;
        targetDrag = 1.6f;
        break;
    }

    float blend = std::min(1.0f, kSurfaceBlendRate * dt);
    car.surfaceGrip() += (targetGrip - car.surfaceGrip()) * blend;
    car.surfaceDrag() += (targetDrag - car.surfaceDrag()) * blend;
}

void PlayerDriveSystem::update(Car &car, const CarInput &input,
    float steerSmoothed, float dt, ChunkStreamer &streamer,
    float &wheelSpinOut)
{
    float wx = car.position().x;
    float wz = car.position().z;
    SurfaceKind surface = streamer.sampleSurface(wx, wz);
    float prevVelocityHeading = car.velocityHeading();

    applySurface(car, surface, dt);
    car.update(input, dt);

    wx = car.position().x;
    wz = car.position().z;
    float groundY = streamer.sampleHeight(wx, wz);
    car.position().y = groundY + 0.06f;

    softClampCar(car, dt);

    wheelSpinOut += car.speed() * dt / kWheelRadius;
    (void)steerSmoothed;

    if (CarTelemetry::isEnabled()) {
        float lateralG = estimateLateralG(
            prevVelocityHeading, car.velocityHeading(), car.speed(), dt);
        CarTelemetry::record(dt, car.speed(), lateralG, input.throttle,
            input.steer, input.handbrake, car.surfaceGrip(),
            car.surfaceDrag(), car.isDrifting(), car.airborne());
    }
}

} // namespace racer::world
