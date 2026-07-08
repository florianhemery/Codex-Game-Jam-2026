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

AIDriver::AIDriver(float skill, unsigned int seed, AiDifficulty difficulty)
    : skill_(skill)
    , difficulty_(difficulty)
{
    if (seed == 0) {
        seed = static_cast<unsigned int>(skill * kDefaultSeedMultiplier)
            + kDefaultSeedOffset;
    }
    unsigned int hash = hashU32(seed);
    laneOffset_ = (static_cast<float>(hash % kLaneHashModulus)
        / kLaneHashNormalizer * 2.0f - 1.0f) * kLaneOffsetRange;
    float weakness = std::max(0.0f, 1.0f - skill_);
    nitroReserve_ = kNitroReserveBase + weakness * kNitroReserveWeaknessScale;

    switch (difficulty_) {
    case AiDifficulty::Easy:
        speedMul_ = kEasySpeedMul;
        gripMul_ = kEasyGripMul;
        lookaheadMul_ = kEasyLookaheadMul;
        steerMul_ = kEasySteerMul;
        nitroMul_ = kEasyNitroMul;
        break;
    case AiDifficulty::Hard:
        speedMul_ = kHardSpeedMul;
        gripMul_ = kHardGripMul;
        lookaheadMul_ = kHardLookaheadMul;
        steerMul_ = kHardSteerMul;
        nitroMul_ = kHardNitroMul;
        break;
    case AiDifficulty::Normal:
    default:
        speedMul_ = 1.0f;
        gripMul_ = 1.0f;
        lookaheadMul_ = 1.0f;
        steerMul_ = 1.0f;
        nitroMul_ = 1.0f;
        break;
    }
}

AIDriver::CornerLimits AIDriver::anticipateCorners(
    const Car &car,
    const Track &track,
    float currentDist,
    float speedAbs,
    float rubberBand) const
{
    CornerLimits limits;
    limits.vLimit = kUnlimitedSpeed;
    float chord = kChordBase + speedAbs * kChordSpeedScale;
    scanCornerSamples(limits, car, track, currentDist, chord, rubberBand);
    return limits;
}

void AIDriver::scanCornerSamples(
    CornerLimits &limits,
    const Car &car,
    const Track &track,
    float currentDist,
    float chord,
    float rubberBand) const
{
    Vector2 prev = track.pointAtDistance(currentDist);
    Vector2 cur = track.pointAtDistance(currentDist + chord);
    for (int sampleIndex = 1; sampleIndex <= kCornerSampleCount; ++sampleIndex) {
        Vector2 next = track.pointAtDistance(
            currentDist + chord * static_cast<float>(sampleIndex + 1));
        float prevHeading = std::atan2(cur.x - prev.x, cur.y - prev.y);
        float nextHeading = std::atan2(next.x - cur.x, next.y - cur.y);
        float turnPerUnit = std::fabs(
            normalizeAngle(nextHeading - prevHeading)) / chord;
        accumulateCornerSample(
            limits, car, chord, turnPerUnit, sampleIndex, rubberBand);
        prev = cur;
        cur = next;
    }
}

void AIDriver::accumulateCornerSample(
    CornerLimits &limits,
    const Car &car,
    float chord,
    float turnPerU,
    int sampleIndex,
    float rubberBand) const
{
    limits.maxTurnPerU = std::max(limits.maxTurnPerU, turnPerU);
    float vCorner = std::max(
        kMinCornerSpeed,
        kGripBudget * gripMul_ * rubberBand * skill_ * skill_
            / std::max(turnPerU, kMinTurnPerUnit));
    float distToTurn = chord * static_cast<float>(sampleIndex - 1);
    float brakeAccel = car.tuning().braking * kBrakeFraction;
    float vAllowed = std::sqrt(
        vCorner * vCorner + 2.0f * brakeAccel * distToTurn);
    limits.vLimit = std::min(limits.vLimit, vAllowed);
}

float AIDriver::computeLookahead(float speedAbs) const
{
    return (kLookaheadBase + speedAbs * kLookaheadSpeedScale) * lookaheadMul_;
}

