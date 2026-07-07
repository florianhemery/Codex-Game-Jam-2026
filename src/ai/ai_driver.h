#pragma once

#include "track/track.h"
#include "vehicle/car.h"

namespace racer {

// IA par poursuite de point-cible : projette la voiture sur la piste, vise un
// point plus loin sur la ligne centrale (decale par la "personnalite" du
// pilote), et pilote vers lui. S'y ajoutent une anticipation de freinage par
// echantillonnage de la courbure a venir et une gestion du nitro dependante
// du skill. Pas de A*, pas d'etat mutable -- tout est const apres construction.
class AIDriver {
public:
    // seed = 0 : graine derivee du skill (deterministe). Passer une graine
    // explicite donne des personnalites distinctes meme a skill egal.
    explicit AIDriver(float skill = 1.0f, unsigned int seed = 0);

    CarInput ComputeInput(const Car& car, const Track& track) const;

private:
    float skill_ = 1.0f;        // 1.0 = reference ; <1 plus lent/moins precis, >1 l'inverse
    float laneOffset_ = 0.0f;   // trajectoire preferee : offset lateral (u, +-1.8, reduit en virage)
    float nitroReserve_ = 1.2f; // reserve minimale avant d'oser le nitro (plus haute si faible)
};

} // namespace racer
