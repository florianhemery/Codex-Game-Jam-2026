#pragma once

#include <vector>

#include "race/race_state.h"
#include "track/track.h"

namespace racer {

void DrawHud(const RaceState& race, int screenWidth, int screenHeight);

void DrawMenu(const std::vector<TrackDef>& presets, int selectedIndex, int screenWidth, int screenHeight);

} // namespace racer
