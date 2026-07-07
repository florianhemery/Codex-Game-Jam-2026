/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Headless race simulation for game logic validation
*/

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "Ai/AiDriver.hpp"
#include "Race/RaceState.hpp"
#include "Track/Track.hpp"

namespace {

constexpr int kAiCount = 4;
constexpr int kLaps = 3;
constexpr float kDt = 1.0f / 60.0f;
constexpr int kMaxSteps = 60 * 180;
constexpr float kEarlyLapWindow = 5.0f;
constexpr float kContactDist = 3.0f;
constexpr float kOverlapDist = 2.7f;
constexpr int kProgressLogInterval = 300;

class RaceSimDebug {
public:
    static bool runRace(int presetIndex);

private:
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

        SimContext(const racer::TrackDef &def, int laps, int aiCount)
            : race(racer::Track::make(def), laps, aiCount), playerAsAi(1.0f)
        {
            carCount = race.racers().size();
            offTrackExits.assign(carCount, 0);
            wasOffTrack.assign(carCount, false);
            offTrackHalfWidth = race.getTrack().width() * 0.5f + 0.6f;
        }
    };

    static float totalProgress(
        const racer::Track &track, const racer::RacerEntry &entry);
    static bool isFiniteCar(const racer::Car &car);
    static void printPresetHeader(
        int presetIndex, const racer::TrackDef &def, const SimContext &ctx);
    static void checkEarlyLaps(SimContext &ctx);
    static void updateCarMetrics(SimContext &ctx, size_t carIndex);
    static void updatePairMetrics(
        SimContext &ctx, const racer::Car &a, const racer::Car &b);
    static void printProgressLog(const SimContext &ctx);
    static bool simulateSteps(SimContext &ctx);
    struct StandingRow {
        size_t rank = 0;
        int racerIndex = 0;
        float progress = 0.0f;
        float leaderProgress = 0.0f;
    };

    static void printStandingLine(
        const SimContext &ctx, const racer::RacerEntry &entry,
        const StandingRow &row);
    static void printStandingRows(
        SimContext &ctx, const std::vector<int> &order, float leaderProgress);
    static void printStabilityStats(const SimContext &ctx, float minGap,
        float maxGap);
    static void printStandings(SimContext &ctx);
    static bool evaluateHealth(const SimContext &ctx);
};

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

void RaceSimDebug::printPresetHeader(
    int presetIndex, const racer::TrackDef &def, const SimContext &ctx)
{
    const char *surface = def.surfaceStyle == racer::SurfaceStyle::ABIMEE
        ? "abimee" : "propre";

    std::printf(
        "=== Preset %d : %s (surface %s, longueur %.0f u) ===\n",
        presetIndex, def.name.c_str(), surface,
        ctx.race.getTrack().totalLength());
}

void RaceSimDebug::checkEarlyLaps(SimContext &ctx)
{
    if (ctx.race.elapsedTime() >= kEarlyLapWindow)
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
    if (dist < kContactDist)
        ctx.contactPairFrames++;
    if (dist < kOverlapDist)
        ctx.overlapPairFrames++;
}

void RaceSimDebug::updateCarMetrics(SimContext &ctx, size_t carIndex)
{
    const racer::RacerEntry &entry = ctx.race.racers()[carIndex];

    if (!isFiniteCar(entry.car))
        ctx.sawNaN = true;
    bool off = std::fabs(
        ctx.race.getTrack().projectPosition(entry.car.position()).lateralOffset)
        > ctx.offTrackHalfWidth;

    if (off && !ctx.wasOffTrack[carIndex])
        ctx.offTrackExits[carIndex]++;
    ctx.wasOffTrack[carIndex] = off;
    for (size_t j = carIndex + 1; j < ctx.carCount; ++j)
        updatePairMetrics(ctx, entry.car, ctx.race.racers()[j].car);
}

void RaceSimDebug::printProgressLog(const SimContext &ctx)
{
    std::printf("t=%5.1fs  ", static_cast<float>(ctx.step) * kDt);
    for (const auto &entry : ctx.race.racers()) {
        std::printf(
            "%s[lap=%d,v=%4.1f] ", entry.name.c_str(), entry.lap,
            entry.car.speed());
    }
    std::printf("pos_joueur=%d\n", ctx.race.playerPosition());
}

bool RaceSimDebug::simulateSteps(SimContext &ctx)
{
    for (; ctx.step < kMaxSteps; ++ctx.step) {
        racer::CarInput playerInput = ctx.playerAsAi.computeInput(
            ctx.race.racers()[static_cast<size_t>(ctx.race.playerIndex())].car,
            ctx.race.getTrack());

        ctx.race.update(kDt, playerInput);
        checkEarlyLaps(ctx);
        for (size_t i = 0; i < ctx.carCount; ++i)
            updateCarMetrics(ctx, i);
        if (ctx.step % kProgressLogInterval == 0)
            printProgressLog(ctx);
        if (ctx.race.phase() == racer::RacePhase::FINISHED)
            return true;
    }
    return false;
}

