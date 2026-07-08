/*
** EPITECH PROJECT, 2026
** racer
** File description:
** HUD track selection menu
*/

#ifndef HUD_MENU_HPP_
#define HUD_MENU_HPP_

#include <vector>

#include "raylib.h"

#include "Track/TrackDef.hpp"
#include "Render/Hud/HudTypes.hpp"

namespace racer {

struct HudMenuLayout {
    std::vector<Rectangle> cards;
    Rectangle startButton;
};

struct HudMenu {
    static HudMenuLayout computeLayout(
        const std::vector<TrackDef> &presets, int screenWidth,
        int screenHeight);
    static int pickCard(const HudMenuLayout &layout, Vector2 mouse);
    static bool hitStartButton(const HudMenuLayout &layout, Vector2 mouse);
    static const HudTrackPreview &getTrackPreview(const TrackDef &def);
    static void drawTrackCardPreview(const TrackDef &def,
        const HudTrackPreview &preview, Rectangle inset, bool selected);
    static void drawTrackCardBadge(const TrackDef &def, Rectangle card);
    static void drawTrackCard(const TrackDef &def, Rectangle card,
        bool selected);
    static void drawTitle(int screenWidth);
    static void drawCards(const std::vector<TrackDef> &presets,
        int selectedIndex, int screenWidth);
    static void drawStartButton(const HudMenuLayout &layout);
};

} // namespace racer

#endif /* !HUD_MENU_HPP_ */
