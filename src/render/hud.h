#pragma once

#include <vector>

#include "raylib.h"

#include "race/race_state.h"
#include "track/track.h"

namespace racer {

// Donnees optionnelles pour enrichir le HUD (chronos au tour, couleurs des
// coureurs). Les valeurs par defaut donnent un HUD complet mais sans chronos.
struct HudExtras {
    float currentLapTime = 0.0f, lastLapTime = 0.0f, bestLapTime = 0.0f; // 0 = pas encore de donnee
    std::vector<Color> racerColors; // couleur par index de racer ; si vide, palette interne de secours
};

void DrawHud(const RaceState& race, int screenWidth, int screenHeight);
void DrawHudEx(const RaceState& race, int screenWidth, int screenHeight, const HudExtras& extras);

void DrawMenu(const std::vector<TrackDef>& presets, int selectedIndex, int screenWidth, int screenHeight);

} // namespace racer
