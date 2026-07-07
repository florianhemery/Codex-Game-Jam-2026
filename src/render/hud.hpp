/*
** EPITECH PROJECT, 2026
** racer
** File description:
** HUD rendering for race and menu screens
*/

#ifndef HUD_HPP_
#define HUD_HPP_

#include <vector>

#include "raylib.h"

#include "race/race_state.hpp"
#include "track/track.hpp"

namespace racer {

struct HudExtras {
    float currentLapTime = 0.0f;
    float lastLapTime = 0.0f;
    float bestLapTime = 0.0f;
    std::vector<Color> racerColors;
};

void DrawHud(const RaceState &race, int screenWidth, int screenHeight);
void DrawHudEx(const RaceState &race, int screenWidth, int screenHeight,
    const HudExtras &extras);
void DrawMenu(const std::vector<TrackDef> &presets, int selectedIndex,
    int screenWidth, int screenHeight);

}

#endif /* !HUD_HPP_ */
