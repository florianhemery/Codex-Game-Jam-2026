/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Pause-menu encyclopedia screen — "plaques des Veilleurs" lore entries
*/

#ifndef HUD_ENCYCLOPEDIA_HPP_
#define HUD_ENCYCLOPEDIA_HPP_

#include <vector>

#include "raylib.h"

#include "World/Sim/ProgressionState.hpp"

namespace racer {

struct HudEncyclopediaLayout {
    Rectangle panel{};
    Rectangle backButton{};
    Rectangle detailPanel{};
    std::vector<Rectangle> tiles;
};

struct HudEncyclopedia {
    static HudEncyclopediaLayout computeLayout(int screenWidth,
        int screenHeight);
    static int pickTile(const HudEncyclopediaLayout &layout, Vector2 mouse);
    static bool hitBack(const HudEncyclopediaLayout &layout, Vector2 mouse);
    // progression may be null (no save loaded yet): every entry then shows
    // as locked. selectedIndex may be -1 (nothing selected yet).
    static void draw(int screenWidth, int screenHeight,
        const HudEncyclopediaLayout &layout,
        const world::ProgressionState *progression, int selectedIndex);
};

} // namespace racer

#endif /* !HUD_ENCYCLOPEDIA_HPP_ */
