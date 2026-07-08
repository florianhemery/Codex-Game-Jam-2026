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

namespace {

// Podium-style rank coloring (gold/silver/bronze) so the top 3 read
// instantly, matching the "real conclusion" feel the finish screen should
// have rather than a plain sorted list.
Color medalColorFor(int rank, bool isPlayer)
{
    if (isPlayer) {
        return YELLOW;
    }
    switch (rank) {
        case 1:
            return Color{255, 215, 90, 255};
        case 2:
            return Color{205, 210, 218, 255};
        case 3:
            return Color{205, 145, 90, 255};
        default:
            return HudGfx::fade(WHITE, 0.85f);
    }
}

} // namespace

void HudFinishScreen::drawRow(const HudFinishRowParams &params)
{
    if (params.racer.isPlayer) {
        Rectangle highlight{
            params.panel.x + 16.0f, params.rowY - 4.0f,
            params.panelW - 32.0f, 30.0f - 2.0f
        };
        HudGfx::drawRectangleRounded(highlight, 0.4f, 6, HudGfx::fade(YELLOW, 0.14f));
    }

    Color rankColor = medalColorFor(params.rank, params.racer.isPlayer);
    char pos[8];

    std::snprintf(pos, sizeof(pos), "%d", params.rank);
    HudGfx::drawTextRightAligned(pos, static_cast<int>(params.panel.x + 46.0f),
        static_cast<int>(params.rowY), 20, rankColor);
    if (params.rank <= 3) {
        HudGfx::drawCircleLinesV(
            Vector2{params.panel.x + 30.0f, params.rowY + 10.0f}, 12.0f,
            HudGfx::fade(rankColor, 0.55f));
    }

    Vector2 dot{params.panel.x + 66.0f, params.rowY + 10.0f};

    HudGfx::drawCircleV(dot, 6.5f,
        HudGfx::racerColorFor(params.extras, params.idx, params.racer.isPlayer));
    HudGfx::drawCircleLinesV(dot, 6.5f, HudGfx::fade(WHITE, 0.35f));
    HudGfx::drawText(params.racer.name.c_str(),
        static_cast<int>(params.panel.x + 84.0f),
        static_cast<int>(params.rowY), 20,
        params.racer.isPlayer ? YELLOW : RAYWHITE);

    char right[32];
    char gap[24] = "";

    if (params.racer.finished) {
        HudGfx::formatTime(params.racer.finishTime, right, sizeof(right));
        if (params.rank > 1 && params.leaderFinishTime > 0.0f) {
            std::snprintf(gap, sizeof(gap), "+%.2fs",
                params.racer.finishTime - params.leaderFinishTime);
        }
    } else {
        float estSeconds = 0.0f;
        if (estimateGapSeconds(params.race, params.racer, &estSeconds)
            && estSeconds > 0.5f) {
            std::snprintf(right, sizeof(right), "~%.0fs", estSeconds);
        } else {
            std::snprintf(right, sizeof(right), "DNF");
        }
    }
    int timeRightX = static_cast<int>(params.panel.x + params.panelW - 24.0f);

    HudGfx::drawTextRightAligned(right, timeRightX,
        static_cast<int>(params.rowY), 20,
        params.racer.finished ? RAYWHITE : HudGfx::fade(WHITE, 0.60f));
    if (gap[0] != '\0') {
        int timeWidth = HudGfx::measureText(right, 20);

        HudGfx::drawTextRightAligned(gap, timeRightX - timeWidth - 10,
            static_cast<int>(params.rowY) + 4, 12, HudGfx::fade(WHITE, 0.45f));
    }
}

void HudFinishScreen::drawHeader(const RaceState &race, const HudExtras &extras,
    const Rectangle &panel, float panelW)
{
    char ordinal[16];

    HudGfx::formatOrdinal(race.playerPosition(), ordinal, sizeof(ordinal));
    char title[64];

    std::snprintf(title, sizeof(title), "ARRIVEE -- %s place", ordinal);
    HudShadowTextParams titleShadow{
        static_cast<int>(panel.x + panelW * 0.5f),
        static_cast<int>(panel.y + 20.0f), 34, YELLOW, 3
    };

    HudGfx::drawTextShadowCentered(title, titleShadow);

    const std::vector<RacerEntry> &racers = race.racers();
    const RacerEntry &player = racers[static_cast<size_t>(race.playerIndex())];
    char timeBuf[32];

    HudGfx::formatTime(player.finished ? player.finishTime : race.elapsedTime(),
        timeBuf, sizeof(timeBuf));
    char timeLine[64];

    std::snprintf(timeLine, sizeof(timeLine), "Temps total : %s", timeBuf);
    HudGfx::drawTextCentered(timeLine, static_cast<int>(panel.x + panelW * 0.5f),
        static_cast<int>(panel.y + 66.0f), 18, HudGfx::fade(WHITE, 0.85f));

    char bestBuf[32];
    char statsLine[96];

    if (extras.bestLapTime > 0.0f) {
        HudGfx::formatLapTime(extras.bestLapTime, bestBuf, sizeof(bestBuf));
        std::snprintf(statsLine, sizeof(statsLine),
            "Meilleur tour : %s   |   Derapages : %d", bestBuf, extras.driftCount);
    } else {
        std::snprintf(statsLine, sizeof(statsLine),
            "Derapages : %d", extras.driftCount);
    }
    HudGfx::drawTextCentered(statsLine, static_cast<int>(panel.x + panelW * 0.5f),
        static_cast<int>(panel.y + 88.0f), 16, HudGfx::fade(WHITE, 0.65f));
    HudGfx::drawLineEx(
        Vector2{panel.x + 24.0f, panel.y + 110.0f},
        Vector2{panel.x + panelW - 24.0f, panel.y + 110.0f},
        1.0f, HudGfx::fade(WHITE, 0.20f));
}

