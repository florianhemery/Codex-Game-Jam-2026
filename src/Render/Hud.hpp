/*
** EPITECH PROJECT, 2026
** racer
** File description:
** HUD rendering for race and menu screens
*/

#ifndef HUD_HPP_
#define HUD_HPP_

#include <cstddef>
#include <string>
#include <vector>

#include "raylib.h"

#include "Race/RaceState.hpp"
#include "Track/TrackDef.hpp"

namespace racer {

struct HudExtras {
    float currentLapTime = 0.0f;
    float lastLapTime = 0.0f;
    float bestLapTime = 0.0f;
    std::vector<Color> racerColors;
};

class Hud {
public:
    void draw(const RaceState &race, int screenWidth, int screenHeight);
    void drawHudEx(const RaceState &race, int screenWidth, int screenHeight,
        const HudExtras &extras);
    void drawMenu(const std::vector<TrackDef> &presets, int selectedIndex,
        int screenWidth, int screenHeight);

private:
    struct Gfx {
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
    };

    struct ShadowTextParams {
        int centerX;
        int y;
        int fontSize;
        Color color;
        int offset;
    };

    struct WrappedTextParams {
        int x;
        int y;
        int maxWidth;
        int fontSize;
        int lineHeight;
        Color color;
    };

    struct GaugeArcParams {
        Vector2 center;
        float ratio;
        float kmhMax;
        float angleMin;
        float angleSpan;
        float rIn;
        float rOut;
    };

    struct StandingsRowParams {
        const RacerEntry &racer;
        size_t idx;
        int rank;
        const HudExtras &extras;
        const Rectangle &panel;
        float rowY;
    };

    struct FinishRowParams {
        const RaceState &race;
        const RacerEntry &racer;
        size_t idx;
        int rank;
        const HudExtras &extras;
        const Rectangle &panel;
        float panelW;
        float rowY;
    };

    struct MapProjection {
        Vector2 screenCenter{};
        Vector2 worldCenter{};
        float scale = 1.0f;
        bool rotated = false;

        Vector2 apply(Vector2 world) const;
    };

    struct TrackPreview {
        std::string key;
        std::vector<Vector2> points;
    };

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
        const ShadowTextParams &params);
    static void drawTextWrapped(const char *text,
        const WrappedTextParams &params);
    static MapProjection fitTrackInRect(const std::vector<Vector2> &points,
        Rectangle area, bool allowRotate);
    static void drawTrackPolyline(const std::vector<Vector2> &points,
        const MapProjection &proj, float thickness, Color color);
    static void drawFinishLineTick(const std::vector<Vector2> &points,
        const MapProjection &proj, float halfLength, Color color);
    static void drawMinimapOpponents(const RaceState &race,
        const HudExtras &extras, const MapProjection &proj,
        const Rectangle &panel);
    static void drawMinimapPlayer(const RaceState &race,
        const HudExtras &extras, const MapProjection &proj,
        const Rectangle &panel);
    static void drawMinimap(const RaceState &race, const HudExtras &extras,
        int screenWidth, int screenHeight);
    static void drawSpeedGaugeArc(const GaugeArcParams &params);
    static void drawSpeedGaugeTicks(const Vector2 &center, float kmhMax,
        float angleMin, float angleSpan, float rOut);
    static void drawSpeedGaugeNeedle(const Vector2 &center, float ratio,
        float angleMin, float angleSpan, float rIn);
    static void drawNitroBar(const Car &car, const Rectangle &panel,
        float panelW, float panelH);
    static void drawSpeedGauge(const Car &car, int screenHeight);
    static void drawStandingsFinishFlag(int x, int y);
    static void drawStandingsRow(const StandingsRowParams &params);
    static void drawStandingsPanel(const RaceState &race,
        const HudExtras &extras);
    static void drawTimersPanelLastLap(const HudExtras &extras,
        const Rectangle &panel);
    static void drawTimersPanel(const RaceState &race,
        const HudExtras &extras, int screenWidth);
    static void drawStartLights(int centerX, int y, int litCount, float alpha);
    static void drawCountdown(const RaceState &race, int screenWidth,
        int screenHeight);
    static void drawGoFlash(const RaceState &race, int screenWidth,
        int screenHeight);
    static bool estimateGapSeconds(const RaceState &race,
        const RacerEntry &racer, float *outSeconds);
    static void drawFinishScreenRow(const FinishRowParams &params);
    static void drawFinishScreenHeader(const RaceState &race,
        const Rectangle &panel, float panelW);
    static void drawFinishScreen(const RaceState &race,
        const HudExtras &extras, int screenWidth, int screenHeight);
    static const TrackPreview &getTrackPreview(const TrackDef &def);
    static void drawTrackCardPreview(const TrackDef &def,
        const TrackPreview &preview, Rectangle inset, bool selected);
    static void drawTrackCardBadge(const TrackDef &def, Rectangle card);
    static void drawTrackCard(const TrackDef &def, Rectangle card,
        bool selected);
    static void drawMenuTitle(int screenWidth);
    static void drawMenuCards(const std::vector<TrackDef> &presets,
        int selectedIndex, int screenWidth);
};

} // namespace racer

#endif /* !HUD_HPP_ */
