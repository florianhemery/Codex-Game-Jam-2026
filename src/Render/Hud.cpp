/*
** EPITECH PROJECT, 2026
** racer
** File description:
** HUD rendering for race and menu screens
*/

#include "Render/Hud.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

#include "raylib.h"

namespace racer {

int Hud::Gfx::measureText(const char *text, int fontSize)
{
    return MeasureText(text, fontSize);
}

float Hud::Gfx::time()
{
    return static_cast<float>(GetTime());
}

Color Hud::Gfx::fade(Color color, float alpha)
{
    return Fade(color, alpha);
}

void Hud::Gfx::drawText(const char *text, int x, int y, int fontSize,
    Color color)
{
    DrawText(text, x, y, fontSize, color);
}

void Hud::Gfx::drawRectangleRounded(Rectangle rect, float roundness,
    int segments, Color color)
{
    DrawRectangleRounded(rect, roundness, segments, color);
}

void Hud::Gfx::drawRectangleRoundedLinesEx(Rectangle rect, float roundness,
    int segments, float lineThick, Color color)
{
    DrawRectangleRoundedLinesEx(rect, roundness, segments, lineThick, color);
}

void Hud::Gfx::drawLineEx(Vector2 start, Vector2 end, float thick, Color color)
{
    DrawLineEx(start, end, thick, color);
}

void Hud::Gfx::drawCircleV(Vector2 center, float radius, Color color)
{
    DrawCircleV(center, radius, color);
}

void Hud::Gfx::drawCircleLinesV(Vector2 center, float radius, Color color)
{
    DrawCircleLinesV(center, radius, color);
}

void Hud::Gfx::drawRing(Vector2 center, float innerR, float outerR,
    float startAngle, float endAngle, int segments, Color color)
{
    DrawRing(center, innerR, outerR, startAngle, endAngle, segments, color);
}

void Hud::Gfx::drawRectangle(int x, int y, int w, int h, Color color)
{
    DrawRectangle(x, y, w, h, color);
}

void Hud::Gfx::clearBackground(Color color)
{
    ClearBackground(color);
}

void Hud::Gfx::drawRectangleGradientV(int x, int y, int w, int h,
    Color color1, Color color2)
{
    DrawRectangleGradientV(x, y, w, h, color1, color2);
}


void Hud::formatOrdinal(int position, char *buf, size_t bufSize)
{
    if (position == 1) {
        std::snprintf(buf, bufSize, "1re");
    } else {
        std::snprintf(buf, bufSize, "%de", position);
    }
}

void Hud::formatTime(float seconds, char *buf, size_t bufSize)
{
    int minutes = static_cast<int>(seconds) / 60;
    float secs = seconds - static_cast<float>(minutes * 60);

    std::snprintf(buf, bufSize, "%d:%05.2f", minutes, secs);
}

void Hud::formatLapTime(float seconds, char *buf, size_t bufSize)
{
    if (seconds <= 0.0f) {
        std::snprintf(buf, bufSize, "--");
        return;
    }
    formatTime(seconds, buf, bufSize);
}

Color Hud::lerpColor(Color a, Color b, float t)
{
    t = std::clamp(t, 0.0f, 1.0f);
    auto mix = [t](unsigned char x, unsigned char y) {
        float delta = static_cast<float>(y) - static_cast<float>(x);

        return static_cast<unsigned char>(
            static_cast<float>(x) + delta * t);
    };

    return Color{mix(a.r, b.r), mix(a.g, b.g), mix(a.b, b.b), mix(a.a, b.a)};
}

Color Hud::racerColorFor(const HudExtras &extras, size_t index, bool isPlayer)
{
    if (index < extras.racerColors.size()) {
        return extras.racerColors[index];
    }
    if (isPlayer) {
        return RED;
    }
    constexpr Color kFallback[] = {
        BLUE, DARKGREEN, ORANGE, PURPLE, SKYBLUE, MAROON
    };

    return kFallback[index % (sizeof(kFallback) / sizeof(kFallback[0]))];
}

void Hud::drawPanel(Rectangle rect, float alpha)
{
    Gfx::drawRectangleRounded(rect, 0.12f, 8, Gfx::fade(BLACK, alpha));
    Gfx::drawRectangleRoundedLinesEx(rect, 0.12f, 8, 1.0f,
        Gfx::fade(WHITE, 0.10f));
}

void Hud::drawTextCentered(const char *text, int centerX, int y, int fontSize,
    Color color)
{
    int width = Gfx::measureText(text, fontSize);

    Gfx::drawText(text, centerX - width / 2, y, fontSize, color);
}

void Hud::drawTextRightAligned(const char *text, int rightX, int y,
    int fontSize, Color color)
{
    int width = Gfx::measureText(text, fontSize);

    Gfx::drawText(text, rightX - width, y, fontSize, color);
}

void Hud::drawTextShadowCentered(const char *text,
    const ShadowTextParams &params)
{
    int width = Gfx::measureText(text, params.fontSize);

    Gfx::drawText(text, params.centerX - width / 2 + params.offset,
        params.y + params.offset, params.fontSize, Gfx::fade(BLACK, 0.55f));
    Gfx::drawText(text, params.centerX - width / 2, params.y, params.fontSize,
        params.color);
}

void Hud::drawTextWrapped(const char *text, const WrappedTextParams &params)
{
    char line[192] = {0};
    int lineLen = 0;
    int lines = 0;
    const char *cursor = text;

    while (true) {
        while (*cursor == ' ') {
            ++cursor;
        }
        if (*cursor == '\0') {
            break;
        }

        char word[96];
        int wordLen = 0;

        while (*cursor != '\0' && *cursor != ' ' && wordLen < 95) {
            word[wordLen++] = *cursor++;
        }
        word[wordLen] = '\0';

        char candidate[192];
        if (lineLen == 0) {
            std::snprintf(candidate, sizeof(candidate), "%s", word);
        } else {
            std::snprintf(candidate, sizeof(candidate), "%s %s", line, word);
        }

        if (lineLen > 0 && Gfx::measureText(candidate, params.fontSize) >
            params.maxWidth) {
            Gfx::drawText(line, params.x, params.y + lines * params.lineHeight,
                params.fontSize, params.color);
            ++lines;
            lineLen = std::snprintf(line, sizeof(line), "%s", word);
        } else {
            lineLen = std::snprintf(line, sizeof(line), "%s", candidate);
        }
    }
    if (lineLen > 0) {
        Gfx::drawText(line, params.x, params.y + lines * params.lineHeight,
            params.fontSize, params.color);
    }
}

Vector2 Hud::MapProjection::apply(Vector2 world) const
{
    float dx = world.x - worldCenter.x;
    float dz = world.y - worldCenter.y;

    if (rotated) {
        return Vector2{
            screenCenter.x + dz * scale,
            screenCenter.y + dx * scale
        };
    }
    return Vector2{
        screenCenter.x + dx * scale,
        screenCenter.y - dz * scale
    };
}

Hud::MapProjection Hud::fitTrackInRect(const std::vector<Vector2> &points,
    Rectangle area, bool allowRotate)
{
    Vector2 minPoint{1e9f, 1e9f};
    Vector2 maxPoint{-1e9f, -1e9f};

    for (const Vector2 &point : points) {
        minPoint.x = std::min(minPoint.x, point.x);
        minPoint.y = std::min(minPoint.y, point.y);
        maxPoint.x = std::max(maxPoint.x, point.x);
        maxPoint.y = std::max(maxPoint.y, point.y);
    }
    float bw = std::max(maxPoint.x - minPoint.x, 0.001f);
    float bh = std::max(maxPoint.y - minPoint.y, 0.001f);

    Hud::MapProjection proj;
    proj.worldCenter = Vector2{
        (minPoint.x + maxPoint.x) * 0.5f,
        (minPoint.y + maxPoint.y) * 0.5f
    };
    proj.screenCenter = Vector2{
        area.x + area.width * 0.5f,
        area.y + area.height * 0.5f
    };
    float scaleStraight = std::min(area.width / bw, area.height / bh);
    float scaleRotated = std::min(area.width / bh, area.height / bw);

    proj.rotated = allowRotate && scaleRotated > scaleStraight;
    proj.scale = proj.rotated ? scaleRotated : scaleStraight;
    return proj;
}

void Hud::drawTrackPolyline(const std::vector<Vector2> &points,
    const Hud::MapProjection &proj, float thickness, Color color)
{
    size_t count = points.size();

    for (size_t i = 0; i < count; ++i) {
        Vector2 a = proj.apply(points[i]);
        Vector2 b = proj.apply(points[(i + 1) % count]);

        Gfx::drawLineEx(a, b, thickness, color);
        Gfx::drawCircleV(a, thickness * 0.5f, color);
    }
}

void Hud::drawFinishLineTick(const std::vector<Vector2> &points,
    const Hud::MapProjection &proj, float halfLength, Color color)
{
    if (points.size() < 2) {
        return;
    }
    Vector2 a = proj.apply(points[0]);
    Vector2 b = proj.apply(points[1]);
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float len = std::sqrt(dx * dx + dy * dy);

    if (len < 0.0001f) {
        return;
    }
    Vector2 perp{-dy / len, dx / len};

    Gfx::drawLineEx(
        Vector2{a.x + perp.x * halfLength, a.y + perp.y * halfLength},
        Vector2{a.x - perp.x * halfLength, a.y - perp.y * halfLength},
        3.0f, color);
}

void Hud::drawMinimapOpponents(const RaceState &race, const HudExtras &extras,
    const Hud::MapProjection &proj, const Rectangle &panel)
{
    const std::vector<RacerEntry> &racers = race.racers();

    for (size_t i = 0; i < racers.size(); ++i) {
        if (racers[i].isPlayer) {
            continue;
        }
        Vector2 pos = proj.apply(Vector2{
            racers[i].car.position().x,
            racers[i].car.position().z
        });
        pos.x = std::clamp(pos.x, panel.x + 10.0f,
            panel.x + panel.width - 10.0f);
        pos.y = std::clamp(pos.y, panel.y + 10.0f,
            panel.y + panel.height - 10.0f);
        Gfx::drawCircleV(pos, 4.5f, racerColorFor(extras, i, false));
        Gfx::drawCircleLinesV(pos, 4.5f, Gfx::fade(BLACK, 0.55f));
    }
}

void Hud::drawMinimapPlayer(const RaceState &race, const HudExtras &extras,
    const Hud::MapProjection &proj, const Rectangle &panel)
{
    const std::vector<RacerEntry> &racers = race.racers();
    size_t playerIdx = static_cast<size_t>(race.playerIndex());
    Vector2 pos = proj.apply(Vector2{
        racers[playerIdx].car.position().x,
        racers[playerIdx].car.position().z
    });

    pos.x = std::clamp(pos.x, panel.x + 10.0f, panel.x + panel.width - 10.0f);
    pos.y = std::clamp(pos.y, panel.y + 10.0f, panel.y + panel.height - 10.0f);
    Gfx::drawCircleV(pos, 6.0f, racerColorFor(extras, playerIdx, true));
    Gfx::drawRing(pos, 7.5f, 9.5f, 0.0f, 360.0f, 24, WHITE);
}

void Hud::drawMinimap(const RaceState &race, const HudExtras &extras,
    int screenWidth, int screenHeight)
{
    const float size = 220.0f;
    Rectangle panel{
        static_cast<float>(screenWidth) - size - 16.0f,
        static_cast<float>(screenHeight) - size - 16.0f,
        size, size
    };

    drawPanel(panel, 0.45f);

    const std::vector<Vector2> &waypoints = race.getTrack().waypoints();

    if (waypoints.size() < 2) {
        return;
    }
    Rectangle area{
        panel.x + 20.0f, panel.y + 20.0f,
        panel.width - 40.0f, panel.height - 40.0f
    };
    Hud::MapProjection proj = fitTrackInRect(waypoints, area, false);

    float road = std::clamp(race.getTrack().width() * proj.scale, 3.0f, 10.0f);

    drawTrackPolyline(waypoints, proj, road, Gfx::fade(WHITE, 0.15f));
    drawTrackPolyline(waypoints, proj, 1.5f, Gfx::fade(WHITE, 0.28f));
    drawFinishLineTick(waypoints, proj, road * 0.8f + 2.0f, RAYWHITE);
    drawMinimapOpponents(race, extras, proj, panel);
    drawMinimapPlayer(race, extras, proj, panel);
}

void Hud::drawSpeedGaugeArc(const GaugeArcParams &params)
{
    Gfx::drawRing(params.center, params.rIn, params.rOut, params.angleMin,
        params.angleMin + params.angleSpan, 64, Gfx::fade(WHITE, 0.08f));
    float redlineStart = params.angleMin + params.angleSpan *
        (200.0f / params.kmhMax);

    Gfx::drawRing(params.center, params.rIn, params.rOut, redlineStart,
        params.angleMin + params.angleSpan, 16, Gfx::fade(RED, 0.20f));
    if (params.ratio > 0.004f) {
        Color fill = lerpColor(
            Color{255, 168, 40, 255},
            Color{255, 66, 40, 255},
            params.ratio);
        Gfx::drawRing(params.center, params.rIn + 1.0f, params.rOut - 1.0f,
            params.angleMin, params.angleMin + params.angleSpan * params.ratio,
            64, fill);
    }
}

void Hud::drawSpeedGaugeTicks(const Vector2 &center, float kmhMax,
    float angleMin,
    float angleSpan, float rOut)
{
    for (int speed = 0; speed <= static_cast<int>(kmhMax); speed += 10) {
        bool major = (speed % 50) == 0;
        float angle = (angleMin + angleSpan * static_cast<float>(speed) /
            kmhMax) * DEG2RAD;
        Vector2 dir{std::cos(angle), std::sin(angle)};
        float r1 = rOut + 3.0f;
        float r2 = rOut + (major ? 10.0f : 6.0f);

        Gfx::drawLineEx(
            Vector2{center.x + dir.x * r1, center.y + dir.y * r1},
            Vector2{center.x + dir.x * r2, center.y + dir.y * r2},
            major ? 2.0f : 1.0f,
            Gfx::fade(WHITE, major ? 0.70f : 0.32f));
        if (major) {
            char label[8];

            std::snprintf(label, sizeof(label), "%d", speed);
            float labelRadius = rOut + 20.0f;
            drawTextCentered(label,
                static_cast<int>(center.x + dir.x * labelRadius),
                static_cast<int>(center.y + dir.y * labelRadius) - 5,
                10, Gfx::fade(WHITE, 0.55f));
        }
    }
}

void Hud::drawSpeedGaugeNeedle(const Vector2 &center, float ratio,
    float angleMin,
    float angleSpan, float rIn)
{
    float angle = (angleMin + angleSpan * ratio) * DEG2RAD;
    Vector2 dir{std::cos(angle), std::sin(angle)};
    Vector2 tip{
        center.x + dir.x * (rIn - 3.0f),
        center.y + dir.y * (rIn - 3.0f)
    };

    Gfx::drawLineEx(
        Vector2{center.x + dir.x * 30.0f, center.y + dir.y * 30.0f},
        tip, 3.0f, Color{255, 92, 70, 255});
    Gfx::drawCircleV(tip, 2.5f, RAYWHITE);
}

void Hud::drawNitroBar(const Car &car, const Rectangle &panel, float panelW,
    float panelH)
{
    float nitroRatio = std::clamp(
        car.nitroRemaining() / car.tuning().nitroCapacity, 0.0f, 1.0f);
    float barY = panel.y + panelH - 30.0f;
    Rectangle barBg{panel.x + 86.0f, barY, panelW - 86.0f - 18.0f, 13.0f};

    Gfx::drawText("NITRO", static_cast<int>(panel.x + 20.0f),
        static_cast<int>(barY), 13, Gfx::fade(ORANGE, 0.95f));
    Gfx::drawRectangleRounded(barBg, 0.7f, 6, Gfx::fade(WHITE, 0.10f));
    if (nitroRatio <= 0.01f) {
        return;
    }
    Color col = ORANGE;

    if (nitroRatio >= 0.999f) {
        float pulse = 0.5f + 0.5f * std::sin(
            static_cast<float>(Gfx::time()) * 6.0f);
        col = lerpColor(ORANGE, Color{255, 236, 120, 255}, pulse * 0.65f);
    }
    Rectangle fill{barBg.x, barBg.y, barBg.width * nitroRatio, barBg.height};

    Gfx::drawRectangleRounded(fill, 0.7f, 6, col);
}

void Hud::drawSpeedGauge(const Car &car, int screenHeight)
{
    const float panelW = 272.0f;
    const float panelH = 232.0f;
    Rectangle panel{
        16.0f,
        static_cast<float>(screenHeight) - panelH - 16.0f,
        panelW, panelH
    };

    drawPanel(panel, 0.50f);

    const Vector2 center{panel.x + panelW * 0.5f, panel.y + 108.0f};
    const float rOut = 82.0f;
    const float rIn = 70.0f;
    const float angleMin = 150.0f;
    const float angleSpan = 240.0f;
    const float kmhMax = 230.0f;
    float kmh = std::fabs(car.speed()) * 6.0f;
    float ratio = std::clamp(kmh / kmhMax, 0.0f, 1.0f);

    GaugeArcParams arc{center, ratio, kmhMax, angleMin, angleSpan, rIn, rOut};

    drawSpeedGaugeArc(arc);
    drawSpeedGaugeTicks(center, kmhMax, angleMin, angleSpan, rOut);
    drawSpeedGaugeNeedle(center, ratio, angleMin, angleSpan, rIn);

    char speedBuf[16];

    std::snprintf(speedBuf, sizeof(speedBuf), "%.0f", kmh);
    drawTextCentered(speedBuf, static_cast<int>(center.x),
        static_cast<int>(center.y) - 16, 40, RAYWHITE);
    drawTextCentered("km/h", static_cast<int>(center.x),
        static_cast<int>(center.y) + 28, 14, Gfx::fade(WHITE, 0.60f));
    drawNitroBar(car, panel, panelW, panelH);
}

void Hud::drawStandingsFinishFlag(int x, int y)
{
    Gfx::drawRectangle(x, y, 5, 5, RAYWHITE);
    Gfx::drawRectangle(x + 5, y + 5, 5, 5, RAYWHITE);
    Gfx::drawRectangle(x + 5, y, 5, 5, Gfx::fade(WHITE, 0.25f));
    Gfx::drawRectangle(x, y + 5, 5, 5, Gfx::fade(WHITE, 0.25f));
}

void Hud::drawStandingsRow(const StandingsRowParams &params)
{
    if (params.racer.isPlayer) {
        Rectangle highlight{
            params.panel.x + 8.0f, params.rowY - 3.0f,
            params.panel.width - 16.0f, 28.0f - 2.0f
        };
        Gfx::drawRectangleRounded(highlight, 0.4f, 6, Gfx::fade(YELLOW, 0.16f));
    }

    char pos[8];

    std::snprintf(pos, sizeof(pos), "%d", params.rank);
    drawTextRightAligned(pos, static_cast<int>(params.panel.x + 34.0f),
        static_cast<int>(params.rowY), 18,
        params.racer.isPlayer ? YELLOW : Gfx::fade(WHITE, 0.80f));

    Vector2 dot{params.panel.x + 50.0f, params.rowY + 9.0f};

    Gfx::drawCircleV(dot, 6.0f,
        racerColorFor(params.extras, params.idx, params.racer.isPlayer));
    Gfx::drawCircleLinesV(dot, 6.0f, Gfx::fade(WHITE, 0.35f));
    Gfx::drawText(params.racer.name.c_str(),
        static_cast<int>(params.panel.x + 66.0f),
        static_cast<int>(params.rowY), 18,
        params.racer.isPlayer ? YELLOW : RAYWHITE);
    if (params.racer.finished) {
        drawStandingsFinishFlag(
            static_cast<int>(params.panel.x + params.panel.width - 26.0f),
            static_cast<int>(params.rowY + 3.0f));
    }
}

void Hud::drawStandingsPanel(const RaceState &race, const HudExtras &extras)
{
    const std::vector<RacerEntry> &racers = race.racers();
    const RacerEntry &player = racers[static_cast<size_t>(race.playerIndex())];
    std::vector<int> order = race.standings();
    const float rowH = 28.0f;
    Rectangle panel{
        16.0f, 16.0f, 212.0f,
        50.0f + rowH * static_cast<float>(racers.size()) + 10.0f
    };

    drawPanel(panel, 0.45f);

    char lapBuf[32];

    std::snprintf(lapBuf, sizeof(lapBuf), "TOUR %d/%d",
        std::min(player.lap + 1, race.lapsToWin()), race.lapsToWin());
    Gfx::drawText(lapBuf, static_cast<int>(panel.x + 14.0f),
        static_cast<int>(panel.y + 12.0f), 20, RAYWHITE);
    Gfx::drawLineEx(
        Vector2{panel.x + 12.0f, panel.y + 42.0f},
        Vector2{panel.x + panel.width - 12.0f, panel.y + 42.0f},
        1.0f, Gfx::fade(WHITE, 0.15f));

    for (size_t i = 0; i < order.size(); ++i) {
        size_t idx = static_cast<size_t>(order[i]);
        float rowY = panel.y + 50.0f + rowH * static_cast<float>(i);

        StandingsRowParams rowParams{racers[idx], idx,
            static_cast<int>(i) + 1, extras, panel, rowY};

        drawStandingsRow(rowParams);
    }
}

void Hud::drawTimersPanelLastLap(const HudExtras &extras,
    const Rectangle &panel)
{
    char last[32];

    formatTime(extras.lastLapTime, last, sizeof(last));
    float blink = 0.65f + 0.35f * std::sin(
        static_cast<float>(Gfx::time()) * 8.0f);
    Color lastColor = Gfx::fade(YELLOW, blink * 0.8f);
    Gfx::drawText("Dernier", static_cast<int>(panel.x + 14.0f),
        static_cast<int>(panel.y + 120.0f), 16, lastColor);
    drawTextRightAligned(last,
        static_cast<int>(panel.x + panel.width - 14.0f),
        static_cast<int>(panel.y + 120.0f), 16, Gfx::fade(YELLOW, blink));
}

void Hud::drawTimersPanel(const RaceState &race, const HudExtras &extras,
    int screenWidth)
{
    bool showLast = extras.lastLapTime > 0.0f && extras.currentLapTime < 3.0f;
    const float panelW = 240.0f;
    float panelH = showLast ? 158.0f : 134.0f;
    Rectangle panel{
        static_cast<float>(screenWidth) - panelW - 16.0f,
        16.0f, panelW, panelH
    };

    drawPanel(panel, 0.45f);
    Gfx::drawText("TEMPS DE COURSE", static_cast<int>(panel.x + 14.0f),
        static_cast<int>(panel.y + 10.0f), 12, Gfx::fade(WHITE, 0.55f));

    char total[32];

    formatTime(race.elapsedTime(), total, sizeof(total));
    Gfx::drawText(total, static_cast<int>(panel.x + 14.0f),
        static_cast<int>(panel.y + 26.0f), 30, RAYWHITE);
    Gfx::drawLineEx(
        Vector2{panel.x + 12.0f, panel.y + 64.0f},
        Vector2{panel.x + panel.width - 12.0f, panel.y + 64.0f},
        1.0f, Gfx::fade(WHITE, 0.15f));

    char cur[32];

    formatLapTime(extras.currentLapTime, cur, sizeof(cur));
    Gfx::drawText("Tour", static_cast<int>(panel.x + 14.0f),
        static_cast<int>(panel.y + 72.0f), 16, Gfx::fade(WHITE, 0.65f));
    drawTextRightAligned(cur, static_cast<int>(panel.x + panel.width - 14.0f),
        static_cast<int>(panel.y + 72.0f), 16, RAYWHITE);

    char best[32];

    formatLapTime(extras.bestLapTime, best, sizeof(best));
    Gfx::drawText("Meilleur", static_cast<int>(panel.x + 14.0f),
        static_cast<int>(panel.y + 96.0f), 16, Gfx::fade(WHITE, 0.65f));
    drawTextRightAligned(best, static_cast<int>(panel.x + panel.width - 14.0f),
        static_cast<int>(panel.y + 96.0f), 16, Color{170, 220, 255, 255});
    if (showLast) {
        drawTimersPanelLastLap(extras, panel);
    }
}

void Hud::drawStartLights(int centerX, int y, int litCount, float alpha)
{
    Rectangle band{
        static_cast<float>(centerX) - 96.0f,
        static_cast<float>(y), 192.0f, 56.0f
    };

    Gfx::drawRectangleRounded(band, 0.5f, 8, Gfx::fade(BLACK, 0.50f * alpha));
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
            Gfx::drawCircleV(pos, 21.0f, Gfx::fade(onColors[i], 0.30f * alpha));
        }
        Color fill = on ? Gfx::fade(onColors[i], alpha)
            : Gfx::fade(WHITE, 0.08f * alpha);
        Gfx::drawCircleV(pos, 14.0f, fill);
        Gfx::drawCircleLinesV(pos, 14.5f, Gfx::fade(WHITE, 0.22f * alpha));
    }
}

