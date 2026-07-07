/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Headless race simulation for game logic validation
*/

#ifndef RACE_SIM_DEBUG_HPP_
#define RACE_SIM_DEBUG_HPP_

#include <vector>

#include "Ai/AiDriver.hpp"
#include "Race/RaceState.hpp"
#include "Track/Track.hpp"

inline constexpr int kRaceSimAiCount = 4;
inline constexpr int kRaceSimLaps = 3;
inline constexpr float kRaceSimDt = 1.0f / 60.0f;
inline constexpr int kRaceSimMaxSteps = 60 * 180;
inline constexpr float kRaceSimEarlyLapWindow = 5.0f;
inline constexpr float kRaceSimContactDist = 3.0f;
inline constexpr float kRaceSimOverlapDist = 2.7f;
inline constexpr int kRaceSimProgressLogInterval = 300;

class RaceSimPrinter;

class RaceSimDebug {
public:
    struct SimContext {
        racer::RaceState race;
        racer::AIDriver playerAsAi;
        size_t carCount = 0;
        std::vector<int> offTrackExits;
        std::vector<bool> wasOffTrack;
        float minPairDist = 1e9f;
        int contactPairFrames = 0;
        int overlapPairFrames = 0;
        bool sawNaN = false;
        bool earlyFalseLap = false;
        float offTrackHalfWidth = 0.0f;
        int step = 0;
        int presetIndex = 0;

        SimContext(const racer::TrackDef &def, int laps, int aiCount,
            int preset);
    };

    static bool runRace(int presetIndex);

private:
    friend class RaceSimPrinter;

    static SimContext initSimulation(int presetIndex);
    static bool advanceFrames(SimContext &ctx);
    static bool printResults(SimContext &ctx);

    static float totalProgress(
        const racer::Track &track, const racer::RacerEntry &entry);
    static bool isFiniteCar(const racer::Car &car);
    static void checkEarlyLaps(SimContext &ctx);
    static void updateCarMetrics(SimContext &ctx, size_t carIndex);
    static void updatePairMetrics(
        SimContext &ctx, const racer::Car &a, const racer::Car &b);
    static bool evaluateHealth(const SimContext &ctx);
    static bool anyFrozenCar(const SimContext &ctx);
};

#endif /* !RACE_SIM_DEBUG_HPP_ */
