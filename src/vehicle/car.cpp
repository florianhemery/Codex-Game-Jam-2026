#include "vehicle/car.h"

#include <algorithm>
#include <cmath>

namespace racer {

namespace {
float NormalizeAngle(float a) {
    while (a > PI) a -= 2.0f * PI;
    while (a < -PI) a += 2.0f * PI;
    return a;
}
float Sign(float v) { return v > 0.0f ? 1.0f : (v < 0.0f ? -1.0f : 0.0f); }
} // namespace

Vector3 Car::Forward() const {
    return Vector3{std::sin(heading), 0.0f, std::cos(heading)};
}

Vector3 Car::Velocity() const {
    return Vector3{std::sin(velocityHeading) * speed, 0.0f, std::cos(velocityHeading) * speed};
}

void Car::Update(const CarInput& input, float dt) {
    // Nitro : consomme si actif et disponible, sinon regenere lentement.
    bool nitroActive = input.nitro && nitroRemaining > 0.0f;
    if (nitroActive) {
        nitroRemaining = std::max(0.0f, nitroRemaining - dt);
    } else {
        nitroRemaining = std::min(tuning.nitroCapacity, nitroRemaining + tuning.nitroRegenPerSecond * dt);
    }

    float currentMaxSpeed = tuning.maxSpeed + (nitroActive ? tuning.nitroMaxSpeedBonus : 0.0f);
    float maxReverseSpeed = tuning.maxSpeed * 0.4f;

    // Acceleration moteur / freinage / marche arriere.
    float engineAccel = 0.0f;
    if (input.throttle > 0.0f) {
        engineAccel = input.throttle * tuning.acceleration;
        if (nitroActive) engineAccel += tuning.nitroBoost;
    } else if (input.throttle < 0.0f) {
        if (speed > 0.5f) {
            engineAccel = input.throttle * tuning.braking; // freine tant qu'on avance encore
        } else {
            engineAccel = input.throttle * tuning.acceleration * 0.6f; // marche arriere, plus lente
        }
    }

    speed += engineAccel * dt;
    speed -= tuning.dragCoeff * speed * dt; // trainee proportionnelle a la vitesse
    speed = std::clamp(speed, -maxReverseSpeed, currentMaxSpeed);

    // Direction du chassis (heading) : ne tourne efficacement qu'au-dela d'une vitesse minimale.
    isDrifting = input.handbrake && std::fabs(speed) > 4.0f;
    float driftTurnBoost = isDrifting ? 1.5f : 1.0f;
    float speedFactor = std::min(1.0f, std::fabs(speed) / 3.0f);
    heading += input.steer * tuning.turnRate * driftTurnBoost * speedFactor * Sign(speed == 0.0f ? 1.0f : speed) * dt;
    heading = NormalizeAngle(heading);

    // Direction reelle de deplacement : rattrape le heading a une vitesse de
    // "grip" donnee. Grip bas (handbrake) = le chassis tourne plus vite que
    // la trajectoire ne suit -> glissade visible, c'est le drift.
    float grip = isDrifting ? tuning.gripDrift : tuning.gripNormal;
    float headingDiff = NormalizeAngle(heading - velocityHeading);
    velocityHeading += headingDiff * std::min(1.0f, grip * dt);
    velocityHeading = NormalizeAngle(velocityHeading);

    Vector3 vel = Velocity();
    position.x += vel.x * dt;
    position.z += vel.z * dt;
}

} // namespace racer
