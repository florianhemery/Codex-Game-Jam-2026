/*
** EPITECH PROJECT, 2026
** racer
** File description:
** HUD race finish screen
*/

#ifndef HUD_FINISH_SCREEN_HPP_
#define HUD_FINISH_SCREEN_HPP_

#include "raylib.h"

#include "Race/RaceState.hpp"
#include "Render/Hud/HudExtras.hpp"
#include "Render/Hud/HudTypes.hpp"

namespace racer {

struct HudFinishLayout {
    Rectangle panel{};
    Rectangle restartButton{};
    Rectangle menuButton{};
};

struct HudFinishScreen {
    static bool estimateGapSeconds(const RaceState &race,
        const RacerEntry &racer, float *outSeconds);
    static void drawRow(const HudFinishRowParams &params);
    static void drawHeader(const RaceState &race, const HudExtras &extras,
        const Rectangle &panel, float panelW);
    static HudFinishLayout computeLayout(const RaceState &race, int screenWidth,
        int screenHeight);
    static void draw(const RaceState &race, const HudExtras &extras,
        int screenWidth, int screenHeight);
    static bool hitRestart(const HudFinishLayout &layout, Vector2 mouse);
    static bool hitMenu(const HudFinishLayout &layout, Vector2 mouse);
};

} // namespace racer

#endif /* !HUD_FINISH_SCREEN_HPP_ */