void Hud::drawCountdown(const RaceState &race, int screenWidth,
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
    ShadowTextParams shadow{
        screenWidth / 2, textY, fontSize, RAYWHITE, 5
    };

    drawTextShadowCentered(buf, shadow);
}

void Hud::drawGoFlash(const RaceState &race, int screenWidth, int screenHeight)
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
    ShadowTextParams shadow{
        screenWidth / 2, textY, fontSize,
        Gfx::fade(Color{92, 230, 110, 255}, alpha * flash), 5
    };

    drawTextShadowCentered("GO !", shadow);
}

bool Hud::estimateGapSeconds(const RaceState &race, const RacerEntry &racer,
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

void Hud::drawFinishScreenRow(const FinishRowParams &params)
{
    if (params.racer.isPlayer) {
        Rectangle highlight{
            params.panel.x + 16.0f, params.rowY - 4.0f,
            params.panelW - 32.0f, 30.0f - 2.0f
        };
        Gfx::drawRectangleRounded(highlight, 0.4f, 6, Gfx::fade(YELLOW, 0.14f));
    }

    char pos[8];

    std::snprintf(pos, sizeof(pos), "%d", params.rank);
    drawTextRightAligned(pos, static_cast<int>(params.panel.x + 46.0f),
        static_cast<int>(params.rowY), 20,
        params.racer.isPlayer ? YELLOW : Gfx::fade(WHITE, 0.85f));

    Vector2 dot{params.panel.x + 66.0f, params.rowY + 10.0f};

    Gfx::drawCircleV(dot, 6.5f,
        racerColorFor(params.extras, params.idx, params.racer.isPlayer));
    Gfx::drawCircleLinesV(dot, 6.5f, Gfx::fade(WHITE, 0.35f));
    Gfx::drawText(params.racer.name.c_str(),
        static_cast<int>(params.panel.x + 84.0f),
        static_cast<int>(params.rowY), 20,
        params.racer.isPlayer ? YELLOW : RAYWHITE);

    char right[32];

    if (params.racer.finished) {
        formatTime(params.racer.finishTime, right, sizeof(right));
    } else {
        float gap = 0.0f;

        if (estimateGapSeconds(params.race, params.racer, &gap)) {
            std::snprintf(right, sizeof(right), "+%.2fs", gap);
        } else {
            std::snprintf(right, sizeof(right), "en course");
        }
    }
    drawTextRightAligned(right,
        static_cast<int>(params.panel.x + params.panelW - 24.0f),
        static_cast<int>(params.rowY), 20,
        params.racer.finished ? RAYWHITE : Gfx::fade(WHITE, 0.60f));
}

void Hud::drawFinishScreenHeader(const RaceState &race, const Rectangle &panel,
    float panelW)
{
    char ordinal[16];

    formatOrdinal(race.playerPosition(), ordinal, sizeof(ordinal));
    char title[64];

    std::snprintf(title, sizeof(title), "ARRIVEE -- %s place", ordinal);
    ShadowTextParams titleShadow{
        static_cast<int>(panel.x + panelW * 0.5f),
        static_cast<int>(panel.y + 20.0f), 34, YELLOW, 3
    };

    drawTextShadowCentered(title, titleShadow);

    const std::vector<RacerEntry> &racers = race.racers();
    const RacerEntry &player = racers[static_cast<size_t>(race.playerIndex())];
    char timeBuf[32];

    formatTime(player.finished ? player.finishTime : race.elapsedTime(),
        timeBuf, sizeof(timeBuf));
    char timeLine[64];

    std::snprintf(timeLine, sizeof(timeLine), "Temps de course : %s", timeBuf);
    drawTextCentered(timeLine, static_cast<int>(panel.x + panelW * 0.5f),
        static_cast<int>(panel.y + 66.0f), 18, Gfx::fade(WHITE, 0.85f));
    Gfx::drawLineEx(
        Vector2{panel.x + 24.0f, panel.y + 94.0f},
        Vector2{panel.x + panelW - 24.0f, panel.y + 94.0f},
        1.0f, Gfx::fade(WHITE, 0.20f));
}

void Hud::drawFinishScreen(const RaceState &race, const HudExtras &extras,
    int screenWidth, int screenHeight)
{
    Color scrim = Gfx::fade(BLACK, 0.65f);
    Gfx::drawRectangle(0, 0, screenWidth, screenHeight, scrim);

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

    Gfx::drawRectangleRounded(panel, 0.08f, 8, Gfx::fade(BLACK, 0.55f));
    Color panelBorder = Gfx::fade(YELLOW, 0.35f);
    Gfx::drawRectangleRoundedLinesEx(panel, 0.08f, 8, 2.0f, panelBorder);
    drawFinishScreenHeader(race, panel, panelW);

    for (size_t i = 0; i < order.size(); ++i) {
        size_t idx = static_cast<size_t>(order[i]);
        float rowY = panel.y + headerH + rowH * static_cast<float>(i);

        FinishRowParams rowParams{race, racers[idx], idx,
            static_cast<int>(i) + 1, extras, panel, panelW, rowY};

        drawFinishScreenRow(rowParams);
    }
    drawTextCentered("R rejouer      M menu",
        static_cast<int>(panel.x + panelW * 0.5f),
        static_cast<int>(panel.y + panelH - 40.0f), 20, LIGHTGRAY);
}

const Hud::TrackPreview &Hud::getTrackPreview(const TrackDef &def)
{
    static std::vector<TrackPreview> cache;

    for (const TrackPreview &entry : cache) {
        if (entry.key == def.name) {
            return entry;
        }
    }
    TrackPreview entry;

    entry.key = def.name;
    entry.points = Track::make(def).waypoints();
    cache.push_back(std::move(entry));
    return cache.back();
}

void Hud::drawTrackCardPreview(const TrackDef &def,
    const TrackPreview &preview,
    Rectangle inset, bool selected)
{
    if (preview.points.size() < 2) {
        return;
    }
    Rectangle area{
        inset.x + 10.0f, inset.y + 10.0f,
        inset.width - 20.0f, inset.height - 20.0f
    };
    Hud::MapProjection proj = fitTrackInRect(preview.points, area, true);
    float road = std::clamp(def.width * proj.scale, 2.0f, 6.0f);

    drawTrackPolyline(preview.points, proj, road, Gfx::fade(WHITE, 0.14f));
    drawTrackPolyline(preview.points, proj, 1.5f,
        Gfx::fade(WHITE, selected ? 0.60f : 0.40f));
    Gfx::drawCircleV(proj.apply(preview.points[0]), 3.5f,
        Color{88, 214, 104, 255});
}

void Hud::drawTrackCardBadge(const TrackDef &def, Rectangle card)
{
    if (def.surfaceStyle != SurfaceStyle::ABIMEE) {
        return;
    }
    const Color amber{255, 178, 48, 255};
    const char *tag = "ROUTE ABIMEE";
    int tagWidth = Gfx::measureText(tag, 13);
    Rectangle badge{
        card.x + (card.width - static_cast<float>(tagWidth) - 20.0f) * 0.5f,
        card.y + card.height - 40.0f,
        static_cast<float>(tagWidth) + 20.0f, 22.0f
    };

    Gfx::drawRectangleRounded(badge, 0.6f, 6, Gfx::fade(amber, 0.16f));
    Gfx::drawRectangleRoundedLinesEx(badge, 0.6f, 6, 1.0f,
        Gfx::fade(amber, 0.85f));
    Gfx::drawText(tag, static_cast<int>(badge.x + 10.0f),
        static_cast<int>(badge.y + 5.0f), 13, amber);
}

void Hud::drawTrackCard(const TrackDef &def, Rectangle card, bool selected)
{
    Gfx::drawRectangleRounded(card, 0.10f, 8,
        Gfx::fade(BLACK, selected ? 0.55f : 0.42f));
    if (selected) {
        Gfx::drawRectangleRoundedLinesEx(card, 0.10f, 8, 3.0f, YELLOW);
        Rectangle glow{
            card.x - 4.0f, card.y - 4.0f,
            card.width + 8.0f, card.height + 8.0f
        };
        Gfx::drawRectangleRoundedLinesEx(glow, 0.10f, 8, 1.0f,
            Gfx::fade(YELLOW, 0.30f));
    } else {
        Gfx::drawRectangleRoundedLinesEx(card, 0.10f, 8, 1.0f,
            Gfx::fade(WHITE, 0.12f));
    }

    float insetW = card.width - 90.0f;
    float insetH = insetW * 0.70f;
    Rectangle inset{
        card.x + (card.width - insetW) * 0.5f,
        card.y + 16.0f, insetW, insetH
    };

    Gfx::drawRectangleRounded(inset, 0.12f, 6, Gfx::fade(BLACK, 0.40f));
    Gfx::drawRectangleRoundedLinesEx(inset, 0.12f, 6, 1.0f,
        Gfx::fade(WHITE, 0.10f));

    const TrackPreview &preview = getTrackPreview(def);

    drawTrackCardPreview(def, preview, inset, selected);

    float textTop = inset.y + inset.height;

    drawTextCentered(def.name.c_str(),
        static_cast<int>(card.x + card.width * 0.5f),
        static_cast<int>(textTop + 14.0f),
        selected ? 22 : 20, selected ? YELLOW : RAYWHITE);
    WrappedTextParams wrapped{
        static_cast<int>(card.x + 16.0f),
        static_cast<int>(textTop + 46.0f),
        static_cast<int>(card.width - 32.0f),
        15, 19, Gfx::fade(WHITE, 0.62f)
    };

    drawTextWrapped(def.description.c_str(), wrapped);
    drawTrackCardBadge(def, card);
}

void Hud::drawMenuTitle(int screenWidth)
{
    const char *title = "RACER";
    const int titleSize = 78;
    int centerX = screenWidth / 2;
    int titleWidth = Gfx::measureText(title, titleSize);

    Gfx::drawText(title, centerX - titleWidth / 2 + 6, 46 + 6, titleSize,
        Color{110, 45, 16, 255});
    Gfx::drawText(title, centerX - titleWidth / 2, 46, titleSize, ORANGE);
    Gfx::drawLineEx(
        Vector2{static_cast<float>(centerX - titleWidth / 2), 134.0f},
        Vector2{static_cast<float>(centerX + titleWidth / 2), 134.0f},
        3.0f, Gfx::fade(ORANGE, 0.55f));
    drawTextCentered("Choisissez un circuit", centerX, 152, 22, LIGHTGRAY);
}

void Hud::drawMenuCards(const std::vector<TrackDef> &presets, int selectedIndex,
    int screenWidth)
{
    int count = static_cast<int>(presets.size());

    if (count <= 0) {
        return;
    }
    const float gap = 20.0f;
    float cardW = std::min(250.0f,
        (static_cast<float>(screenWidth) - 60.0f -
            gap * static_cast<float>(count - 1)) /
        static_cast<float>(count));
    const float cardH = 280.0f;
    float totalW = cardW * static_cast<float>(count) +
        gap * static_cast<float>(count - 1);
    float x0 = (static_cast<float>(screenWidth) - totalW) * 0.5f;
    const float y0 = 246.0f;

    for (int i = 0; i < count; ++i) {
        Rectangle card{x0 + static_cast<float>(i) * (cardW + gap), y0,
            cardW, cardH};
        bool selected = (i == selectedIndex);

        if (selected) {
            card.x -= 7.0f;
            card.y -= 7.0f;
            card.width += 14.0f;
            card.height += 14.0f;
        }
        drawTrackCard(presets[static_cast<size_t>(i)], card, selected);
    }
}


void Hud::draw(const RaceState &race, int screenWidth, int screenHeight)
{
    drawHudEx(race, screenWidth, screenHeight, HudExtras{});
}

void Hud::drawHudEx(const RaceState &race, int screenWidth, int screenHeight,
    const HudExtras &extras)
{
    const std::vector<RacerEntry> &racers = race.racers();
    const RacerEntry &player = racers[static_cast<size_t>(race.playerIndex())];

    drawStandingsPanel(race, extras);
    drawTimersPanel(race, extras, screenWidth);
    drawSpeedGauge(player.car, screenHeight);
    drawMinimap(race, extras, screenWidth, screenHeight);

    switch (race.phase()) {
        case RacePhase::COUNTDOWN:
            drawCountdown(race, screenWidth, screenHeight);
            break;
        case RacePhase::RACING:
            drawGoFlash(race, screenWidth, screenHeight);
            break;
        case RacePhase::FINISHED:
            drawFinishScreen(race, extras, screenWidth, screenHeight);
            break;
    }
}

void Hud::drawMenu(const std::vector<TrackDef> &presets, int selectedIndex,
    int screenWidth, int screenHeight)
{
    Gfx::clearBackground(Color{15, 17, 26, 255});
    Gfx::drawRectangleGradientV(0, 0, screenWidth, screenHeight,
        Color{22, 26, 40, 255}, Color{10, 11, 18, 255});

    int centerX = screenWidth / 2;

    drawMenuTitle(screenWidth);
    drawMenuCards(presets, selectedIndex, screenWidth);
    drawTextCentered("Haut/Bas : choisir   --   Entree : demarrer", centerX,
        screenHeight - 56, 20, GRAY);
}

}
