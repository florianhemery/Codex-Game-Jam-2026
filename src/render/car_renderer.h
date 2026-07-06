#pragma once

#include "raylib.h"
#include "vehicle/car.h"

namespace racer {

// Voiture en primitives (boites) -- pas de modele 3D, coherent avec
// l'esthetique "low-poly colore" du reste du rendu.
void DrawCar(const Car& car, Color bodyColor);

} // namespace racer
