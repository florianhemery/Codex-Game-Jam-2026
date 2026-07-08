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
    applyTrackHeight(player, 0.0f);
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
    applyTrackHeight(ai, 0.0f);
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
        processCountdownInput(playerInput);
        updateCountdown(dt);
        tickStartBanners(dt);
        return;
    }
    if (phase_ == RacePhase::FINISHED) {
        tickStartBanners(dt);
        return;
    }
    if (phase_ == RacePhase::WRAP_UP) {
        updateWrapUp(dt);
        tickStartBanners(dt);
        return;
    }
    updateRacers(dt, playerInput);
    tickStartBanners(dt);
}

bool RaceState::allRacersFinished() const
{
    for (const RacerEntry& racer : racers_) {
        if (!racer.finished) {
            return false;
        }
    }
    return true;
}

void RaceState::updateWrapUp(float dt)
{
    wrapUpTimer_ += dt;
    const float simDt = dt * kWrapUpTimeScale;
    int numSegments = static_cast<int>(track_.waypoints().size());
    CarInput idle{};

    elapsedTime_ += simDt;
    for (size_t i = 0; i < racers_.size(); ++i) {
        RacerEntry& racer = racers_[i];
        if (racer.finished) {
            continue;
        }
        CarInput input = racer.isPlayer
            ? idle
            : aiDrivers_[i - 1].computeInput(racer.car, track_);
        racer.lastInput = input;
        racer.car.update(input, simDt);
        Track::Progress prog = track_.projectPosition(racer.car.position());
        applyTrackHeight(racer, simDt);
        prog = track_.projectPosition(racer.car.position());
        applySurfaceGrip(racer, prog);
        updateMidpointFlag(racer, prog, numSegments);
        updateLapCount(racer, prog, numSegments);
        racer.lastSegment = prog.segmentIndex;
    }
    RaceContactResolver::resolveAll(racers_);
    if (allRacersFinished() || wrapUpTimer_ >= kWrapUpMaxSeconds) {
        phase_ = RacePhase::FINISHED;
    }
}

void RaceState::processCountdownInput(const CarInput &input)
{
    if (input.throttle > 0.08f || input.handbrake || input.nitro) {
        revvedDuringCountdown_ = true;
    }
}

void RaceState::beginRacing()
{
    phase_ = RacePhase::RACING;
    elapsedTime_ = 0.0f;
    waitingForLaunch_ = true;
    launchGrade_ = StartLaunchGrade::None;
}

void RaceState::applyLaunchGrade(float reactionSeconds, RacerEntry &player)
{
    if (reactionSeconds <= kPerfectLaunchMax) {
        launchGrade_ = StartLaunchGrade::Perfect;
        player.car.startBoostTimer() = 3.5f;
        player.car.startBoostAccelMul() = 1.38f;
        player.car.startBoostSpeedBonus() = 5.0f;
        player.car.nitroRemaining() = std::min(
            player.car.tuning().nitroCapacity,
            player.car.nitroRemaining() + 1.2f);
        launchBanner_ = 2.2f;
    } else if (reactionSeconds <= kGoodLaunchMax) {
        launchGrade_ = StartLaunchGrade::Good;
        player.car.startBoostTimer() = 2.5f;
        player.car.startBoostAccelMul() = 1.22f;
        player.car.startBoostSpeedBonus() = 3.0f;
        launchBanner_ = 1.8f;
    } else if (reactionSeconds <= kOkLaunchMax) {
        launchGrade_ = StartLaunchGrade::Ok;
        player.car.startBoostTimer() = 1.5f;
        player.car.startBoostAccelMul() = 1.10f;
        player.car.startBoostSpeedBonus() = 1.5f;
        launchBanner_ = 1.4f;
    }
}

void RaceState::tickStartBanners(float dt)
{
    launchBanner_ = std::max(0.0f, launchBanner_ - dt);
    falseStartBanner_ = std::max(0.0f, falseStartBanner_ - dt);
}

float RaceState::startBoostRemaining() const
{
    if (playerIndex_ < 0
        || playerIndex_ >= static_cast<int>(racers_.size())) {
        return 0.0f;
    }
    return racers_[static_cast<size_t>(playerIndex_)].car.startBoostTimer();
}

void RaceState::updateCountdown(float dt)
{
    countdownRemaining_ -= dt;
    if (countdownRemaining_ > 0.0f) {
        return;
    }
    countdownRemaining_ = 0.0f;
    // A false start only re-arms the countdown a bounded number of times.
    // Without a cap, an input source that holds throttle continuously
    // (a player anticipating the green light, or a scripted/AI input)
    // re-triggers revvedDuringCountdown_ every single frame, so the
    // countdown would reset to 3s forever and the race would never begin.
    if (revvedDuringCountdown_ && falseStartCount_ < kMaxFalseStarts) {
        revvedDuringCountdown_ = false;
        falseStartCount_ += 1;
        countdownRemaining_ = 3.0f;
        falseStartBanner_ = 2.8f;
        return;
    }
    revvedDuringCountdown_ = false;
    beginRacing();
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

void RaceState::finalizePlayerIfDone(RacerEntry& racer)
{
    if (!racer.isPlayer || !racer.finished || phase_ != RacePhase::RACING) {
        return;
    }
    phase_ = RacePhase::WRAP_UP;
    wrapUpTimer_ = 0.0f;
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
    if (racer.isPlayer && waitingForLaunch_ && input.throttle > 0.08f) {
        waitingForLaunch_ = false;
        applyLaunchGrade(elapsedTime_, racer);
    }
    racer.lastInput = input;
    racer.car.update(input, dt);
    Track::Progress prog = track_.projectPosition(racer.car.position());
    applyTrackHeight(racer, dt);
    prog = track_.projectPosition(racer.car.position());
    applySurfaceGrip(racer, prog);
    updateMidpointFlag(racer, prog, numSegments);
    updateLapCount(racer, prog, numSegments);
    racer.lastSegment = prog.segmentIndex;
    finalizePlayerIfDone(racer);
}

void RaceState::applyTrackHeight(RacerEntry& racer, float /*dt*/)
{
    constexpr float kTrackSurfaceY = 0.06f;

    racer.car.position().y = kTrackSurfaceY;
    racer.car.verticalVelocity() = 0.0f;
    racer.car.airborne() = false;
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
