/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Console output for headless race simulation
*/

#include <algorithm>
#include <cstdio>

#include "Tools/RaceSimPrinter.hpp"

void RaceSimPrinter::printPresetHeader(
    int presetIndex, const racer::TrackDef &def,
    const RaceSimDebug::SimContext &ctx)
{
    const char *surface = def.surfaceStyle == racer::SurfaceStyle::ABIMEE
        ? "abimee" : "propre";

    std::printf(
        "=== Preset %d : %s (surface %s, longueur %.0f u) ===\n",
        presetIndex, def.name.c_str(), surface,
        ctx.race.getTrack().totalLength());
}

void RaceSimPrinter::printProgressLog(const RaceSimDebug::SimContext &ctx)
{
    std::printf("t=%5.1fs  ", static_cast<float>(ctx.step) * kRaceSimDt);
    for (const auto &entry : ctx.race.racers()) {
        std::printf(
            "%s[lap=%d,v=%4.1f] ", entry.name.c_str(), entry.lap,
            entry.car.speed());
    }
    std::printf("pos_joueur=%d\n", ctx.race.playerPosition());
}

void RaceSimPrinter::printStandingLine(
    const RaceSimDebug::SimContext &ctx, const racer::RacerEntry &entry,
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

void RaceSimPrinter::printStabilityStats(
    const RaceSimDebug::SimContext &ctx, float minGap, float maxGap)
{
    std::printf(
        "  Stabilite contacts : dist_min_paires=%.2fu "
        "paires_frames_contact=%d paires_frames_interpen=%d\n",
        ctx.minPairDist, ctx.contactPairFrames, ctx.overlapPairFrames);
    std::printf(
        "  Ecarts consecutifs au classement : min=%.0fu max=%.0fu\n",
        minGap, maxGap);
}

float RaceSimPrinter::printStandingRowAt(
    RaceSimDebug::SimContext &ctx, const std::vector<int> &order, size_t rank,
    float leaderProgress, float prevProgress, float &minGap, float &maxGap)
{
    const racer::RacerEntry &entry =
        ctx.race.racers()[static_cast<size_t>(order[rank])];
    float progress = RaceSimDebug::totalProgress(ctx.race.getTrack(), entry);
    StandingRow row{rank, order[rank], progress, leaderProgress};

    if (rank > 0) {
        float gap = prevProgress - progress;

        minGap = std::min(minGap, gap);
        maxGap = std::max(maxGap, gap);
    }
    printStandingLine(ctx, entry, row);
    return progress;
}

void RaceSimPrinter::printStandingRows(
    RaceSimDebug::SimContext &ctx, const std::vector<int> &order,
    float leaderProgress)
{
    float minGap = 1e9f;
    float maxGap = 0.0f;
    float prevProgress = leaderProgress;

    for (size_t p = 0; p < order.size(); ++p) {
        prevProgress = printStandingRowAt(
            ctx, order, p, leaderProgress, prevProgress, minGap, maxGap);
    }
    printStabilityStats(ctx, minGap, maxGap);
}

void RaceSimPrinter::printStandings(RaceSimDebug::SimContext &ctx)
{
    bool finished = ctx.race.phase() == racer::RacePhase::FINISHED;
    std::vector<int> order = ctx.race.standings();
    const racer::RacerEntry &leader =
        ctx.race.racers()[static_cast<size_t>(order[0])];
    float leaderProgress =
        RaceSimDebug::totalProgress(ctx.race.getTrack(), leader);

    std::printf(
        "-- Course terminee : %s apres %.1fs simulees --\n",
        finished ? "oui" : "NON (cutoff!)",
        static_cast<float>(ctx.step) * kRaceSimDt);
    printStandingRows(ctx, order, leaderProgress);
}

void RaceSimPrinter::printHealthErrors(const RaceSimDebug::SimContext &ctx)
{
    if (ctx.sawNaN)
        std::printf("  ERREUR: NaN detecte dans l'etat d'une voiture\n");
    if (ctx.earlyFalseLap)
        std::printf(
            "  ERREUR: tour comptabilise dans les 5 premieres secondes\n");
    if (RaceSimDebug::anyFrozenCar(ctx))
        std::printf("  ERREUR: voiture figee (v~0) a la fin de la course\n");
}

void RaceSimPrinter::printFinalVerdict(int presetIndex, bool ok)
{
    std::printf("  RACE_SIM[%d]: %s\n\n", presetIndex, ok ? "OK" : "ECHEC");
}
