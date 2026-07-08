/*
** EPITECH PROJECT, 2026
** racer
** File description:
** HudDebugOverlay implementation
*/

#include "Render/Hud/HudDebugOverlay.hpp"

#include <algorithm>

#include "raylib.h"

#include "Render/World/WorldRenderTimings.hpp"

namespace racer::hud {

HudDebugOverlay &HudDebugOverlay::instance()
{
    static HudDebugOverlay overlay;
    return overlay;
}

void HudDebugOverlay::update()
{
    if (IsKeyPressed(KEY_F3)) {
        visible_ = !visible_;
    }

    frameTimesMs_[writeIndex_] = GetFrameTime() * 1000.0f;
    writeIndex_ = (writeIndex_ + 1) % kHistorySize;
    sampleCount_ = std::min(sampleCount_ + 1, static_cast<size_t>(kHistorySize));
}

void HudDebugOverlay::draw(int screenWidth, int screenHeight) const
{
    if (!visible_ || sampleCount_ == 0) {
        return;
    }

    float sum = 0.0f;
    float worst = 0.0f;
    for (size_t i = 0; i < sampleCount_; ++i) {
        float v = frameTimesMs_[i];
        sum += v;
        worst = std::max(worst, v);
    }
    float avgMs = sum / static_cast<float>(sampleCount_);
    float avgFps = avgMs > 0.0f ? 1000.0f / avgMs : 0.0f;

    const world::WorldRenderTimings &t = world::worldRenderTimings();

    constexpr int kPanelW = 300;
    constexpr int kPanelH = 150;
    int x = screenWidth - kPanelW - 12;
    int y = 12;
    (void)screenHeight;

    DrawRectangle(x, y, kPanelW, kPanelH, Fade(BLACK, 0.55f));
    DrawRectangleLines(x, y, kPanelW, kPanelH, Fade(RAYWHITE, 0.6f));

    int lineY = y + 8;
    const int lh = 16;
    DrawText(TextFormat("FPS now: %d  (avg %.1f)", GetFPS(), avgFps),
        x + 10, lineY, 14, RAYWHITE);
    lineY += lh;
    DrawText(TextFormat("Frame: %.2f ms avg / %.2f ms worst (%d)",
        avgMs, worst, static_cast<int>(sampleCount_)),
        x + 10, lineY, 14, RAYWHITE);
    lineY += lh + 4;
    DrawText("Section breakdown (last frame):", x + 10, lineY, 14,
        Fade(RAYWHITE, 0.85f));
    lineY += lh;
    DrawText(TextFormat("  terrain:  %.2f ms", t.terrainMs), x + 10, lineY,
        14, RAYWHITE);
    lineY += lh;
    DrawText(TextFormat("  props:    %.2f ms", t.propsMs), x + 10, lineY,
        14, RAYWHITE);
    lineY += lh;
    DrawText(TextFormat("  triggers: %.2f ms", t.triggersMs), x + 10, lineY,
        14, RAYWHITE);
    lineY += lh;
    DrawText(TextFormat("  traffic:  %.2f ms", t.trafficMs), x + 10, lineY,
        14, RAYWHITE);
    lineY += lh + 2;
    DrawText("F3: toggle", x + 10, lineY, 12, Fade(RAYWHITE, 0.6f));
}

} // namespace racer::hud
