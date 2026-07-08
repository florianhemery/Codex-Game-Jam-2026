/*
** EPITECH PROJECT, 2026
** racer
** File description:
** HUD layout parameter records
*/

#ifndef HUD_TYPES_HPP_
#define HUD_TYPES_HPP_

#include <cstddef>
#include <string>
#include <vector>

#include "raylib.h"

#include "Race/RaceState.hpp"
#include "Render/Hud/HudExtras.hpp"

namespace racer {

struct HudShadowTextParams {
    int centerX;
    int y;
    int fontSize;
    Color color;
    int offset;
};

struct HudWrappedTextParams {
    int x;
    int y;
    int maxWidth;
    int fontSize;
    int lineHeight;
    Color color;
};

struct HudGaugeArcParams {
    Vector2 center;
    float ratio;
    float kmhMax;
    float angleMin;
    float angleSpan;
    float rIn;
    float rOut;
};

struct HudStandingsRowParams {
    const RacerEntry &racer;
    size_t idx;
    int rank;
    const HudExtras &extras;
    const Rectangle &panel;
    float rowY;
    // Current rubber-band multiplier for this racer (1.0 = neutral). Lets the
    // standings row show a subtle "catching up" / "pulling away" cue.
    float rubberBand = 1.0f;
};

struct HudFinishRowParams {
    const RaceState &race;
    const RacerEntry &racer;
    size_t idx;
    int rank;
    const HudExtras &extras;
    const Rectangle &panel;
    float panelW;
    float rowY;
    // Winner's finish time (0 if nobody has finished yet), used to show a
    // "+x.xxs" gap-to-leader instead of a bare time for the rest of the pack.
    float leaderFinishTime = 0.0f;
};

struct HudMapProjection {
    Vector2 screenCenter{};
    Vector2 worldCenter{};
    float scale = 1.0f;
    bool rotated = false;

    Vector2 apply(Vector2 world) const;
};

struct HudTrackPreview {
    std::string key;
    std::vector<Vector2> points;
};

} // namespace racer

#endif /* !HUD_TYPES_HPP_ */
