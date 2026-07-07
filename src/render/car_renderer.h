#pragma once

#include "raylib.h"
#include "vehicle/car.h"

namespace racer {

inline constexpr float kWheelRadius = 0.35f; // le jeu accumule wheelSpin += speed*dt/kWheelRadius

// Etat visuel purement cosmetique, anime par le jeu (aucun impact physique).
struct CarVisual {
    float steer = 0.0f;      // -1..1 braquage visuel des roues avant
    float wheelSpin = 0.0f;  // angle cumule (radians) de rotation des roues
    bool braking = false;    // feux stop allumes
    bool nitro = false;      // flammes d'echappement
    bool headlights = false; // phares allumes (ambiances sombres)
    bool drifting = false;   // pose plus agressive
};

// Voiture de course en primitives raylib (aucun modele externe), esthetique
// low-poly coloree. ~60-90 draw calls par voiture selon les effets actifs.
void DrawCar(const Car& car, Color bodyColor); // wrapper compat (CarVisual{} par defaut)
void DrawCarEx(const Car& car, const CarVisual& vis, Color bodyColor);

// Points d'ancrage lumineux en coordonnees monde (halos, VFX externes...).
struct CarLightPoints { Vector3 headL, headR, brakeL, brakeR, exhaust; };
CarLightPoints GetCarLightPoints(const Car& car);

} // namespace racer