Vector2 AIDriver::offsetTargetFromLane(
    const Track &track,
    float targetDist,
    float maxTurnPerU,
    Vector2 target) const
{
    float offsetScale = std::clamp(
        1.0f - maxTurnPerU / kMaxTurnPerUnit,
        kOffsetScaleMin,
        kOffsetScaleMax);
    Vector2 tangentA = track.pointAtDistance(targetDist - kTangentDelta);
    Vector2 tangentB = track.pointAtDistance(targetDist + kTangentDelta);
    float tangentDx = tangentB.x - tangentA.x;
    float tangentDz = tangentB.y - tangentA.y;
    float tangentLen = std::sqrt(tangentDx * tangentDx + tangentDz * tangentDz);
    if (tangentLen > kMinTangentLength) {
        target.x += (-tangentDz / tangentLen) * laneOffset_ * offsetScale;
        target.y += (tangentDx / tangentLen) * laneOffset_ * offsetScale;
    }
    return target;
}

Vector2 AIDriver::computeTarget(
    const Track &track,
    float currentDist,
    float speedAbs,
    float maxTurnPerU) const
{
    float targetDist = currentDist + computeLookahead(speedAbs);
    Vector2 target = track.pointAtDistance(targetDist);
    return offsetTargetFromLane(track, targetDist, maxTurnPerU, target);
}

float AIDriver::computeHeadingError(const Car &car, const Vector2 &target) const
{
    float dx = target.x - car.position().x;
    float dz = target.y - car.position().z;
    float desiredHeading = std::atan2(dx, dz);
    return normalizeAngle(desiredHeading - car.heading());
}

float AIDriver::computeTargetSpeed(
    const Car &car,
    float turnSeverity,
    float vLimit,
    bool wantNitro,
    float rubberBand) const
{
    float alignFactor = std::clamp(
        1.0f - turnSeverity * kAlignTurnScale,
        kAlignFactorMin,
        kAlignFactorMax);
    float vCruise = car.tuning().maxSpeed * alignFactor * skill_
        * speedMul_ * rubberBand;
    if (wantNitro) {
        vCruise += car.tuning().nitroMaxSpeedBonus;
    }
    return std::min(vCruise, vLimit);
}

bool AIDriver::shouldUseNitro(
    const Car &car,
    float speedAbs,
    float vLimit,
    float turnSeverity) const
{
    float nitroMargin = (kNitroMarginBase + kNitroMarginSkillScale * skill_)
        / nitroMul_;
    return car.nitroRemaining() > nitroReserve_
        && turnSeverity < kNitroTurnThreshold
        && vLimit > speedAbs + nitroMargin;
}

void AIDriver::applyThrottle(
    const Car &car,
    float speedAbs,
    float vLimit,
    float vTarget,
    CarInput &input) const
{
    if (car.speed() > vLimit * kOverspeedRatio) {
        input.throttle = kBrakeThrottle;
    } else if (speedAbs < vTarget) {
        input.throttle = kFullThrottle;
    } else {
        input.throttle = kCoastThrottle;
    }
}

void AIDriver::applyDriveInput(
    const Car &car,
    float speedAbs,
    float vLimit,
    float turnSeverity,
    float rubberBand,
    CarInput &input) const
{
    bool wantNitro = shouldUseNitro(car, speedAbs, vLimit, turnSeverity);
    float vTarget = computeTargetSpeed(
        car, turnSeverity, vLimit, wantNitro, rubberBand);
    applyThrottle(car, speedAbs, vLimit, vTarget, input);
    input.handbrake = turnSeverity > kHandbrakeTurnThreshold
        && speedAbs > kHandbrakeMinSpeed;
    input.nitro = wantNitro && input.throttle >= kFullThrottle;
}

CarInput AIDriver::computeInput(
    const Car &car, const Track &track, float rubberBand) const
{
    float rb = std::clamp(rubberBand, kRubberBandMin, kRubberBandMax);
    Track::Progress trackProgress = track.projectPosition(car.position());
    float currentDist = track.cumulativeDistance(trackProgress);
    float speedAbs = std::fabs(car.speed());
    CornerLimits limits = anticipateCorners(
        car, track, currentDist, speedAbs, rb);
    Vector2 target = computeTarget(
        track, currentDist, speedAbs, limits.maxTurnPerU);
    float headingError = computeHeadingError(car, target);
    float turnSeverity = std::fabs(headingError);
    CarInput input;
    input.steer = std::clamp(
        headingError * kSteerGain * steerMul_, kSteerMin, kSteerMax);
    applyDriveInput(car, speedAbs, limits.vLimit, turnSeverity, rb, input);
    return input;
}

} // namespace racer
