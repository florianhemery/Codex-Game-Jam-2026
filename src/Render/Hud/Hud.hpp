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
#include "Render/Hud/HudExtras.hpp"

namespace racer {

namespace world {
class AureliaWorld;
}

class Hud {
public:
    void draw(const RaceState &race, int screenWidth, int screenHeight);
    void drawHudEx(const RaceState &race, int screenWidth, int screenHeight,
        const HudExtras &extras);
    void drawMainMenu(int screenWidth, int screenHeight, bool showHowToPlay);
    void drawMenu(const std::vector<TrackDef> &presets, int selectedIndex,
        int screenWidth, int screenHeight, bool showHowToPlay = false);
    void drawQuickRaceOverlay(const std::vector<TrackDef> &presets,
        int selectedIndex, int screenWidth, int screenHeight);
    void drawOpenWorldHud(const world::AureliaWorld &world,
        const std::vector<TrackDef> &presets, int screenWidth, int screenHeight);
    void drawPauseOverlay(int screenWidth, int screenHeight);
};

} // namespace racer

#endif /* !HUD_HPP_ */
