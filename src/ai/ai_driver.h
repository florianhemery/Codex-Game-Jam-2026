#pragma once

#include "track/track.h"
#include "vehicle/car.h"

namespace racer {

// IA minimale par poursuite de point-cible : projette la voiture sur la
// piste, vise un point plus loin sur la ligne centrale, et pilote vers lui.
// Pas de A* ni d'evitement d'obstacles -- suffisant pour une piste a une
// seule voie sans obstacles mobiles.
class AIDriver {
public:
    explicit AIDriver(float skill = 1.0f) : skill_(skill) {}

    CarInput ComputeInput(const Car& car, const Track& track) const;

private:
    float skill_ = 1.0f; // 1.0 = reference ; <1 plus lent/moins precis, >1 l'inverse
};

} // namespace racer
