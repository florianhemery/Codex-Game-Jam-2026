/*
** EPITECH PROJECT, 2026
** racer
** File description:
** HUD speed gauge drawing
*/

#include <cmath>

#include "Render/Hud/HudRaceOverlay.hpp"
#include "Render/Hud/HudGfx.hpp"

namespace racer {

void HudRaceOverlay::drawSpeedGaugeArc(const HudGaugeArcParams &params)
{
    HudGfx::drawRing(params.center, params.rIn, params.rOut, params.angleMin,
        params.angleMin + params.angleSpan, 64, HudGfx::fade(WHITE, 0.08f));
    float redlineStart = params.angleMin + params.angleSpan *
        (200.0f / params.kmhMax);

    HudGfx::drawRing(params.center, params.rIn, params.rOut, redlineStart,
        params.angleMin + params.angleSpan, 16, HudGfx::fade(RED, 0.20f));
    if (params.ratio > 0.004f) {
        Color fill = HudGfx::lerpColor(
            Color{255, 168, 40, 255},
            Color{255, 66, 40, 255},
            params.ratio);
        HudGfx::drawRing(params.center, params.rIn + 1.0f, params.rOut - 1.0f,
            params.angleMin, params.angleMin + params.angleSpan * params.ratio,
            64, fill);
    }
}

void HudRaceOverlay::drawSpeedGaugeTicks(const Vector2 &center, float kmhMax,
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

        HudGfx::drawLineEx(
            Vector2{center.x + dir.x * r1, center.y + dir.y * r1},
            Vector2{center.x + dir.x * r2, center.y + dir.y * r2},
            major ? 2.0f : 1.0f,
            HudGfx::fade(WHITE, major ? 0.70f : 0.32f));
        if (major) {
            char label[8];

            std::snprintf(label, sizeof(label), "%d", speed);
            float labelRadius = rOut + 20.0f;
            HudGfx::drawTextCentered(label,
                static_cast<int>(center.x + dir.x * labelRadius),
                static_cast<int>(center.y + dir.y * labelRadius) - 5,
                10, HudGfx::fade(WHITE, 0.55f));
        }
    }
}

void HudRaceOverlay::drawSpeedGaugeNeedle(const Vector2 &center, float ratio,
    float angleMin,
    float angleSpan, float rIn)
{
    float angle = (angleMin + angleSpan * ratio) * DEG2RAD;
    Vector2 dir{std::cos(angle), std::sin(angle)};
    Vector2 tip{
        center.x + dir.x * (rIn - 3.0f),
        center.y + dir.y * (rIn - 3.0f)
    };

    HudGfx::drawLineEx(
        Vector2{center.x + dir.x * 30.0f, center.y + dir.y * 30.0f},
        tip, 3.0f, Color{255, 92, 70, 255});
    HudGfx::drawCircleV(tip, 2.5f, RAYWHITE);
}

void HudRaceOverlay::drawNitroBar(const Car &car, const HudExtras &extras,
    const Rectangle &panel, float panelW, float panelH)
{
    float nitroRatio = std::clamp(
        car.nitroRemaining() / car.tuning().nitroCapacity, 0.0f, 1.0f);
    float barY = panel.y + panelH - 30.0f;
    Rectangle barBg{panel.x + 86.0f, barY, panelW - 86.0f - 18.0f, 13.0f};
    const char *nitroLabel = extras.nitroActive ? "NITRO [SHIFT]" : "NITRO";

    HudGfx::drawText(nitroLabel, static_cast<int>(panel.x + 20.0f),
        static_cast<int>(barY), 13,
        extras.nitroActive ? Color{255, 220, 120, 255}
            : HudGfx::fade(ORANGE, 0.95f));
    HudGfx::drawRectangleRounded(barBg, 0.7f, 6, HudGfx::fade(WHITE, 0.10f));
    if (nitroRatio <= 0.01f) {
        return;
    }
    Color col = ORANGE;

    if (extras.nitroActive) {
        float pulse = 0.55f + 0.45f * std::sin(
            static_cast<float>(HudGfx::time()) * 10.0f);
        col = HudGfx::lerpColor(ORANGE, Color{255, 250, 180, 255}, pulse);
    } else if (nitroRatio >= 0.999f) {
        float pulse = 0.5f + 0.5f * std::sin(
            static_cast<float>(HudGfx::time()) * 6.0f);
        col = HudGfx::lerpColor(ORANGE, Color{255, 236, 120, 255}, pulse * 0.65f);
    }
    Rectangle fill{barBg.x, barBg.y, barBg.width * nitroRatio, barBg.height};

    HudGfx::drawRectangleRounded(fill, 0.7f, 6, col);
    if (extras.nitroActive) {
        HudGfx::drawRectangleRoundedLinesEx(
            panel, 0.12f, 8, 2.0f, HudGfx::fade(ORANGE, 0.55f));
    }
}

void HudRaceOverlay::drawSpeedGauge(const Car &car, int screenHeight,
    const HudExtras &extras)
{
    const float panelW = 272.0f;
    const float panelH = 232.0f;
    Rectangle panel{
        16.0f,
        static_cast<float>(screenHeight) - panelH - 16.0f,
        panelW, panelH
    };

    HudGfx::drawPanel(panel, 0.50f);

    const Vector2 center{panel.x + panelW * 0.5f, panel.y + 108.0f};
    const float rOut = 82.0f;
    const float rIn = 70.0f;
    const float angleMin = 150.0f;
    const float angleSpan = 240.0f;
    const float kmhMax = 230.0f;
    float kmh = std::fabs(car.speed()) * 6.0f;
    float ratio = std::clamp(kmh / kmhMax, 0.0f, 1.0f);

    HudGaugeArcParams arc{center, ratio, kmhMax, angleMin, angleSpan, rIn, rOut};

    drawSpeedGaugeArc(arc);
    drawSpeedGaugeTicks(center, kmhMax, angleMin, angleSpan, rOut);
    drawSpeedGaugeNeedle(center, ratio, angleMin, angleSpan, rIn);

    char speedBuf[16];

    std::snprintf(speedBuf, sizeof(speedBuf), "%.0f", kmh);
    HudGfx::drawTextCentered(speedBuf, static_cast<int>(center.x),
        static_cast<int>(center.y) - 16, 40, RAYWHITE);
    HudGfx::drawTextCentered("km/h", static_cast<int>(center.x),
        static_cast<int>(center.y) + 28, 14, HudGfx::fade(WHITE, 0.60f));
    drawNitroBar(car, extras, panel, panelW, panelH);
}


} // namespace racer
