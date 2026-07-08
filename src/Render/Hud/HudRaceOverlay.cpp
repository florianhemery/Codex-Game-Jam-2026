/*
** EPITECH PROJECT, 2026
** racer
** File description:
** HUD race overlay widgets
*/

#include "Render/Hud/HudRaceOverlay.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "Render/Hud/HudGfx.hpp"

namespace racer {

void HudRaceOverlay::drawStandingsFinishFlag(int x, int y)
{
    HudGfx::drawRectangle(x, y, 5, 5, RAYWHITE);
    HudGfx::drawRectangle(x + 5, y + 5, 5, 5, RAYWHITE);
    HudGfx::drawRectangle(x + 5, y, 5, 5, HudGfx::fade(WHITE, 0.25f));
    HudGfx::drawRectangle(x, y + 5, 5, 5, HudGfx::fade(WHITE, 0.25f));
}

void HudRaceOverlay::drawStandingsRow(const HudStandingsRowParams &params)
{
    if (params.racer.isPlayer) {
        Rectangle highlight{
            params.panel.x + 8.0f, params.rowY - 3.0f,
            params.panel.width - 16.0f, 28.0f - 2.0f
        };
        HudGfx::drawRectangleRounded(highlight, 0.4f, 6, HudGfx::fade(YELLOW, 0.16f));
    }

    char pos[8];

    std::snprintf(pos, sizeof(pos), "%d", params.rank);
    HudGfx::drawTextRightAligned(pos, static_cast<int>(params.panel.x + 34.0f),
        static_cast<int>(params.rowY), 18,
        params.racer.isPlayer ? YELLOW : HudGfx::fade(WHITE, 0.80f));

    Vector2 dot{params.panel.x + 50.0f, params.rowY + 9.0f};

    HudGfx::drawCircleV(dot, 6.0f,
        HudGfx::racerColorFor(params.extras, params.idx, params.racer.isPlayer));
    HudGfx::drawCircleLinesV(dot, 6.0f, HudGfx::fade(WHITE, 0.35f));
    HudGfx::drawText(params.racer.name.c_str(),
        static_cast<int>(params.panel.x + 66.0f),
        static_cast<int>(params.rowY), 18,
        params.racer.isPlayer ? YELLOW : RAYWHITE);
    if (params.racer.finished) {
        drawStandingsFinishFlag(
            static_cast<int>(params.panel.x + params.panel.width - 26.0f),
            static_cast<int>(params.rowY + 3.0f));
    }
}

void HudRaceOverlay::drawStandingsPanel(const RaceState &race, const HudExtras &extras)
{
    const std::vector<RacerEntry> &racers = race.racers();
    const RacerEntry &player = racers[static_cast<size_t>(race.playerIndex())];
    std::vector<int> order = race.standings();
    const float rowH = 28.0f;
    Rectangle panel{
        16.0f, 16.0f, 212.0f,
        50.0f + rowH * static_cast<float>(racers.size()) + 10.0f
    };

    HudGfx::drawPanel(panel, 0.45f);

    char lapBuf[32];

    std::snprintf(lapBuf, sizeof(lapBuf), "TOUR %d/%d",
        std::min(player.lap + 1, race.lapsToWin()), race.lapsToWin());
    HudGfx::drawText(lapBuf, static_cast<int>(panel.x + 14.0f),
        static_cast<int>(panel.y + 12.0f), 20, RAYWHITE);
    HudGfx::drawLineEx(
        Vector2{panel.x + 12.0f, panel.y + 42.0f},
        Vector2{panel.x + panel.width - 12.0f, panel.y + 42.0f},
        1.0f, HudGfx::fade(WHITE, 0.15f));

    for (size_t i = 0; i < order.size(); ++i) {
        size_t idx = static_cast<size_t>(order[i]);
        float rowY = panel.y + 50.0f + rowH * static_cast<float>(i);

        HudStandingsRowParams rowParams{racers[idx], idx,
            static_cast<int>(i) + 1, extras, panel, rowY};

        drawStandingsRow(rowParams);
    }
}

void HudRaceOverlay::drawTimersPanelLastLap(const HudExtras &extras,
    const Rectangle &panel)
{
    char last[32];

    HudGfx::formatTime(extras.lastLapTime, last, sizeof(last));
    float blink = 0.65f + 0.35f * std::sin(
        static_cast<float>(HudGfx::time()) * 8.0f);
    Color lastColor = HudGfx::fade(YELLOW, blink * 0.8f);
    HudGfx::drawText("Dernier", static_cast<int>(panel.x + 14.0f),
        static_cast<int>(panel.y + 120.0f), 16, lastColor);
    HudGfx::drawTextRightAligned(last,
        static_cast<int>(panel.x + panel.width - 14.0f),
        static_cast<int>(panel.y + 120.0f), 16, HudGfx::fade(YELLOW, blink));
}

void HudRaceOverlay::drawTimersPanel(const RaceState &race, const HudExtras &extras,
    int screenWidth)
{
    bool showLast = extras.lastLapTime > 0.0f && extras.currentLapTime < 3.0f;
    const float panelW = 240.0f;
    float panelH = showLast ? 158.0f : 134.0f;
    Rectangle panel{
        static_cast<float>(screenWidth) - panelW - 16.0f,
        16.0f, panelW, panelH
    };

    HudGfx::drawPanel(panel, 0.45f);
    HudGfx::drawText("TEMPS DE COURSE", static_cast<int>(panel.x + 14.0f),
        static_cast<int>(panel.y + 10.0f), 12, HudGfx::fade(WHITE, 0.55f));

    char total[32];

    HudGfx::formatTime(race.elapsedTime(), total, sizeof(total));
    HudGfx::drawText(total, static_cast<int>(panel.x + 14.0f),
        static_cast<int>(panel.y + 26.0f), 30, RAYWHITE);
    HudGfx::drawLineEx(
        Vector2{panel.x + 12.0f, panel.y + 64.0f},
        Vector2{panel.x + panel.width - 12.0f, panel.y + 64.0f},
        1.0f, HudGfx::fade(WHITE, 0.15f));

    char cur[32];

    HudGfx::formatLapTime(extras.currentLapTime, cur, sizeof(cur));
    HudGfx::drawText("Tour", static_cast<int>(panel.x + 14.0f),
        static_cast<int>(panel.y + 72.0f), 16, HudGfx::fade(WHITE, 0.65f));
    HudGfx::drawTextRightAligned(cur, static_cast<int>(panel.x + panel.width - 14.0f),
        static_cast<int>(panel.y + 72.0f), 16, RAYWHITE);

    char best[32];

    HudGfx::formatLapTime(extras.bestLapTime, best, sizeof(best));
    HudGfx::drawText("Meilleur", static_cast<int>(panel.x + 14.0f),
        static_cast<int>(panel.y + 96.0f), 16, HudGfx::fade(WHITE, 0.65f));
    HudGfx::drawTextRightAligned(best, static_cast<int>(panel.x + panel.width - 14.0f),
        static_cast<int>(panel.y + 96.0f), 16, Color{170, 220, 255, 255});
    if (showLast) {
        drawTimersPanelLastLap(extras, panel);
    }
}

void HudRaceOverlay::drawStartLights(int centerX, int y, int litCount, float alpha)
{
    Rectangle band{
        static_cast<float>(centerX) - 96.0f,
        static_cast<float>(y), 192.0f, 56.0f
    };

    HudGfx::drawRectangleRounded(band, 0.5f, 8, HudGfx::fade(BLACK, 0.50f * alpha));
    const Color onColors[3] = {
        Color{235, 64, 52, 255},
        Color{235, 64, 52, 255},
        Color{86, 225, 104, 255}
    };

    for (int i = 0; i < 3; ++i) {
        Vector2 pos{
            static_cast<float>(centerX) + static_cast<float>(i - 1) * 56.0f,
            band.y + 28.0f
        };
        bool on = i < litCount;

        if (on) {
            HudGfx::drawCircleV(pos, 21.0f, HudGfx::fade(onColors[i], 0.30f * alpha));
        }
        Color fill = on ? HudGfx::fade(onColors[i], alpha)
            : HudGfx::fade(WHITE, 0.08f * alpha);
        HudGfx::drawCircleV(pos, 14.0f, fill);
        HudGfx::drawCircleLinesV(pos, 14.5f, HudGfx::fade(WHITE, 0.22f * alpha));
    }
}

void HudRaceOverlay::drawCountdown(const RaceState &race, int screenWidth,
    int screenHeight)
{
    float remaining = race.countdownRemaining();
    int digit = std::clamp(static_cast<int>(std::ceil(remaining)), 1, 3);
    int lit = digit >= 3 ? 1 : 2;

    drawStartLights(screenWidth / 2,
        static_cast<int>(static_cast<float>(screenHeight) * 0.15f),
        lit, 1.0f);

    float age = std::clamp(static_cast<float>(digit) - remaining, 0.0f, 1.0f);
    float k = 1.0f - age;
    float scale = 1.0f + 0.5f * k * k * k;
    int fontSize = static_cast<int>(112.0f * scale);
    char buf[4];

    std::snprintf(buf, sizeof(buf), "%d", digit);
    int textY = static_cast<int>(static_cast<float>(screenHeight) * 0.40f) -
        fontSize / 2;
    HudShadowTextParams shadow{
        screenWidth / 2, textY, fontSize, RAYWHITE, 5
    };

    HudGfx::drawTextShadowCentered(buf, shadow);
}

void HudRaceOverlay::drawGoFlash(const RaceState &race, int screenWidth, int screenHeight)
{
    const float duration = 1.2f;
    float elapsed = race.elapsedTime();

    if (elapsed >= duration) {
        return;
    }
    float alpha = 1.0f - elapsed / duration;

    drawStartLights(screenWidth / 2,
        static_cast<int>(static_cast<float>(screenHeight) * 0.15f),
        3, alpha);
    float flash = 0.72f + 0.28f * std::sin(elapsed * 24.0f);
    const int fontSize = 116;
    int textY = static_cast<int>(static_cast<float>(screenHeight) * 0.40f) -
        fontSize / 2;
    HudShadowTextParams shadow{
        screenWidth / 2, textY, fontSize,
        HudGfx::fade(Color{92, 230, 110, 255}, alpha * flash), 5
    };

    HudGfx::drawTextShadowCentered("GO !", shadow);
}

} // namespace racer