HudFinishLayout HudFinishScreen::computeLayout(const RaceState &race,
    int screenWidth, int screenHeight)
{
    HudFinishLayout layout;
    const std::vector<RacerEntry> &racers = race.racers();
    const float rowH = 30.0f;
    const float panelW = 540.0f;
    const float headerH = 120.0f;
    const float footerH = 72.0f;
    float panelH = headerH + rowH * static_cast<float>(racers.size()) + footerH;

    layout.panel = Rectangle{
        (static_cast<float>(screenWidth) - panelW) * 0.5f,
        (static_cast<float>(screenHeight) - panelH) * 0.5f - 20.0f,
        panelW, panelH
    };

    const float btnW = 180.0f;
    const float btnH = 42.0f;
    const float gap = 20.0f;
    float btnY = layout.panel.y + panelH - btnH - 18.0f;
    float totalBtnW = btnW * 2.0f + gap;
    float btnX = layout.panel.x + (panelW - totalBtnW) * 0.5f;

    layout.restartButton = Rectangle{btnX, btnY, btnW, btnH};
    layout.menuButton = Rectangle{btnX + btnW + gap, btnY, btnW, btnH};
    return layout;
}

bool HudFinishScreen::hitRestart(const HudFinishLayout &layout, Vector2 mouse)
{
    return CheckCollisionPointRec(mouse, layout.restartButton);
}

bool HudFinishScreen::hitMenu(const HudFinishLayout &layout, Vector2 mouse)
{
    return CheckCollisionPointRec(mouse, layout.menuButton);
}

void HudFinishScreen::draw(const RaceState &race, const HudExtras &extras,
    int screenWidth, int screenHeight)
{
    Color scrim = HudGfx::fade(BLACK, 0.65f);
    HudGfx::drawRectangle(0, 0, screenWidth, screenHeight, scrim);

    HudFinishLayout layout = computeLayout(race, screenWidth, screenHeight);
    const std::vector<RacerEntry> &racers = race.racers();
    std::vector<int> order = race.standings();
    const float rowH = 30.0f;
    const float headerH = 120.0f;
    float panelW = layout.panel.width;
    Vector2 mouse = GetMousePosition();
    bool hoverRestart = CheckCollisionPointRec(mouse, layout.restartButton);
    bool hoverMenu = CheckCollisionPointRec(mouse, layout.menuButton);

    HudGfx::drawRectangleRounded(layout.panel, 0.08f, 8, HudGfx::fade(BLACK, 0.55f));
    Color panelBorder = HudGfx::fade(YELLOW, 0.35f);
    HudGfx::drawRectangleRoundedLinesEx(layout.panel, 0.08f, 8, 2.0f, panelBorder);
    drawHeader(race, extras, layout.panel, panelW);

    const RacerEntry &leader = racers[static_cast<size_t>(order[0])];
    float leaderFinishTime = leader.finished ? leader.finishTime : 0.0f;

    for (size_t i = 0; i < order.size(); ++i) {
        size_t idx = static_cast<size_t>(order[i]);
        float rowY = layout.panel.y + headerH + rowH * static_cast<float>(i);

        HudFinishRowParams rowParams{race, racers[idx], idx,
            static_cast<int>(i) + 1, extras, layout.panel, panelW, rowY,
            leaderFinishTime};

        drawRow(rowParams);
    }

    HudGfx::drawRectangleRounded(layout.restartButton, 0.35f, 8,
        hoverRestart ? Color{255, 196, 40, 255} : Color{230, 150, 24, 255});
    HudGfx::drawTextCentered("Rejouer",
        static_cast<int>(layout.restartButton.x + layout.restartButton.width * 0.5f),
        static_cast<int>(layout.restartButton.y + 10.0f), 22, BLACK);
    HudGfx::drawRectangleRounded(layout.menuButton, 0.35f, 8,
        hoverMenu ? HudGfx::fade(WHITE, 0.25f) : HudGfx::fade(WHITE, 0.12f));
    HudGfx::drawTextCentered("Menu",
        static_cast<int>(layout.menuButton.x + layout.menuButton.width * 0.5f),
        static_cast<int>(layout.menuButton.y + 10.0f), 22, RAYWHITE);
    HudGfx::drawTextCentered("R / Entree : rejouer   |   M / Echap : menu",
        static_cast<int>(layout.panel.x + layout.panel.width * 0.5f),
        static_cast<int>(layout.panel.y + layout.panel.height - 8.0f),
        14, HudGfx::fade(WHITE, 0.45f));
}

} // namespace racer
