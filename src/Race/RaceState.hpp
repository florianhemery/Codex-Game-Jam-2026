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
    WRAP_UP,
    FINISHED,
};

enum class StartLaunchGrade {
    None,
    Perfect,
    Good,
    Ok,
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
    bool allRacersFinished() const;

    StartLaunchGrade launchGrade() const { return launchGrade_; }
    float launchBannerTimer() const { return launchBanner_; }
    float falseStartBannerTimer() const { return falseStartBanner_; }
    float startBoostRemaining() const;

private:
    void processCountdownInput(const CarInput &input);
    void beginRacing();
    void applyLaunchGrade(float reactionSeconds, RacerEntry &player);
    void tickStartBanners(float dt);

    Track track_;
    std::vector<RacerEntry> racers_;
    std::vector<AIDriver> aiDrivers_;
    int playerIndex_ = 0;
    int lapsToWin_;
    RacePhase phase_ = RacePhase::COUNTDOWN;
    float countdownRemaining_ = 3.0f;
    float elapsedTime_ = 0.0f;
    float wrapUpTimer_ = 0.0f;
    bool revvedDuringCountdown_ = false;
    bool waitingForLaunch_ = false;
    StartLaunchGrade launchGrade_ = StartLaunchGrade::None;
    float launchBanner_ = 0.0f;
    float falseStartBanner_ = 0.0f;
    int falseStartCount_ = 0;
    static constexpr float kPerfectLaunchMax = 0.12f;
    static constexpr float kGoodLaunchMax = 0.28f;
    static constexpr float kOkLaunchMax = 0.50f;
    static constexpr float kWrapUpMaxSeconds = 12.0f;
    static constexpr float kWrapUpTimeScale = 3.0f;
    static constexpr int kMaxFalseStarts = 1;

    void initPlayer(int totalCars);
    void initAiRacer(int aiIndex, int totalCars);
    void updateCountdown(float dt);
    void updateRacers(float dt, const CarInput& playerInput);
    void updateSingleRacer(
        size_t index, float dt, const CarInput& playerInput, int numSegments);
    void applySurfaceGrip(RacerEntry& racer, const Track::Progress& prog);
    void applyTrackHeight(RacerEntry& racer, float dt);
    void updateMidpointFlag(
        RacerEntry& racer, const Track::Progress& prog, int numSegments);
    void updateLapCount(
        RacerEntry& racer, const Track::Progress& prog, int numSegments);
    void finalizePlayerIfDone(RacerEntry& racer);
    void updateWrapUp(float dt);
    float raceProgress(const RacerEntry& racer) const;
};

} // namespace racer

#endif /* !RACE_STATE_HPP_ */
