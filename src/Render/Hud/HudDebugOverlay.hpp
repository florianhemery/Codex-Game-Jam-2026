/*
** EPITECH PROJECT, 2026
** racer
** File description:
** HudDebugOverlay — opt-in FPS/frame-time debug overlay (F3). Purely
** additive: draws a rolling-average/worst-case frame time readout plus a
** per-section breakdown sourced from WorldRenderTimings. Does not affect
** gameplay or normal rendering when hidden.
*/

#ifndef HUD_DEBUG_OVERLAY_HPP_
#define HUD_DEBUG_OVERLAY_HPP_

#include <array>
#include <cstddef>

namespace racer::hud {

class HudDebugOverlay {
public:
    static HudDebugOverlay &instance();

    // Call once per frame, regardless of visibility, to feed the rolling
    // frame-time history (cheap: one GetFrameTime() read + ring buffer
    // write). Also handles the F3 toggle key.
    void update();

    // Call once per frame from the draw sequence (after HUD draw is a
    // reasonable spot); no-op when hidden.
    void draw(int screenWidth, int screenHeight) const;

    bool visible() const { return visible_; }

private:
    HudDebugOverlay() = default;

    static constexpr int kHistorySize = 120;

    bool visible_ = false;
    std::array<float, kHistorySize> frameTimesMs_{};
    size_t writeIndex_ = 0;
    size_t sampleCount_ = 0;
};

} // namespace racer::hud

#endif /* !HUD_DEBUG_OVERLAY_HPP_ */
