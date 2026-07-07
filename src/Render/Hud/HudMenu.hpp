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

struct HudMenu {
    static const HudTrackPreview &getTrackPreview(const TrackDef &def);
    static void drawTrackCardPreview(const TrackDef &def,
        const HudTrackPreview &preview, Rectangle inset, bool selected);
    static void drawTrackCardBadge(const TrackDef &def, Rectangle card);
    static void drawTrackCard(const TrackDef &def, Rectangle card,
        bool selected);
    static void drawTitle(int screenWidth);
    static void drawCards(const std::vector<TrackDef> &presets,
        int selectedIndex, int screenWidth);
};

} // namespace racer

#endif /* !HUD_MENU_HPP_ */
