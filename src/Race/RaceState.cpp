/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Race state machine (countdown, laps, standings, collisions)
*/

#include "Race/RaceState.hpp"

#include "Race/RaceContactResolver.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace racer {

RaceState::RaceState(Track track, int lapsToWin, int aiCount)
    : track_(std::move(track))
    , lapsToWin_(lapsToWin)
{
    int totalCars = aiCount + 1;

    initPlayer(totalCars);
    for (int i = 0; i < aiCount; ++i) {
        initAiRacer(i, totalCars);
    }
}

void RaceState::initPlayer(int totalCars)
{
    RacerEntry player;

    player.name = "Joueur";
    player.isPlayer = true;
    player.car.position() = track_.startPosition(0, totalCars);
    player.car.heading() = track_.startHeading();
    player.car.velocityHeading() = player.car.heading();
    Track::Progress prog = track_.projectPosition(player.car.position());
    player.lastSegment = prog.segmentIndex;
    racers_.push_back(player);
    playerIndex_ = 0;
}

void RaceState::initAiRacer(int aiIndex, int totalCars)
{
    RacerEntry ai;

    ai.name = "IA " + std::to_string(aiIndex + 1);
    ai.car.position() = track_.startPosition(aiIndex + 1, totalCars);
    ai.car.heading() = track_.startHeading();
    ai.car.velocityHeading() = ai.car.heading();
    Track::Progress prog = track_.projectPosition(ai.car.position());
    ai.lastSegment = prog.segmentIndex;
    racers_.push_back(ai);

    float skill = 1.0f - 0.05f * static_cast<float>(aiIndex);
    ai.car.tuning().maxSpeed *= 0.90f + 0.10f * skill;
    ai.car.tuning().acceleration *= 0.85f + 0.15f * skill;
    unsigned int seed = static_cast<unsigned int>(1000 + aiIndex * 7919);
    aiDrivers_.emplace_back(skill, seed);
}

void RaceState::update(float dt, const CarInput& playerInput)
{
    if (phase_ == RacePhase::COUNTDOWN) {
        updateCountdown(dt);
        return;
    }
    if (phase_ == RacePhase::FINISHED) {
        return;
    }
    updateRacers(dt, playerInput);
}

void RaceState::updateCountdown(float dt)
{
    countdownRemaining_ -= dt;
    if (countdownRemaining_ <= 0.0f) {
        countdownRemaining_ = 0.0f;
        phase_ = RacePhase::RACING;
    }
}

void RaceState::updateRacers(float dt, const CarInput& playerInput)
{
    elapsedTime_ += dt;
    int numSegments = static_cast<int>(track_.waypoints().size());

    for (size_t i = 0; i < racers_.size(); ++i) {
        updateSingleRacer(i, dt, playerInput, numSegments);
    }
    RaceContactResolver::resolveAll(racers_);
}

void RaceState::finalizePlayerIfDone(const RacerEntry& racer)
{
    if (racer.isPlayer && racer.finished) {
        phase_ = RacePhase::FINISHED;
    }
}

void RaceState::updateSingleRacer(
    size_t index, float dt, const CarInput& playerInput, int numSegments)
{
    RacerEntry& racer = racers_[index];
    if (racer.finished) {
        return;
    }
    CarInput input = racer.isPlayer
        ? playerInput
        : aiDrivers_[index - 1].computeInput(racer.car, track_);
    racer.lastInput = input;
    racer.car.update(input, dt);
    Track::Progress prog = track_.projectPosition(racer.car.position());
    applySurfaceGrip(racer, prog);
    updateMidpointFlag(racer, prog, numSegments);
    updateLapCount(racer, prog, numSegments);
    racer.lastSegment = prog.segmentIndex;
    finalizePlayerIfDone(racer);
}

void RaceState::applySurfaceGrip(
    RacerEntry& racer, const Track::Progress& prog)
{
    float grassLimit = track_.width() * 0.5f + 0.6f;

    if (std::fabs(prog.lateralOffset) > grassLimit) {
        racer.car.surfaceGrip() = 0.55f;
        racer.car.surfaceDrag() = 3.0f;
    } else if (track_.style() == SurfaceStyle::ABIMEE) {
        racer.car.surfaceGrip() = 0.85f;
        racer.car.surfaceDrag() = 1.15f;
    } else {
        racer.car.surfaceGrip() = 1.0f;
        racer.car.surfaceDrag() = 1.0f;
    }
}

void RaceState::updateMidpointFlag(
    RacerEntry& racer, const Track::Progress& prog, int numSegments)
{
    int mid = numSegments / 2;
    int midWindow = numSegments / 10;

    if (std::abs(prog.segmentIndex - mid) <= midWindow) {
        racer.passedMidpoint = true;
    }
}

void RaceState::updateLapCount(
    RacerEntry& racer, const Track::Progress& prog, int numSegments)
{
    int highSeg = numSegments * 7 / 10;
    int lowSeg = numSegments * 3 / 10;
    bool crossedFinish = racer.passedMidpoint
        && racer.lastSegment > highSeg
        && prog.segmentIndex < lowSeg;

    if (!crossedFinish) {
        return;
    }
    racer.lap += 1;
    racer.passedMidpoint = false;
    if (racer.lap >= lapsToWin_) {
        racer.finished = true;
        racer.finishTime = elapsedTime_;
    }
}

float RaceState::raceProgress(const RacerEntry& racer) const
{
    Track::Progress prog = track_.projectPosition(racer.car.position());
    float lapDist = static_cast<float>(racer.lap) * track_.totalLength();

    return lapDist + track_.cumulativeDistance(prog);
}

std::vector<int> RaceState::standings() const
{
    std::vector<int> order(racers_.size());
    std::iota(order.begin(), order.end(), 0);

    std::sort(order.begin(), order.end(), [this](int a, int b) {
        const RacerEntry& ra = racers_[static_cast<size_t>(a)];
        const RacerEntry& rb = racers_[static_cast<size_t>(b)];

        if (ra.finished != rb.finished) {
            return ra.finished;
        }
        if (ra.finished && rb.finished) {
            return ra.finishTime < rb.finishTime;
        }
        return raceProgress(ra) > raceProgress(rb);
    });

    return order;
}

int RaceState::playerPosition() const
{
    std::vector<int> order = standings();

    for (size_t i = 0; i < order.size(); ++i) {
        if (order[i] == playerIndex_) {
            return static_cast<int>(i) + 1;
        }
    }
    return static_cast<int>(order.size());
}

} // namespace racer
