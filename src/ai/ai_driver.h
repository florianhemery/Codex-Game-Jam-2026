/*
** EPITECH PROJECT, 2026
** racer
** File description:
** ai_driver
*/

#ifndef AI_DRIVER_H_
#define AI_DRIVER_H_

#include "track/track.h"
#include "vehicle/car.h"

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

    CarInput ComputeInput(const Car &car, const Track &track) const;

private:
    struct CornerLimits {
        float maxTurnPerU = 0.0f;
        float vLimit = 0.0f;
    };

    static constexpr float kGripBudget = 1.4f;
    static constexpr float kMinCornerSpeed = 7.0f;

    static float normalizeAngle(float angle);
    static unsigned int hashU32(unsigned int value);

    CornerLimits anticipateCorners(
        const Car &car,
        const Track &track,
        float currentDist,
        float speedAbs) const;
    void accumulateCornerSample(
        CornerLimits &limits,
        const Car &car,
        float chord,
        float turnPerU,
        int sampleIndex) const;
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

#endif /* !AI_DRIVER_H_ */