void RaceSimDebug::printStandingLine(
    const SimContext &ctx, const racer::RacerEntry &entry,
    const StandingRow &row)
{
    std::printf(
        "  %zu. %-7s tours=%d sorties_piste=%d ",
        row.rank + 1, entry.name.c_str(), entry.lap,
        ctx.offTrackExits[static_cast<size_t>(row.racerIndex)]);
    if (entry.finished) {
        std::printf("temps=%.2fs", entry.finishTime);
    } else {
        std::printf(
            "retard=%.0fu", row.leaderProgress - row.progress);
    }
    std::printf(" v=%.1f\n", entry.car.speed());
}

void RaceSimDebug::printStandingRows(SimContext &ctx,
    const std::vector<int> &order, float leaderProgress)
{
    float minGap = 1e9f;
    float maxGap = 0.0f;
    float prevProgress = leaderProgress;

    for (size_t p = 0; p < order.size(); ++p) {
        const racer::RacerEntry &entry =
            ctx.race.racers()[static_cast<size_t>(order[p])];
        float progress = totalProgress(ctx.race.getTrack(), entry);
        StandingRow row{p, order[p], progress, leaderProgress};

        if (p > 0) {
            float gap = prevProgress - progress;

            minGap = std::min(minGap, gap);
            maxGap = std::max(maxGap, gap);
        }
        prevProgress = progress;
        printStandingLine(ctx, entry, row);
    }
    printStabilityStats(ctx, minGap, maxGap);
}

void RaceSimDebug::printStabilityStats(
    const SimContext &ctx, float minGap, float maxGap)
{
    std::printf(
        "  Stabilite contacts : dist_min_paires=%.2fu "
        "paires_frames_contact=%d paires_frames_interpen=%d\n",
        ctx.minPairDist, ctx.contactPairFrames, ctx.overlapPairFrames);
    std::printf(
        "  Ecarts consecutifs au classement : min=%.0fu max=%.0fu\n",
        minGap, maxGap);
}

void RaceSimDebug::printStandings(SimContext &ctx)
{
    bool finished = ctx.race.phase() == racer::RacePhase::FINISHED;
    std::vector<int> order = ctx.race.standings();
    const racer::RacerEntry &leader =
        ctx.race.racers()[static_cast<size_t>(order[0])];
    float leaderProgress = totalProgress(ctx.race.getTrack(), leader);

    std::printf(
        "-- Course terminee : %s apres %.1fs simulees --\n",
        finished ? "oui" : "NON (cutoff!)",
        static_cast<float>(ctx.step) * kDt);
    printStandingRows(ctx, order, leaderProgress);
}

bool RaceSimDebug::evaluateHealth(const SimContext &ctx)
{
    bool finished = ctx.race.phase() == racer::RacePhase::FINISHED;
    bool anyFrozen = false;

    for (const auto &entry : ctx.race.racers()) {
        if (!entry.finished && std::fabs(entry.car.speed()) < 0.5f)
            anyFrozen = true;
    }
    bool ok = finished && !ctx.earlyFalseLap && !ctx.sawNaN && !anyFrozen
        && ctx.race.racers()[static_cast<size_t>(ctx.race.playerIndex())].lap
            >= kLaps;

    if (ctx.sawNaN)
        std::printf("  ERREUR: NaN detecte dans l'etat d'une voiture\n");
    if (ctx.earlyFalseLap)
        std::printf(
            "  ERREUR: tour comptabilise dans les 5 premieres secondes\n");
    if (anyFrozen)
        std::printf("  ERREUR: voiture figee (v~0) a la fin de la course\n");
    return ok;
}

bool RaceSimDebug::runRace(int presetIndex)
{
    const racer::TrackDef &def =
        racer::Track::presets()[static_cast<size_t>(presetIndex)];
    SimContext ctx(def, kLaps, kAiCount);

    printPresetHeader(presetIndex, def, ctx);
    simulateSteps(ctx);
    printStandings(ctx);
    bool ok = evaluateHealth(ctx);

    std::printf("  RACE_SIM[%d]: %s\n\n", presetIndex, ok ? "OK" : "ECHEC");
    return ok;
}

} // namespace

int main(int argc, char **argv)
{
    int presetCount = static_cast<int>(racer::Track::presets().size());
    bool allOk = true;

    if (argc > 1) {
        int idx = std::atoi(argv[1]);

        if (idx < 0 || idx >= presetCount) {
            std::printf(
                "Index de preset invalide : %d (0..%d)\n", idx,
                presetCount - 1);
            return 1;
        }
        allOk = RaceSimDebug::runRace(idx);
    } else {
        for (int i = 0; i < presetCount; ++i)
            allOk = RaceSimDebug::runRace(i) && allOk;
    }
    std::printf("RACE_SIM_GLOBAL: %s\n", allOk ? "OK" : "ECHEC");
    return allOk ? 0 : 1;
}
