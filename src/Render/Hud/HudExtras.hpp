/*
** EPITECH PROJECT, 2026
** racer
** File description:
** HUD optional timing and color extras
*/

#ifndef HUD_EXTRAS_HPP_
#define HUD_EXTRAS_HPP_

#include <vector>

#include "raylib.h"

namespace racer {

struct HudExtras {
    float currentLapTime = 0.0f;
    float lastLapTime = 0.0f;
    float bestLapTime = 0.0f;
    std::vector<Color> racerColors;
};

} // namespace racer

#endif /* !HUD_EXTRAS_HPP_ */
