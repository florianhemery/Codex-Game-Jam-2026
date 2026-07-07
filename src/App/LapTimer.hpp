/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Lap timing state and updates
*/

#ifndef LAP_TIMER_HPP_
#define LAP_TIMER_HPP_

#include "Race/RaceState.hpp"

namespace racer {
namespace app {

struct LapTimerState {
    float currentLapTime = 0.0f;
    float lastLapTime = 0.0f;
    float bestLapTime = 0.0f;
    int lastLapCount = 0;
    float lastLapFlash = 0.0f;
};

void updateLapTimer(
    LapTimerState &timer, const RacerEntry &player, float dt,
    RacePhase phase);

} // namespace app
} // namespace racer

#endif
