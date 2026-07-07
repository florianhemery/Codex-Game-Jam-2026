/*
** EPITECH PROJECT, 2026
** racer
** File description:
** HUD race overlay widgets
*/

#ifndef HUD_RACE_OVERLAY_HPP_
#define HUD_RACE_OVERLAY_HPP_

#include "raylib.h"

#include "Race/RaceState.hpp"
#include "Render/Hud/HudExtras.hpp"
#include "Render/Hud/HudTypes.hpp"
#include "Vehicle/Car.hpp"

namespace racer {

struct HudRaceOverlay {
    static void drawSpeedGaugeArc(const HudGaugeArcParams &params);
    static void drawSpeedGaugeTicks(const Vector2 &center, float kmhMax,
        float angleMin, float angleSpan, float rOut);
    static void drawSpeedGaugeNeedle(const Vector2 &center, float ratio,
        float angleMin, float angleSpan, float rIn);
    static void drawNitroBar(const Car &car, const Rectangle &panel,
        float panelW, float panelH);
    static void drawSpeedGauge(const Car &car, int screenHeight);
    static void drawStandingsFinishFlag(int x, int y);
    static void drawStandingsRow(const HudStandingsRowParams &params);
    static void drawStandingsPanel(const RaceState &race,
        const HudExtras &extras);
    static void drawTimersPanelLastLap(const HudExtras &extras,
        const Rectangle &panel);
    static void drawTimersPanel(const RaceState &race, const HudExtras &extras,
        int screenWidth);
    static void drawStartLights(int centerX, int y, int litCount, float alpha);
    static void drawCountdown(const RaceState &race, int screenWidth,
        int screenHeight);
    static void drawGoFlash(const RaceState &race, int screenWidth,
        int screenHeight);
};

} // namespace racer

#endif /* !HUD_RACE_OVERLAY_HPP_ */
