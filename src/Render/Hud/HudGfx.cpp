/*
** EPITECH PROJECT, 2026
** racer
** File description:
** HUD raylib drawing helpers
*/

#include "Render/Hud/HudGfx.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace racer {

namespace {

bool g_colorblindMode = false;

} // namespace

bool HudGfx::colorblindMode()
{
    return g_colorblindMode;
}

void HudGfx::setColorblindMode(bool enabled)
{
    g_colorblindMode = enabled;
}

void HudGfx::toggleColorblindMode()
{
    g_colorblindMode = !g_colorblindMode;
}

int HudGfx::measureText(const char *text, int fontSize)
{
    return MeasureText(text, fontSize);
}

float HudGfx::time()
{
    return static_cast<float>(GetTime());
}

Color HudGfx::fade(Color color, float alpha)
{
    return Fade(color, alpha);
}

void HudGfx::drawText(const char *text, int x, int y, int fontSize,
    Color color)
{
    DrawText(text, x, y, fontSize, color);
}

void HudGfx::drawRectangleRounded(Rectangle rect, float roundness,
    int segments, Color color)
{
    DrawRectangleRounded(rect, roundness, segments, color);
}

void HudGfx::drawRectangleRoundedLinesEx(Rectangle rect, float roundness,
    int segments, float lineThick, Color color)
{
    DrawRectangleRoundedLinesEx(rect, roundness, segments, lineThick, color);
}

void HudGfx::drawLineEx(Vector2 start, Vector2 end, float thick, Color color)
{
    DrawLineEx(start, end, thick, color);
}

void HudGfx::drawCircleV(Vector2 center, float radius, Color color)
{
    DrawCircleV(center, radius, color);
}

void HudGfx::drawCircleLinesV(Vector2 center, float radius, Color color)
{
    DrawCircleLinesV(center, radius, color);
}

void HudGfx::drawRing(Vector2 center, float innerR, float outerR,
    float startAngle, float endAngle, int segments, Color color)
{
    DrawRing(center, innerR, outerR, startAngle, endAngle, segments, color);
}

void HudGfx::drawRectangle(int x, int y, int w, int h, Color color)
{
    DrawRectangle(x, y, w, h, color);
}

void HudGfx::clearBackground(Color color)
{
    ClearBackground(color);
}

void HudGfx::drawRectangleGradientV(int x, int y, int w, int h,
    Color color1, Color color2)
{
    DrawRectangleGradientV(x, y, w, h, color1, color2);
}


void HudGfx::formatOrdinal(int position, char *buf, size_t bufSize)
{
    if (position == 1) {
        std::snprintf(buf, bufSize, "1re");
    } else {
        std::snprintf(buf, bufSize, "%de", position);
    }
}

void HudGfx::formatTime(float seconds, char *buf, size_t bufSize)
{
    int minutes = static_cast<int>(seconds) / 60;
    float secs = seconds - static_cast<float>(minutes * 60);

    std::snprintf(buf, bufSize, "%d:%05.2f", minutes, secs);
}

void HudGfx::formatLapTime(float seconds, char *buf, size_t bufSize)
{
    if (seconds <= 0.0f) {
        std::snprintf(buf, bufSize, "--");
        return;
    }
    formatTime(seconds, buf, bufSize);
}

Color HudGfx::lerpColor(Color a, Color b, float t)
{
    t = std::clamp(t, 0.0f, 1.0f);
    auto mix = [t](unsigned char x, unsigned char y) {
        float delta = static_cast<float>(y) - static_cast<float>(x);

        return static_cast<unsigned char>(
            static_cast<float>(x) + delta * t);
    };

    return Color{mix(a.r, b.r), mix(a.g, b.g), mix(a.b, b.b), mix(a.a, b.a)};
}

Color HudGfx::racerColorFor(const HudExtras &extras, size_t index, bool isPlayer)
{
    // Per-race extras.racerColors (see App::colorForRacerIndex) already
    // honors the colorblind toggle, so prefer it when present; the palettes
    // below only cover callers (demos, tests) that never populated extras.
    if (index < extras.racerColors.size()) {
        return extras.racerColors[index];
    }
    if (g_colorblindMode) {
        // Okabe-Ito colorblind-safe palette: vermillion for the player (high
        // contrast, distinguishable from the rest under all common CVD
        // types), then blue / bluish-green / reddish-purple / sky-blue /
        // yellow for opponents instead of the default red/blue/green/
        // orange/purple set which collapses under protanopia/deuteranopia.
        if (isPlayer) {
            return Color{213, 94, 0, 255};
        }
        constexpr Color kColorblindFallback[] = {
            Color{0, 114, 178, 255}, Color{0, 158, 115, 255},
            Color{204, 121, 167, 255}, Color{86, 180, 233, 255},
            Color{240, 228, 66, 255}
        };
        return kColorblindFallback[index
            % (sizeof(kColorblindFallback) / sizeof(kColorblindFallback[0]))];
    }
    if (isPlayer) {
        return RED;
    }
    constexpr Color kFallback[] = {
        BLUE, DARKGREEN, ORANGE, PURPLE, SKYBLUE, MAROON
    };

    return kFallback[index % (sizeof(kFallback) / sizeof(kFallback[0]))];
}

void HudGfx::drawPanel(Rectangle rect, float alpha)
{
    HudGfx::drawRectangleRounded(rect, 0.12f, 8, HudGfx::fade(BLACK, alpha));
    HudGfx::drawRectangleRoundedLinesEx(rect, 0.12f, 8, 1.0f,
        HudGfx::fade(WHITE, 0.10f));
}

void HudGfx::drawTextCentered(const char *text, int centerX, int y, int fontSize,
    Color color)
{
    int width = HudGfx::measureText(text, fontSize);

    HudGfx::drawText(text, centerX - width / 2, y, fontSize, color);
}

void HudGfx::drawTextRightAligned(const char *text, int rightX, int y,
    int fontSize, Color color)
{
    int width = HudGfx::measureText(text, fontSize);

    HudGfx::drawText(text, rightX - width, y, fontSize, color);
}

void HudGfx::drawTextShadowCentered(const char *text,
    const HudShadowTextParams &params)
{
    int width = HudGfx::measureText(text, params.fontSize);

    HudGfx::drawText(text, params.centerX - width / 2 + params.offset,
        params.y + params.offset, params.fontSize, HudGfx::fade(BLACK, 0.55f));
    HudGfx::drawText(text, params.centerX - width / 2, params.y, params.fontSize,
        params.color);
}

void HudGfx::drawTextWrapped(const char *text, const HudWrappedTextParams &params)
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

        if (lineLen > 0 && HudGfx::measureText(candidate, params.fontSize) >
            params.maxWidth) {
            HudGfx::drawText(line, params.x, params.y + lines * params.lineHeight,
                params.fontSize, params.color);
            ++lines;
            lineLen = std::snprintf(line, sizeof(line), "%s", word);
        } else {
            lineLen = std::snprintf(line, sizeof(line), "%s", candidate);
        }
    }
    if (lineLen > 0) {
        HudGfx::drawText(line, params.x, params.y + lines * params.lineHeight,
            params.fontSize, params.color);
    }
}

} // namespace racer
