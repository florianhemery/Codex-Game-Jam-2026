/*
** EPITECH PROJECT, 2026
** racer
** File description:
** HUD minimap rendering
*/

#ifndef HUD_MINIMAP_HPP_
#define HUD_MINIMAP_HPP_

#include <vector>

#include "raylib.h"

#include "Race/RaceState.hpp"
#include "Render/Hud/HudExtras.hpp"
#include "Render/Hud/HudTypes.hpp"

namespace racer {

struct HudMinimap {
    static HudMapProjection fitTrackInRect(
        const std::vector<Vector2> &points, Rectangle area, bool allowRotate);
    static void drawTrackPolyline(const std::vector<Vector2> &points,
        const HudMapProjection &proj, float thickness, Color color);
    static void drawPathPolyline(const std::vector<Vector2> &points,
        const HudMapProjection &proj, float thickness, Color color);
    static void drawFinishLineTick(const std::vector<Vector2> &points,
        const HudMapProjection &proj, float halfLength, Color color);
    static void drawOpponents(const RaceState &race, const HudExtras &extras,
        const HudMapProjection &proj, const Rectangle &panel);
    static void drawPlayer(const RaceState &race, const HudExtras &extras,
        const HudMapProjection &proj, const Rectangle &panel);
    static void draw(const RaceState &race, const HudExtras &extras,
        int screenWidth, int screenHeight);
};

} // namespace racer

#endif /* !HUD_MINIMAP_HPP_ */
