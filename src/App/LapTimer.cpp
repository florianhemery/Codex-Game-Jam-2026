/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Lap timing state and updates
*/

#include <algorithm>

#include "App/LapTimer.hpp"

namespace racer {
namespace app {

void updateLapTimer(
    LapTimerState &timer, const RacerEntry &player, float dt,
    RacePhase phase)
{
    if (phase != RacePhase::RACING)
        return;
    timer.currentLapTime += dt;
    if (player.lap > timer.lastLapCount) {
        timer.lastLapTime = timer.currentLapTime;
        timer.currentLapTime = 0.0f;
        if (timer.bestLapTime <= 0.0f
            || timer.lastLapTime < timer.bestLapTime)
            timer.bestLapTime = timer.lastLapTime;
        timer.lastLapFlash = 3.0f;
        timer.lastLapCount = player.lap;
    }
    timer.lastLapFlash = std::max(0.0f, timer.lastLapFlash - dt);
}

} // namespace app
} // namespace racer
