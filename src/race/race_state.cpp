/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Race state machine (countdown, laps, standings, collisions)
*/

#include "race/race_state.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <numeric>

namespace racer {

float RaceState::NormalizeAngle(float angle)
{
    while (angle > PI) {
        angle -= 2.0f * PI;
    }
    while (angle < -PI) {
        angle += 2.0f * PI;
    }
    return angle;
}

float RaceState::Sign(float value)
{
    if (value > 0.0f) {
        return 1.0f;
    }
    if (value < 0.0f) {
        return -1.0f;
    }
    return 0.0f;
}

RaceState::RaceState(Track track, int lapsToWin, int aiCount)
    : track_(std::move(track))
    , lapsToWin_(lapsToWin)
{
    int totalCars = aiCount + 1;

    InitPlayer(totalCars);
    for (int i = 0; i < aiCount; ++i) {
        InitAiRacer(i, totalCars);
    }
}

void RaceState::InitPlayer(int totalCars)
{
    RacerEntry player;

    player.name = "Joueur";
    player.isPlayer = true;
    player.car.position = track_.StartPosition(0, totalCars);
    player.car.heading = track_.StartHeading();
    player.car.velocityHeading = player.car.heading;
    Track::Progress prog = track_.ProjectPosition(player.car.position);
    player.lastSegment = prog.segmentIndex;
    racers_.push_back(player);
    playerIndex_ = 0;
}

void RaceState::InitAiRacer(int aiIndex, int totalCars)
{
    RacerEntry ai;

    ai.name = "IA " + std::to_string(aiIndex + 1);
    ai.car.position = track_.StartPosition(aiIndex + 1, totalCars);
    ai.car.heading = track_.StartHeading();
    ai.car.velocityHeading = ai.car.heading;
    Track::Progress prog = track_.ProjectPosition(ai.car.position);
    ai.lastSegment = prog.segmentIndex;
    racers_.push_back(ai);

    float skill = 1.0f - 0.05f * static_cast<float>(aiIndex);
    ai.car.tuning.maxSpeed *= 0.90f + 0.10f * skill;
    ai.car.tuning.acceleration *= 0.85f + 0.15f * skill;
    unsigned int seed = static_cast<unsigned int>(1000 + aiIndex * 7919);
    aiDrivers_.emplace_back(skill, seed);
}

void RaceState::Update(float dt, const CarInput& playerInput)
{
    if (phase_ == RacePhase::Countdown) {
        UpdateCountdown(dt);
        return;
    }
    if (phase_ == RacePhase::Finished) {
        return;
    }
    UpdateRacers(dt, playerInput);
}

void RaceState::UpdateCountdown(float dt)
{
    countdownRemaining_ -= dt;
    if (countdownRemaining_ <= 0.0f) {
        countdownRemaining_ = 0.0f;
        phase_ = RacePhase::Racing;
    }
}

void RaceState::UpdateRacers(float dt, const CarInput& playerInput)
{
    elapsedTime_ += dt;
    int numSegments = static_cast<int>(track_.Waypoints().size());

    for (size_t i = 0; i < racers_.size(); ++i) {
        UpdateSingleRacer(i, dt, playerInput, numSegments);
    }
    ResolveCarContacts();
}

void RaceState::UpdateSingleRacer(
    size_t index, float dt, const CarInput& playerInput, int numSegments)
{
    RacerEntry& racer = racers_[index];
    if (racer.finished) {
        return;
    }
    CarInput input = racer.isPlayer
        ? playerInput
        : aiDrivers_[index - 1].ComputeInput(racer.car, track_);
    racer.lastInput = input;
    racer.car.Update(input, dt);
    Track::Progress prog = track_.ProjectPosition(racer.car.position);
    ApplySurfaceGrip(racer, prog);
    UpdateMidpointFlag(racer, prog, numSegments);
    UpdateLapCount(racer, prog, numSegments);
    racer.lastSegment = prog.segmentIndex;
    if (racer.isPlayer && racer.finished) {
        phase_ = RacePhase::Finished;
    }
}

void RaceState::ApplySurfaceGrip(
    RacerEntry& racer, const Track::Progress& prog)
{
    float grassLimit = track_.Width() * 0.5f + 0.6f;

    if (std::fabs(prog.lateralOffset) > grassLimit) {
        racer.car.surfaceGrip = 0.55f;
        racer.car.surfaceDrag = 3.0f;
    } else if (track_.Style() == SurfaceStyle::Abimee) {
        racer.car.surfaceGrip = 0.85f;
        racer.car.surfaceDrag = 1.15f;
    } else {
        racer.car.surfaceGrip = 1.0f;
        racer.car.surfaceDrag = 1.0f;
    }
}

void RaceState::UpdateMidpointFlag(
    RacerEntry& racer, const Track::Progress& prog, int numSegments)
{
    int mid = numSegments / 2;
    int midWindow = numSegments / 10;

    if (std::abs(prog.segmentIndex - mid) <= midWindow) {
        racer.passedMidpoint = true;
    }
}

void RaceState::UpdateLapCount(
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

// Collisions voiture-voiture : spheres de rayon 1.5 sur les positions.
// Resolution purement positionnelle + amortissement leger, pensee pour rester
// stable a 60 Hz meme en peloton serre (corrections bornees, relaxation 50 %,
// jamais de NaN si deux positions sont confondues).
void RaceState::ResolveCarContacts()
{
    for (size_t i = 0; i < racers_.size(); ++i) {
        if (racers_[i].finished) {
            continue;
        }
        for (size_t j = i + 1; j < racers_.size(); ++j) {
            if (racers_[j].finished) {
                continue;
            }
            ResolveContactPair(i, j);
        }
    }
}

bool RaceState::TryPrepareContact(
    size_t i, size_t j, float& nx, float& nz, float& overlap)
{
    constexpr float kContactDist = 3.0f;
    const Car& a = racers_[i].car;
    const Car& b = racers_[j].car;
    float dx = b.position.x - a.position.x;
    float dz = b.position.z - a.position.z;
    float distSq = dx * dx + dz * dz;

    if (distSq >= kContactDist * kContactDist) {
        return false;
    }
    float dist = std::sqrt(distSq);
    if (dist > 1e-4f) {
        nx = dx / dist;
        nz = dz / dist;
    } else {
        Vector3 fwd = a.Forward();
        nx = fwd.x;
        nz = fwd.z;
        dist = 0.0f;
    }
    overlap = kContactDist - dist;
    return true;
}

void RaceState::ResolveContactPair(size_t i, size_t j)
{
    float nx = 0.0f;
    float nz = 0.0f;
    float overlap = 0.0f;

    if (!TryPrepareContact(i, j, nx, nz, overlap)) {
        return;
    }
    Car& a = racers_[i].car;
    Car& b = racers_[j].car;
    ApplyContactSeparation(a, b, nx, nz, overlap);
    ApplyContactDamping(a, b, nx, nz);
    ApplyContactDeflection(a, b, nx, nz, overlap);
}

void RaceState::ApplyContactSeparation(
    Car& a, Car& b, float nx, float nz, float overlap)
{
    constexpr float kMaxPush = 0.25f;
    float push = std::min(overlap * 0.25f, kMaxPush);

    a.position.x -= nx * push;
    a.position.z -= nz * push;
    b.position.x += nx * push;
    b.position.z += nz * push;
}

void RaceState::ApplyContactDamping(Car& a, Car& b, float nx, float nz)
{
    constexpr float kSpeedDamping = 0.96f;
    Vector3 va = a.Velocity();
    Vector3 vb = b.Velocity();
    float closing = (va.x - vb.x) * nx + (va.z - vb.z) * nz;
    float t = std::clamp(closing / 6.0f, 0.0f, 1.0f);
    float damping = 1.0f - (1.0f - kSpeedDamping) * t;
    bool aRams = va.x * nx + va.z * nz > 0.0f;
    bool bRams = vb.x * nx + vb.z * nz < 0.0f;

    if (aRams) {
        a.speed *= damping;
    }
    if (bRams) {
        b.speed *= damping;
    }
}

void RaceState::NudgeLateral(
    Car& car, float fwdX, float fwdZ, float push, float sideSign)
{
    car.position.x += sideSign * fwdZ * push * 0.6f;
    car.position.z += sideSign * (-fwdX) * push * 0.6f;
}

void RaceState::ApplyContactDeflection(
    Car& a, Car& b, float nx, float nz, float overlap)
{
    constexpr float kMaxPush = 0.25f;
    constexpr float kMaxDeflect = 0.06f;
    float push = std::min(overlap * 0.25f, kMaxPush);
    float deflect = std::min(kMaxDeflect, overlap * 0.04f);
    float ax = std::sin(a.velocityHeading);
    float az = std::cos(a.velocityHeading);
    float bx = std::sin(b.velocityHeading);
    float bz = std::cos(b.velocityHeading);
    float sideA = Sign(ax * nz - az * nx);
    float sideB = Sign(bx * (-nz) - bz * (-nx));

    a.velocityHeading = NormalizeAngle(
        a.velocityHeading + sideA * deflect);
    b.velocityHeading = NormalizeAngle(
        b.velocityHeading + sideB * deflect);

    Vector3 va = a.Velocity();
    Vector3 vb = b.Velocity();
    if (va.x * nx + va.z * nz > 0.0f) {
        NudgeLateral(a, ax, az, push, sideA);
    }
    if (vb.x * nx + vb.z * nz < 0.0f) {
        NudgeLateral(b, bx, bz, push, sideB);
    }
}

float RaceState::RaceProgress(const RacerEntry& racer) const
{
    Track::Progress prog = track_.ProjectPosition(racer.car.position);
    float lapDist = static_cast<float>(racer.lap) * track_.TotalLength();

    return lapDist + track_.CumulativeDistance(prog);
}

std::vector<int> RaceState::Standings() const
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
        return RaceProgress(ra) > RaceProgress(rb);
    });

    return order;
}

int RaceState::PlayerPosition() const
{
    std::vector<int> order = Standings();

    for (size_t i = 0; i < order.size(); ++i) {
        if (order[i] == playerIndex_) {
            return static_cast<int>(i) + 1;
        }
    }
    return static_cast<int>(order.size());
}

} // namespace racer
