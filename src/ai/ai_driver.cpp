#include "ai/ai_driver.h"

#include <algorithm>
#include <cmath>

namespace racer {

namespace {
float NormalizeAngle(float a) {
    while (a > PI) a -= 2.0f * PI;
    while (a < -PI) a += 2.0f * PI;
    return a;
}
} // namespace

CarInput AIDriver::ComputeInput(const Car& car, const Track& track) const {
    Track::Progress prog = track.ProjectPosition(car.position);
    float currentDist = track.CumulativeDistance(prog);

    float lookahead = 10.0f + std::fabs(car.speed) * 0.6f;
    Vector2 target = track.PointAtDistance(currentDist + lookahead);

    float dx = target.x - car.position.x;
    float dz = target.y - car.position.z; // Vector2::y stocke la coordonnee monde Z
    float desiredHeading = std::atan2(dx, dz);
    float headingError = NormalizeAngle(desiredHeading - car.heading);

    CarInput input;
    input.steer = std::clamp(headingError * 2.0f, -1.0f, 1.0f);

    float turnSeverity = std::fabs(headingError);
    float targetSpeedFactor = std::clamp(1.0f - turnSeverity * 0.8f, 0.35f, 1.0f);
    float currentSpeedFactor = car.tuning.maxSpeed > 0.0f ? car.speed / car.tuning.maxSpeed : 0.0f;

    input.throttle = (currentSpeedFactor < targetSpeedFactor * skill_) ? 1.0f : 0.2f;
    input.handbrake = turnSeverity > 0.9f && std::fabs(car.speed) > 6.0f;
    input.nitro = turnSeverity < 0.15f;

    return input;
}

} // namespace racer
