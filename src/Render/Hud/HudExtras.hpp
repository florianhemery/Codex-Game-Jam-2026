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
    float lastLapFlash = 0.0f;
    std::vector<Color> racerColors;

    bool showControlsBanner = false;
    bool nitroActive = false;
    bool drifting = false;
    bool offroad = false;
    float lapBannerTimer = 0.0f;
    int lapBannerLap = 0;
    bool lapBannerIsFinal = false;
    int driftCount = 0;
    bool startBoostActive = false;
    float startBoostRemaining = 0.0f;
};

} // namespace racer

#endif /* !HUD_EXTRAS_HPP_ */
