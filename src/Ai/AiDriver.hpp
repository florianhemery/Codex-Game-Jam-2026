/*
** EPITECH PROJECT, 2026
** racer
** File description:
** ai_driver
*/

#ifndef AI_DRIVER_HPP_
#define AI_DRIVER_HPP_

#include "Track/Track.hpp"
#include "Vehicle/Car.hpp"

namespace racer {

// IA par poursuite de point-cible : projette la voiture sur la piste, vise un
// point plus loin sur la ligne centrale (decale par la personnalite du
// pilote), et pilote vers lui. S'y ajoutent une anticipation de freinage par
// echantillonnage de la courbure a venir et une gestion du nitro dependante
// du skill. Pas de A*, pas d'etat mutable - tout est const apres construction.
class AIDriver {
public:
    // seed = 0 : graine derivee du skill (deterministe). Passer une graine
    // explicite donne des personnalites distinctes meme a skill egal.
    explicit AIDriver(float skill = 1.0f, unsigned int seed = 0);

    CarInput computeInput(const Car &car, const Track &track) const;

private:
    struct CornerLimits {
        float maxTurnPerU = 0.0f;
        float vLimit = 0.0f;
    };

    static constexpr float kGripBudget = 1.4f;
    static constexpr float kMinCornerSpeed = 7.0f;
    static constexpr float kUnlimitedSpeed = 1e9f;
    static constexpr float kChordBase = 4.0f;
    static constexpr float kChordSpeedScale = 0.12f;
    static constexpr int kCornerSampleCount = 5;
    static constexpr float kMinTurnPerUnit = 1e-3f;
    static constexpr float kBrakeFraction = 0.7f;
    static constexpr float kLookaheadBase = 10.0f;
    static constexpr float kLookaheadSpeedScale = 0.6f;
    static constexpr float kTangentDelta = 2.0f;
    static constexpr float kMaxTurnPerUnit = 0.06f;
    static constexpr float kOffsetScaleMin = 0.4f;
    static constexpr float kOffsetScaleMax = 1.0f;
    static constexpr float kMinTangentLength = 1e-4f;
    static constexpr float kAlignTurnScale = 0.8f;
    static constexpr float kAlignFactorMin = 0.35f;
    static constexpr float kAlignFactorMax = 1.0f;
    static constexpr float kOverspeedRatio = 1.08f;
    static constexpr float kBrakeThrottle = -0.75f;
    static constexpr float kFullThrottle = 1.0f;
    static constexpr float kCoastThrottle = 0.15f;
    static constexpr float kNitroMarginBase = 6.0f;
    static constexpr float kNitroMarginSkillScale = 8.0f;
    static constexpr float kNitroTurnThreshold = 0.15f;
    static constexpr float kHandbrakeTurnThreshold = 1.2f;
    static constexpr float kHandbrakeMinSpeed = 6.0f;
    static constexpr float kSteerGain = 2.0f;
    static constexpr float kSteerMin = -1.0f;
    static constexpr float kSteerMax = 1.0f;
    static constexpr float kDefaultSeedMultiplier = 997.0f;
    static constexpr unsigned int kDefaultSeedOffset = 17u;
    static constexpr unsigned int kLaneHashModulus = 1000u;
    static constexpr float kLaneHashNormalizer = 999.0f;
    static constexpr float kLaneOffsetRange = 1.8f;
    static constexpr float kNitroReserveBase = 1.2f;
    static constexpr float kNitroReserveWeaknessScale = 4.0f;

    static float normalizeAngle(float angle);
    static unsigned int hashU32(unsigned int value);

    CornerLimits anticipateCorners(
        const Car &car,
        const Track &track,
        float currentDist,
        float speedAbs) const;
    void scanCornerSamples(
        CornerLimits &limits,
        const Car &car,
        const Track &track,
        float currentDist,
        float chord) const;
    void accumulateCornerSample(
        CornerLimits &limits,
        const Car &car,
        float chord,
        float turnPerU,
        int sampleIndex) const;
    float computeLookahead(float speedAbs) const;
    Vector2 offsetTargetFromLane(
        const Track &track,
        float targetDist,
        float maxTurnPerU,
        Vector2 target) const;
    Vector2 computeTarget(
        const Track &track,
        float currentDist,
        float speedAbs,
        float maxTurnPerU) const;
    float computeHeadingError(const Car &car, const Vector2 &target) const;
    float computeTargetSpeed(
        const Car &car,
        float turnSeverity,
        float vLimit,
        bool wantNitro) const;
    bool shouldUseNitro(
        const Car &car,
        float speedAbs,
        float vLimit,
        float turnSeverity) const;
    void applyThrottle(
        const Car &car,
        float speedAbs,
        float vLimit,
        float vTarget,
        CarInput &input) const;
    void applyDriveInput(
        const Car &car,
        float speedAbs,
        float vLimit,
        float turnSeverity,
        CarInput &input) const;

    float skill_ = 1.0f;
    float laneOffset_ = 0.0f;
    float nitroReserve_ = 1.2f;
};

} // namespace racer

#endif /* !AI_DRIVER_HPP_ */
