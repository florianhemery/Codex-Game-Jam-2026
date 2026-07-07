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
    COUNTDOWN,
    RACING,
    FINISHED,
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

    void update(float dt, const CarInput& playerInput);

    RacePhase phase() const { return phase_; }
    float countdownRemaining() const { return countdownRemaining_; }
    float elapsedTime() const { return elapsedTime_; }
    int lapsToWin() const { return lapsToWin_; }

    const std::vector<RacerEntry>& racers() const { return racers_; }
    const Track& getTrack() const { return track_; }

    std::vector<int> standings() const;
    int playerPosition() const;
    int playerIndex() const { return playerIndex_; }

private:
    void initPlayer(int totalCars);
    void initAiRacer(int aiIndex, int totalCars);
    void updateCountdown(float dt);
    void updateRacers(float dt, const CarInput& playerInput);
    void updateSingleRacer(
        size_t index, float dt, const CarInput& playerInput, int numSegments);
    void applySurfaceGrip(RacerEntry& racer, const Track::Progress& prog);
    void updateMidpointFlag(
        RacerEntry& racer, const Track::Progress& prog, int numSegments);
    void updateLapCount(
        RacerEntry& racer, const Track::Progress& prog, int numSegments);
    void finalizePlayerIfDone(const RacerEntry& racer);
    float raceProgress(const RacerEntry& racer) const;

    Track track_;
    std::vector<RacerEntry> racers_;
    std::vector<AIDriver> aiDrivers_;
    int playerIndex_ = 0;
    int lapsToWin_;
    RacePhase phase_ = RacePhase::COUNTDOWN;
    float countdownRemaining_ = 3.0f;
    float elapsedTime_ = 0.0f;
};

} // namespace racer

#endif /* !RACE_STATE_HPP_ */
