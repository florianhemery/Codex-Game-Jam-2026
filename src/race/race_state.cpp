#include "race/race_state.h"

#include <algorithm>
#include <numeric>

namespace racer {

RaceState::RaceState(Track track, int lapsToWin, int aiCount) : track_(std::move(track)), lapsToWin_(lapsToWin) {
    int totalCars = aiCount + 1;

    RacerEntry player;
    player.name = "Joueur";
    player.isPlayer = true;
    player.car.position = track_.StartPosition(0, totalCars);
    player.car.heading = track_.StartHeading();
    player.car.velocityHeading = player.car.heading;
    racers_.push_back(player);
    playerIndex_ = 0;

    for (int i = 0; i < aiCount; ++i) {
        RacerEntry ai;
        ai.name = "IA " + std::to_string(i + 1);
        ai.car.position = track_.StartPosition(i + 1, totalCars);
        ai.car.heading = track_.StartHeading();
        ai.car.velocityHeading = ai.car.heading;
        racers_.push_back(ai);

        float skill = 0.85f + 0.05f * static_cast<float>(i); // legere variation pour ne pas avoir un peloton identique
        aiDrivers_.emplace_back(skill);
    }
}

void RaceState::Update(float dt, const CarInput& playerInput) {
    if (phase_ == RacePhase::Countdown) {
        countdownRemaining_ -= dt;
        if (countdownRemaining_ <= 0.0f) {
            countdownRemaining_ = 0.0f;
            phase_ = RacePhase::Racing;
        }
        return;
    }

    if (phase_ == RacePhase::Finished) return;

    elapsedTime_ += dt;

    int numSegments = static_cast<int>(track_.Waypoints().size());

    for (size_t i = 0; i < racers_.size(); ++i) {
        RacerEntry& r = racers_[i];
        if (r.finished) continue;

        CarInput input = r.isPlayer ? playerInput : aiDrivers_[i - 1].ComputeInput(r.car, track_);
        r.car.Update(input, dt);

        Track::Progress prog = track_.ProjectPosition(r.car.position);

        // Detection de tour complete : passage de la fin de piste (dernier
        // segment) au debut (segment 0) dans le sens de la marche.
        if (r.lastSegment > numSegments * 7 / 10 && prog.segmentIndex < numSegments * 3 / 10) {
            r.lap += 1;
            if (r.lap >= lapsToWin_) {
                r.finished = true;
                r.finishTime = elapsedTime_;
            }
        }
        r.lastSegment = prog.segmentIndex;

        if (r.isPlayer && r.finished) {
            phase_ = RacePhase::Finished;
        }
    }
}

float RaceState::RaceProgress(const RacerEntry& r) const {
    Track::Progress prog = track_.ProjectPosition(r.car.position);
    return static_cast<float>(r.lap) * track_.TotalLength() + track_.CumulativeDistance(prog);
}

std::vector<int> RaceState::Standings() const {
    std::vector<int> order(racers_.size());
    std::iota(order.begin(), order.end(), 0);

    std::sort(order.begin(), order.end(), [this](int a, int b) {
        const RacerEntry& ra = racers_[static_cast<size_t>(a)];
        const RacerEntry& rb = racers_[static_cast<size_t>(b)];
        if (ra.finished != rb.finished) return ra.finished && !rb.finished ? true : false;
        if (ra.finished && rb.finished) return ra.finishTime < rb.finishTime;
        return RaceProgress(ra) > RaceProgress(rb);
    });

    return order;
}

int RaceState::PlayerPosition() const {
    std::vector<int> order = Standings();
    for (size_t i = 0; i < order.size(); ++i) {
        if (order[i] == playerIndex_) return static_cast<int>(i) + 1;
    }
    return static_cast<int>(order.size());
}

} // namespace racer
