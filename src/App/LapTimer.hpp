/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Lap timing state and updates
*/

#ifndef LAP_TIMER_HPP_
#define LAP_TIMER_HPP_

#include <functional>

#include "Race/RaceState.hpp"

namespace racer {
namespace app {

struct LapTimerState {
    float currentLapTime = 0.0f;
    float lastLapTime = 0.0f;
    float bestLapTime = 0.0f;
    int lastLapCount = 0;
    float lastLapFlash = 0.0f;
    float lapBannerTimer = 0.0f;
    int lapBannerLap = 0;
    bool lapBannerIsFinal = false;
};

// Fired the frame a lap completes, with that lap's time in seconds.
using LapCompleteCallback = std::function<void(float lapTimeSeconds)>;
// Fired the frame the player finishes the race, with the total race time
// in seconds (RacerEntry::finishTime).
using RaceFinishCallback = std::function<void(float totalTimeSeconds)>;

// Updates lap-timing state. If provided, onLapComplete/onRaceFinish let
// callers (e.g. a leaderboard recorder) observe those events without this
// module depending on them -- kept optional/decoupled so callers that
// don't care about persistence (tests, demos) can omit them.
void updateLapTimer(
    LapTimerState &timer, const RacerEntry &player, float dt,
    RacePhase phase, int lapsToWin,
    const LapCompleteCallback &onLapComplete = nullptr,
    const RaceFinishCallback &onRaceFinish = nullptr);

} // namespace app
} // namespace racer

#endif
