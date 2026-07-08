/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Headless race simulation for game logic validation
*/

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "Tools/RaceSimDebug.hpp"
#include "Tools/RaceSimPrinter.hpp"

RaceSimDebug::SimContext::SimContext(
    const racer::TrackDef &def, int laps, int aiCount, int preset)
    : race(racer::Track::make(def), laps, aiCount), playerAsAi(1.0f),
      presetIndex(preset)
{
    carCount = race.racers().size();
    offTrackExits.assign(carCount, 0);
    wasOffTrack.assign(carCount, false);
    offTrackHalfWidth = race.getTrack().width() * 0.5f + 0.6f;
}

float RaceSimDebug::totalProgress(
    const racer::Track &track, const racer::RacerEntry &entry)
{
    racer::Track::Progress prog = track.projectPosition(entry.car.position());

    return static_cast<float>(entry.lap) * track.totalLength()
        + track.cumulativeDistance(prog);
}

bool RaceSimDebug::isFiniteCar(const racer::Car &car)
{
    return std::isfinite(car.position().x) && std::isfinite(car.position().z)
        && std::isfinite(car.speed()) && std::isfinite(car.velocityHeading())
        && std::isfinite(car.heading());
}

void RaceSimDebug::checkEarlyLaps(SimContext &ctx)
{
    if (ctx.race.elapsedTime() >= kRaceSimEarlyLapWindow)
        return;
    for (const auto &entry : ctx.race.racers()) {
        if (entry.lap > 0)
            ctx.earlyFalseLap = true;
    }
}

void RaceSimDebug::updatePairMetrics(
    SimContext &ctx, const racer::Car &a, const racer::Car &b)
{
    float dx = b.position().x - a.position().x;
    float dz = b.position().z - a.position().z;
    float dist = std::sqrt(dx * dx + dz * dz);

    ctx.minPairDist = std::min(ctx.minPairDist, dist);
    if (dist < kRaceSimContactDist)
        ctx.contactPairFrames++;
    if (dist < kRaceSimOverlapDist)
        ctx.overlapPairFrames++;
}

void RaceSimDebug::updateCarMetrics(SimContext &ctx, size_t carIndex)
{
    const racer::RacerEntry &entry = ctx.race.racers()[carIndex];

    if (!isFiniteCar(entry.car))
        ctx.sawNaN = true;
    float lateral = ctx.race.getTrack()
        .projectPosition(entry.car.position()).lateralOffset;
    bool off = std::fabs(lateral) > ctx.offTrackHalfWidth;

    if (off && !ctx.wasOffTrack[carIndex])
        ctx.offTrackExits[carIndex]++;
    ctx.wasOffTrack[carIndex] = off;
    for (size_t j = carIndex + 1; j < ctx.carCount; ++j)
        updatePairMetrics(ctx, entry.car, ctx.race.racers()[j].car);
}

bool RaceSimDebug::anyFrozenCar(const SimContext &ctx)
{
    for (const auto &entry : ctx.race.racers()) {
        if (!entry.finished && std::fabs(entry.car.speed()) < 0.5f)
            return true;
    }
    return false;
}

bool RaceSimDebug::evaluateHealth(const SimContext &ctx)
{
    bool finished = ctx.race.phase() == racer::RacePhase::FINISHED;
    int playerLap =
        ctx.race.racers()[static_cast<size_t>(ctx.race.playerIndex())].lap;

    return finished && !ctx.earlyFalseLap && !ctx.sawNaN
        && !anyFrozenCar(ctx) && playerLap >= kRaceSimLaps;
}

RaceSimDebug::SimContext RaceSimDebug::initSimulation(int presetIndex)
{
    const racer::TrackDef &def =
        racer::Track::presets()[static_cast<size_t>(presetIndex)];
    SimContext ctx(def, kRaceSimLaps, kRaceSimAiCount, presetIndex);

    RaceSimPrinter::printPresetHeader(presetIndex, def, ctx);
    return ctx;
}

bool RaceSimDebug::advanceFrames(SimContext &ctx)
{
    for (; ctx.step < kRaceSimMaxSteps; ++ctx.step) {
        size_t playerIdx = static_cast<size_t>(ctx.race.playerIndex());
        racer::CarInput playerInput = ctx.playerAsAi.computeInput(
            ctx.race.racers()[playerIdx].car, ctx.race.getTrack());

        ctx.race.update(kRaceSimDt, playerInput);
        checkEarlyLaps(ctx);
        for (size_t i = 0; i < ctx.carCount; ++i)
            updateCarMetrics(ctx, i);
        if (ctx.step % kRaceSimProgressLogInterval == 0)
            RaceSimPrinter::printProgressLog(ctx);
        if (ctx.race.phase() == racer::RacePhase::FINISHED)
            return true;
    }
    return false;
}

bool RaceSimDebug::printResults(SimContext &ctx)
{
    RaceSimPrinter::printStandings(ctx);
    bool ok = evaluateHealth(ctx);

    RaceSimPrinter::printHealthErrors(ctx);
    RaceSimPrinter::printFinalVerdict(ctx.presetIndex, ok);
    return ok;
}

bool RaceSimDebug::runRace(int presetIndex)
{
    SimContext ctx = initSimulation(presetIndex);

    advanceFrames(ctx);
    return printResults(ctx);
}

static bool isValidPresetIndex(int idx)
{
    return idx >= 0
        && idx < static_cast<int>(racer::Track::presets().size());
}

static void printInvalidPreset(int idx)
{
    int last = static_cast<int>(racer::Track::presets().size()) - 1;

    std::printf("Index de preset invalide : %d (0..%d)\n", idx, last);
}

static bool runAllPresets()
{
    bool allOk = true;
    int presetCount = static_cast<int>(racer::Track::presets().size());

    for (int i = 0; i < presetCount; ++i)
        allOk = RaceSimDebug::runRace(i) && allOk;
    return allOk;
}

int main(int argc, char **argv)
{
    bool allOk = true;

    if (argc > 1) {
        int idx = std::atoi(argv[1]);

        if (!isValidPresetIndex(idx)) {
            printInvalidPreset(idx);
            return 1;
        }
        allOk = RaceSimDebug::runRace(idx);
    } else {
        allOk = runAllPresets();
    }
    std::printf("RACE_SIM_GLOBAL: %s\n", allOk ? "OK" : "ECHEC");
    return allOk ? 0 : 1;
}

