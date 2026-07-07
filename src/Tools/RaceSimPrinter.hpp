/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Console output for headless race simulation
*/

#ifndef RACE_SIM_PRINTER_HPP_
#define RACE_SIM_PRINTER_HPP_

#include <vector>

#include "Tools/RaceSimDebug.hpp"

class RaceSimPrinter {
public:
    struct StandingRow {
        size_t rank = 0;
        int racerIndex = 0;
        float progress = 0.0f;
        float leaderProgress = 0.0f;
    };

    static void printPresetHeader(
        int presetIndex, const racer::TrackDef &def,
        const RaceSimDebug::SimContext &ctx);
    static void printProgressLog(const RaceSimDebug::SimContext &ctx);
    static void printStandings(RaceSimDebug::SimContext &ctx);
    static void printHealthErrors(const RaceSimDebug::SimContext &ctx);
    static void printFinalVerdict(int presetIndex, bool ok);

private:
    static void printStandingLine(
        const RaceSimDebug::SimContext &ctx, const racer::RacerEntry &entry,
        const StandingRow &row);
    static float printStandingRowAt(
        RaceSimDebug::SimContext &ctx, const std::vector<int> &order,
        size_t rank, float leaderProgress, float prevProgress, float &minGap,
        float &maxGap);
    static void printStandingRows(
        RaceSimDebug::SimContext &ctx, const std::vector<int> &order,
        float leaderProgress);
    static void printStabilityStats(
        const RaceSimDebug::SimContext &ctx, float minGap, float maxGap);
};

#endif /* !RACE_SIM_PRINTER_HPP_ */
