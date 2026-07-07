/*
** EPITECH PROJECT, 2026
** racer
** File description:
** ai_driver
*/

#include "Ai/AiDriver.hpp"

#include <algorithm>
#include <cmath>

namespace racer {

float AIDriver::normalizeAngle(float angle)
{
    while (angle > PI) {
        angle -= 2.0f * PI;
    }
    while (angle < -PI) {
        angle += 2.0f * PI;
    }
    return angle;
}

unsigned int AIDriver::hashU32(unsigned int value)
{
    value ^= value >> 16;
    value *= 0x7feb352dU;
    value ^= value >> 15;
    value *= 0x846ca68bU;
    value ^= value >> 16;
    return value;
}

AIDriver::AIDriver(float skill, unsigned int seed)
    : skill_(skill)
{
    if (seed == 0) {
        seed = static_cast<unsigned int>(skill * 997.0f) + 17u;
    }
    unsigned int h = hashU32(seed);
    laneOffset_ = (static_cast<float>(h % 1000u) / 999.0f * 2.0f - 1.0f) * 1.8f;
    float weakness = std::max(0.0f, 1.0f - skill_);
    nitroReserve_ = 1.2f + weakness * 4.0f;
}

AIDriver::CornerLimits AIDriver::anticipateCorners(
    const Car &car,
    const Track &track,
    float currentDist,
    float speedAbs) const
{
    CornerLimits limits;
    limits.vLimit = 1e9f;
    float chord = 4.0f + speedAbs * 0.12f;
    Vector2 prev = track.pointAtDistance(currentDist);
    Vector2 cur = track.pointAtDistance(currentDist + chord);
    for (int s = 1; s <= 5; ++s) {
        Vector2 next = track.pointAtDistance(
            currentDist + chord * static_cast<float>(s + 1));
        float h1 = std::atan2(cur.x - prev.x, cur.y - prev.y);
        float h2 = std::atan2(next.x - cur.x, next.y - cur.y);
        float turnPerU = std::fabs(normalizeAngle(h2 - h1)) / chord;
        accumulateCornerSample(limits, car, chord, turnPerU, s);
        prev = cur;
        cur = next;
    }
    return limits;
}

void AIDriver::accumulateCornerSample(
    CornerLimits &limits,
    const Car &car,
    float chord,
    float turnPerU,
    int sampleIndex) const
{
    limits.maxTurnPerU = std::max(limits.maxTurnPerU, turnPerU);
    float vCorner = std::max(
        kMinCornerSpeed,
        kGripBudget * skill_ * skill_ / std::max(turnPerU, 1e-3f));
    float distToTurn = chord * static_cast<float>(sampleIndex - 1);
    float aBrake = car.tuning.braking * 0.7f;
    float vAllowed = std::sqrt(
        vCorner * vCorner + 2.0f * aBrake * distToTurn);
    limits.vLimit = std::min(limits.vLimit, vAllowed);
}

Vector2 AIDriver::computeTarget(
    const Track &track,
    float currentDist,
    float speedAbs,
    float maxTurnPerU) const
{
    float lookahead = 10.0f + speedAbs * 0.6f;
    float targetDist = currentDist + lookahead;
    Vector2 target = track.pointAtDistance(targetDist);
    float offsetScale = std::clamp(1.0f - maxTurnPerU / 0.06f, 0.4f, 1.0f);
    Vector2 ta = track.pointAtDistance(targetDist - 2.0f);
    Vector2 tb = track.pointAtDistance(targetDist + 2.0f);
    float dxT = tb.x - ta.x;
    float dzT = tb.y - ta.y;
    float len = std::sqrt(dxT * dxT + dzT * dzT);
    if (len > 1e-4f) {
        target.x += (-dzT / len) * laneOffset_ * offsetScale;
        target.y += (dxT / len) * laneOffset_ * offsetScale;
    }
    return target;
}

float AIDriver::computeHeadingError(const Car &car, const Vector2 &target) const
{
    float dx = target.x - car.position.x;
    float dz = target.y - car.position.z;
    float desiredHeading = std::atan2(dx, dz);
    return normalizeAngle(desiredHeading - car.heading);
}

float AIDriver::computeTargetSpeed(
    const Car &car,
    float turnSeverity,
    float vLimit,
    bool wantNitro) const
{
    float alignFactor = std::clamp(1.0f - turnSeverity * 0.8f, 0.35f, 1.0f);
    float vCruise = car.tuning.maxSpeed * alignFactor * skill_;
    if (wantNitro) {
        vCruise += car.tuning.nitroMaxSpeedBonus;
    }
    return std::min(vCruise, vLimit);
}

void AIDriver::applyThrottle(
    const Car &car,
    float speedAbs,
    float vLimit,
    float vTarget,
    CarInput &input) const
{
    if (car.speed > vLimit * 1.08f) {
        input.throttle = -0.75f;
    } else if (speedAbs < vTarget) {
        input.throttle = 1.0f;
    } else {
        input.throttle = 0.15f;
    }
}

void AIDriver::applyDriveInput(
    const Car &car,
    float speedAbs,
    float vLimit,
    float turnSeverity,
    CarInput &input) const
{
    float nitroMargin = 6.0f + 8.0f * skill_;
    bool wantNitro = car.nitroRemaining > nitroReserve_
        && turnSeverity < 0.15f
        && vLimit > speedAbs + nitroMargin;
    float vTarget = computeTargetSpeed(car, turnSeverity, vLimit, wantNitro);
    applyThrottle(car, speedAbs, vLimit, vTarget, input);
    input.handbrake = turnSeverity > 1.2f && speedAbs > 6.0f;
    input.nitro = wantNitro && input.throttle >= 1.0f;
}

CarInput AIDriver::computeInput(const Car &car, const Track &track) const
{
    Track::Progress prog = track.projectPosition(car.position);
    float currentDist = track.cumulativeDistance(prog);
    float speedAbs = std::fabs(car.speed);
    CornerLimits limits = anticipateCorners(car, track, currentDist, speedAbs);
    Vector2 target = computeTarget(
        track, currentDist, speedAbs, limits.maxTurnPerU);
    float headingError = computeHeadingError(car, target);
    float turnSeverity = std::fabs(headingError);
    CarInput input;
    input.steer = std::clamp(headingError * 2.0f, -1.0f, 1.0f);
    applyDriveInput(car, speedAbs, limits.vLimit, turnSeverity, input);
    return input;
}

} // namespace racer
