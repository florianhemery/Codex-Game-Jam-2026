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

#include "Race/RaceState.hpp"
#include "Track/TrackDef.hpp"

namespace racer {

struct HudExtras {
    float currentLapTime = 0.0f;
    float lastLapTime = 0.0f;
    float bestLapTime = 0.0f;
    std::vector<Color> racerColors;
};

class Hud {
public:
    void draw(const RaceState &race, int screenWidth, int screenHeight);
    void drawHudEx(const RaceState &race, int screenWidth, int screenHeight,
        const HudExtras &extras);
    void drawMenu(const std::vector<TrackDef> &presets, int selectedIndex,
        int screenWidth, int screenHeight);
};

} // namespace racer

#endif /* !HUD_HPP_ */
