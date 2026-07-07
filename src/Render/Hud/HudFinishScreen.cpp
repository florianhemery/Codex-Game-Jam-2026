/*
** EPITECH PROJECT, 2026
** racer
** File description:
** HUD race finish screen
*/

#include "Render/Hud/HudFinishScreen.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "Track/Track.hpp"
#include "Render/Hud/HudGfx.hpp"

namespace racer {

bool HudFinishScreen::estimateGapSeconds(const RaceState &race, const RacerEntry &racer,
    float *outSeconds)
{
    const Track &track = race.getTrack();
    Track::Progress prog = track.projectPosition(racer.car.position());
    float done = static_cast<float>(racer.lap) * track.totalLength() +
        track.cumulativeDistance(prog);
    float remaining = static_cast<float>(race.lapsToWin()) *
        track.totalLength() - done;

    if (remaining <= 0.0f) {
        *outSeconds = 0.0f;
        return true;
    }
    float speed = std::fabs(racer.car.speed());

    if (speed < 2.0f) {
        return false;
    }
    float estimate = remaining / speed;

    if (estimate > 120.0f) {
        return false;
    }
    *outSeconds = estimate;
    return true;
}

void HudFinishScreen::drawRow(const HudFinishRowParams &params)
{
    if (params.racer.isPlayer) {
        Rectangle highlight{
            params.panel.x + 16.0f, params.rowY - 4.0f,
            params.panelW - 32.0f, 30.0f - 2.0f
        };
        HudGfx::drawRectangleRounded(highlight, 0.4f, 6, HudGfx::fade(YELLOW, 0.14f));
    }

    char pos[8];

    std::snprintf(pos, sizeof(pos), "%d", params.rank);
    HudGfx::drawTextRightAligned(pos, static_cast<int>(params.panel.x + 46.0f),
        static_cast<int>(params.rowY), 20,
        params.racer.isPlayer ? YELLOW : HudGfx::fade(WHITE, 0.85f));

    Vector2 dot{params.panel.x + 66.0f, params.rowY + 10.0f};

    HudGfx::drawCircleV(dot, 6.5f,
        HudGfx::racerColorFor(params.extras, params.idx, params.racer.isPlayer));
    HudGfx::drawCircleLinesV(dot, 6.5f, HudGfx::fade(WHITE, 0.35f));
    HudGfx::drawText(params.racer.name.c_str(),
        static_cast<int>(params.panel.x + 84.0f),
        static_cast<int>(params.rowY), 20,
        params.racer.isPlayer ? YELLOW : RAYWHITE);

    char right[32];

    if (params.racer.finished) {
        HudGfx::formatTime(params.racer.finishTime, right, sizeof(right));
    } else {
        float gap = 0.0f;

        if (estimateGapSeconds(params.race, params.racer, &gap)) {
            std::snprintf(right, sizeof(right), "+%.2fs", gap);
        } else {
            std::snprintf(right, sizeof(right), "en course");
        }
    }
    HudGfx::drawTextRightAligned(right,
        static_cast<int>(params.panel.x + params.panelW - 24.0f),
        static_cast<int>(params.rowY), 20,
        params.racer.finished ? RAYWHITE : HudGfx::fade(WHITE, 0.60f));
}

void HudFinishScreen::drawHeader(const RaceState &race, const Rectangle &panel,
    float panelW)
{
    char ordinal[16];

    HudGfx::formatOrdinal(race.playerPosition(), ordinal, sizeof(ordinal));
    char title[64];

    std::snprintf(title, sizeof(title), "ARRIVEE -- %s place", ordinal);
    HudShadowTextParams titleShadow{
        static_cast<int>(panel.x + panelW * 0.5f),
        static_cast<int>(panel.y + 20.0f), 34, YELLOW, 3
    };

    drawTextShadowCentered(title, titleShadow);

    const std::vector<RacerEntry> &racers = race.racers();
    const RacerEntry &player = racers[static_cast<size_t>(race.playerIndex())];
    char timeBuf[32];

    HudGfx::formatTime(player.finished ? player.finishTime : race.elapsedTime(),
        timeBuf, sizeof(timeBuf));
    char timeLine[64];

    std::snprintf(timeLine, sizeof(timeLine), "Temps de course : %s", timeBuf);
    HudGfx::drawTextCentered(timeLine, static_cast<int>(panel.x + panelW * 0.5f),
        static_cast<int>(panel.y + 66.0f), 18, HudGfx::fade(WHITE, 0.85f));
    HudGfx::drawLineEx(
        Vector2{panel.x + 24.0f, panel.y + 94.0f},
        Vector2{panel.x + panelW - 24.0f, panel.y + 94.0f},
        1.0f, HudGfx::fade(WHITE, 0.20f));
}

void HudFinishScreen::draw(const RaceState &race, const HudExtras &extras,
    int screenWidth, int screenHeight)
{
    Color scrim = HudGfx::fade(BLACK, 0.65f);
    HudGfx::drawRectangle(0, 0, screenWidth, screenHeight, scrim);

    const std::vector<RacerEntry> &racers = race.racers();
    std::vector<int> order = race.standings();
    const float rowH = 30.0f;
    const float panelW = 540.0f;
    const float headerH = 104.0f;
    float panelH = headerH + rowH * static_cast<float>(racers.size()) + 100.0f;
    Rectangle panel{
        (static_cast<float>(screenWidth) - panelW) * 0.5f,
        (static_cast<float>(screenHeight) - panelH) * 0.5f - 30.0f,
        panelW, panelH
    };

    HudGfx::drawRectangleRounded(panel, 0.08f, 8, HudGfx::fade(BLACK, 0.55f));
    Color panelBorder = HudGfx::fade(YELLOW, 0.35f);
    HudGfx::drawRectangleRoundedLinesEx(panel, 0.08f, 8, 2.0f, panelBorder);
    drawFinishScreenHeader(race, panel, panelW);

    for (size_t i = 0; i < order.size(); ++i) {
        size_t idx = static_cast<size_t>(order[i]);
        float rowY = panel.y + headerH + rowH * static_cast<float>(i);

        HudFinishRowParams rowParams{race, racers[idx], idx,
            static_cast<int>(i) + 1, extras, panel, panelW, rowY};

        drawFinishScreenRow(rowParams);
    }
    HudGfx::drawTextCentered("R rejouer      M menu",
        static_cast<int>(panel.x + panelW * 0.5f),
        static_cast<int>(panel.y + panelH - 40.0f), 20, LIGHTGRAY);
}

} // namespace racer
