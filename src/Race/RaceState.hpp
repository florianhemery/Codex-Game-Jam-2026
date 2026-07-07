/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Race state machine (countdown, laps, standings, collisions)
*/

#ifndef RACE_STATE_HPP_
#define RACE_STATE_HPP_

#include <string>
#include <vector>

#include "Ai/AiDriver.hpp"
#include "Track/Track.hpp"
#include "Vehicle/Car.hpp"

namespace racer {

enum class RacePhase {
    Countdown,
    Racing,
    Finished,
};

struct RacerEntry {
    std::string name;
    Car car;
    CarInput lastInput{};
    int lap = 0;
    int lastSegment = 0;
    bool passedMidpoint = false;
    bool finished = false;
    float finishTime = 0.0f;
    bool isPlayer = false;
};

// Machine a etats du but du jeu : compte a rebours -> course -> arrivee du
// joueur. C'est ce qui repond directement au "il y a aucun but" -- finir en
// tete d'une course a N tours.
class RaceState {
public:
    RaceState(Track track, int lapsToWin, int aiCount);

    void Update(float dt, const CarInput& playerInput);

    RacePhase Phase() const { return phase_; }
    float CountdownRemaining() const { return countdownRemaining_; }
    float ElapsedTime() const { return elapsedTime_; }
    int LapsToWin() const { return lapsToWin_; }

    const std::vector<RacerEntry>& Racers() const { return racers_; }
    const Track& GetTrack() const { return track_; }

    std::vector<int> Standings() const;
    int PlayerPosition() const;
    int PlayerIndex() const { return playerIndex_; }

private:
    static float NormalizeAngle(float angle);
    static float Sign(float value);

    void InitPlayer(int totalCars);
    void InitAiRacer(int aiIndex, int totalCars);
    void UpdateCountdown(float dt);
    void UpdateRacers(float dt, const CarInput& playerInput);
    void UpdateSingleRacer(
        size_t index, float dt, const CarInput& playerInput, int numSegments);
    void ApplySurfaceGrip(RacerEntry& racer, const Track::Progress& prog);
    void UpdateMidpointFlag(
        RacerEntry& racer, const Track::Progress& prog, int numSegments);
    void UpdateLapCount(
        RacerEntry& racer, const Track::Progress& prog, int numSegments);
    float RaceProgress(const RacerEntry& racer) const;
    void ResolveCarContacts();
    void ResolveContactPair(size_t i, size_t j);
    bool TryPrepareContact(
        size_t i, size_t j, float& nx, float& nz, float& overlap);
    void ApplyContactSeparation(
        Car& a, Car& b, float nx, float nz, float overlap);
    void ApplyContactDamping(Car& a, Car& b, float nx, float nz);
    void ApplyContactDeflection(
        Car& a, Car& b, float nx, float nz, float overlap);
    void NudgeLateral(
        Car& car, float fwdX, float fwdZ, float push, float sideSign);

    Track track_;
    std::vector<RacerEntry> racers_;
    std::vector<AIDriver> aiDrivers_;
    int playerIndex_ = 0;
    int lapsToWin_;
    RacePhase phase_ = RacePhase::Countdown;
    float countdownRemaining_ = 3.0f;
    float elapsedTime_ = 0.0f;
};

} // namespace racer

#endif /* !RACE_STATE_HPP_ */
