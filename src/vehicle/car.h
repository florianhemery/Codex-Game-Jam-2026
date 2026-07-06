#pragma once

#include "raylib.h"

namespace racer {

struct CarInput {
    float throttle = 0.0f; // -1 (frein/marche arriere) .. 1 (accelere)
    float steer = 0.0f;    // -1 (gauche) .. 1 (droite)
    bool handbrake = false;
    bool nitro = false;
};

struct CarTuning {
    float maxSpeed = 28.0f;       // unites/s en ligne droite
    float acceleration = 14.0f;   // unites/s^2
    float braking = 24.0f;
    float turnRate = 2.6f;        // rad/s a vitesse nulle-ish
    float gripNormal = 6.0f;      // vitesse de reancrage vitesse->direction (grip eleve = peu de glisse)
    float gripDrift = 0.8f;       // grip pendant le handbrake (glisse fortement)
    float dragCoeff = 0.35f;
    float nitroBoost = 12.0f;     // acceleration additionnelle
    float nitroMaxSpeedBonus = 10.0f;
    float nitroCapacity = 3.0f;   // secondes de nitro
    float nitroRegenPerSecond = 0.4f;
};

// Modele arcade volontairement simplifie (pas de vraie dynamique des pneus) :
// la vitesse scalaire suit l'input, la direction de deplacement (velocity)
// ne "rattrape" le cap (heading) qu'a une vitesse de grip donnee -- baisser
// le grip (handbrake) fait deraper la voiture, exactement l'effet recherche
// pour un jeu de drift arcade.
struct Car {
    Vector3 position{0.0f, 0.0f, 0.0f};
    float heading = 0.0f;     // radians, 0 = +Z
    float speed = 0.0f;       // scalaire signe le long de velocityHeading
    float velocityHeading = 0.0f; // direction reelle du deplacement (peut differer de heading en drift)
    float nitroRemaining = 3.0f;
    bool isDrifting = false;

    CarTuning tuning{};

    void Update(const CarInput& input, float dt);
    Vector3 Forward() const;
    Vector3 Velocity() const;
};

} // namespace racer
