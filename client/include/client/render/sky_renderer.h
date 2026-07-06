#pragma once

#include "raylib.h"

namespace client {

struct DayNightState {
    Color skyColor;
    Color groundTint; // multiplie les couleurs de blocs (assombri la nuit)
};

// timeOfDay01 : 0 = minuit, 0.5 = midi. Interpolation sinusoidale continue,
// pas de paliers brusques entre les phases.
DayNightState ComputeDayNight(float timeOfDay01);

} // namespace client
