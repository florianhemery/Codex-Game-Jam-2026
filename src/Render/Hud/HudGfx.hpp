/*
** EPITECH PROJECT, 2026
** racer
** File description:
** HUD raylib drawing helpers
*/

#ifndef HUD_GFX_HPP_
#define HUD_GFX_HPP_

#include "raylib.h"

#include "Render/Hud/HudExtras.hpp"
#include "Render/Hud/HudTypes.hpp"

namespace racer {

struct HudGfx {
    static int measureText(const char *text, int fontSize);
    static float time();
    static Color fade(Color color, float alpha);
    static void drawText(const char *text, int x, int y, int fontSize,
        Color color);
    static void drawRectangleRounded(Rectangle rect, float roundness,
        int segments, Color color);
    static void drawRectangleRoundedLinesEx(Rectangle rect, float roundness,
        int segments, float lineThick, Color color);
    static void drawLineEx(Vector2 start, Vector2 end, float thick,
        Color color);
    static void drawCircleV(Vector2 center, float radius, Color color);
    static void drawCircleLinesV(Vector2 center, float radius, Color color);
    static void drawRing(Vector2 center, float innerR, float outerR,
        float startAngle, float endAngle, int segments, Color color);
    static void drawRectangle(int x, int y, int w, int h, Color color);
    static void clearBackground(Color color);
    static void drawRectangleGradientV(int x, int y, int w, int h,
        Color color1, Color color2);

    static void formatOrdinal(int position, char *buf, size_t bufSize);
    static void formatTime(float seconds, char *buf, size_t bufSize);
    static void formatLapTime(float seconds, char *buf, size_t bufSize);
    static Color lerpColor(Color a, Color b, float t);
    static Color racerColorFor(const HudExtras &extras, size_t index,
        bool isPlayer);
    static void drawPanel(Rectangle rect, float alpha);
    static void drawTextCentered(const char *text, int centerX, int y,
        int fontSize, Color color);
    static void drawTextRightAligned(const char *text, int rightX, int y,
        int fontSize, Color color);
    static void drawTextShadowCentered(const char *text,
        const HudShadowTextParams &params);
    static void drawTextWrapped(const char *text,
        const HudWrappedTextParams &params);
};

} // namespace racer

#endif /* !HUD_GFX_HPP_ */
