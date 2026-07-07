#pragma once

#include <string>
#include <vector>

#include "ai/ai_driver.h"
#include "track/track.h"
#include "vehicle/car.h"

namespace racer {

enum class RacePhase {
    Countdown,
    Racing,
    Finished,
};

struct RacerEntry {
    std::string name;
    Car car;
    CarInput lastInput{}; // input effectivement applique cette frame (feux stop, VFX)
    int lap = 0;
    int lastSegment = 0;
    bool passedMidpoint = false; // garde-fou anti faux-tour (cf. race_state.cpp)
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

    std::vector<int> Standings() const; // indices dans Racers(), 1er en tete
    int PlayerPosition() const;         // 1-based
    int PlayerIndex() const { return playerIndex_; }

private:
    float RaceProgress(const RacerEntry& r) const;
    void ResolveCarContacts(); // collisions voiture-voiture (spheres)

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
