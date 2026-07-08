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

// Palier de difficulte global applique par-dessus le skill individuel de
// chaque pilote IA. Normal == comportement historique (multiplicateurs
// neutres) afin de ne pas casser les courses/tests existants qui ne
// precisent pas de difficulte.
enum class AiDifficulty {
    Easy,
    Normal,
    Hard,
};

// IA par poursuite de point-cible : projette la voiture sur la piste, vise un
// point plus loin sur la ligne centrale (decale par la personnalite du
// pilote), et pilote vers lui. S'y ajoutent une anticipation de freinage par
// echantillonnage de la courbure a venir et une gestion du nitro dependante
// du skill. Pas de A*, pas d'etat mutable - tout est const apres construction.
class AIDriver {
public:
    // seed = 0 : graine derivee du skill (deterministe). Passer une graine
    // explicite donne des personnalites distinctes meme a skill egal.
    explicit AIDriver(
        float skill = 1.0f,
        unsigned int seed = 0,
        AiDifficulty difficulty = AiDifficulty::Normal);

    // rubberBand : multiplicateur borne (cf. kRubberBandMin/Max) applique a
    // la vitesse de croisiere et a la marge d'adherence en virage. >1 aide un
    // pilote en retard sur le joueur, <1 freine un pilote trop en avance.
    // 1.0 (defaut) => comportement identique a avant l'ajout du systeme.
    CarInput computeInput(
        const Car &car, const Track &track, float rubberBand = 1.0f) const;

    AiDifficulty difficulty() const { return difficulty_; }

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
    static constexpr float kLookaheadBase = 15.0f;
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
    static constexpr float kNitroMarginBase = 9.0f;
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

    // Multiplicateur de rubber-banding : borne dur, jamais plus de +/-15%
    // (cf. tache : "never more than ±15-20%").
    static constexpr float kRubberBandMin = 0.85f;
    static constexpr float kRubberBandMax = 1.15f;

    // Paliers de difficulte : chaque facteur est neutre (1.0) en Normal pour
    // garder le comportement historique par defaut.
    // speedMul     : echelle la vitesse de croisiere vise (vCruise).
    // gripMul      : echelle le budget d'adherence en virage (agressivite).
    // lookaheadMul : echelle la distance de visee (reaction/anticipation).
    // steerMul     : echelle le gain de braquage (precision de correction).
    // nitroMul     : >1 rend le nitro plus facile a declencher (marge reduite).
    static constexpr float kEasySpeedMul = 0.90f;
    static constexpr float kEasyGripMul = 0.85f;
    static constexpr float kEasyLookaheadMul = 0.80f;
    static constexpr float kEasySteerMul = 0.82f;
    static constexpr float kEasyNitroMul = 0.75f;
    static constexpr float kHardSpeedMul = 1.06f;
    static constexpr float kHardGripMul = 1.15f;
    static constexpr float kHardLookaheadMul = 1.15f;
    static constexpr float kHardSteerMul = 1.15f;
    static constexpr float kHardNitroMul = 1.25f;

    static float normalizeAngle(float angle);
    static unsigned int hashU32(unsigned int value);

    CornerLimits anticipateCorners(
        const Car &car,
        const Track &track,
        float currentDist,
        float speedAbs,
        float rubberBand) const;
    void scanCornerSamples(
        CornerLimits &limits,
        const Car &car,
        const Track &track,
        float currentDist,
        float chord,
        float rubberBand) const;
    void accumulateCornerSample(
        CornerLimits &limits,
        const Car &car,
        float chord,
        float turnPerU,
        int sampleIndex,
        float rubberBand) const;
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
        bool wantNitro,
        float rubberBand) const;
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
        float rubberBand,
        CarInput &input) const;

    float skill_ = 1.0f;
    float laneOffset_ = 0.0f;
    float nitroReserve_ = 1.2f;
    AiDifficulty difficulty_ = AiDifficulty::Normal;
    float speedMul_ = 1.0f;
    float gripMul_ = 1.0f;
    float lookaheadMul_ = 1.0f;
    float steerMul_ = 1.0f;
    float nitroMul_ = 1.0f;
};

} // namespace racer

#endif /* !AI_DRIVER_HPP_ */
