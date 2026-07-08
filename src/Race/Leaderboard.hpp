/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Persistent best-lap / best-race time tracking per circuit
*/

#ifndef RACE_LEADERBOARD_HPP_
#define RACE_LEADERBOARD_HPP_

#include <map>
#include <string>
#include <vector>

namespace racer::race {

// Save file format: mirrors racer::save::SaveSystem's simple line-based
// "key=value" text convention (no third-party JSON dep), one file for all
// circuits: saves/leaderboard.sav (relative to the executable's working
// directory). Each circuit keeps a small sorted list of its best lap times
// and best race (total) times, capped at kMaxEntriesPerCircuit so the file
// stays bounded.
//
//   version=1
//   lap_<sanitizedCircuit>_<idx>=<seconds>
//   race_<sanitizedCircuit>_<idx>=<seconds>
//
// Entries within a circuit are stored ascending (idx 0 = best). Circuit
// names are sanitized the same way on write and lookup, so callers can
// pass the raw TrackDef::name / preset name directly.
class Leaderboard {
public:
    static constexpr int kMaxEntriesPerCircuit = 10;

    // Full relative path to the leaderboard file, e.g.
    // "saves/leaderboard.sav".
    static std::string defaultPath();

    explicit Leaderboard(std::string path = defaultPath());

    // Loads entries from disk into memory, replacing any current state.
    // Returns false (and resets to empty) if the file is missing or
    // unparseable -- never throws.
    bool load();

    // Writes the current in-memory state to disk. Creates the save
    // directory if needed. Returns false on I/O failure.
    bool save() const;

    // Records a completed lap/race time for a circuit, inserting it into
    // the sorted top-N list if it qualifies (kept in memory only -- call
    // save() to persist). Non-positive times are ignored.
    void recordLap(const std::string &circuit, float lapTimeSeconds);
    void recordRace(const std::string &circuit, float raceTimeSeconds);

    // Best (lowest) known time for a circuit, or -1.0f if none recorded.
    float bestLapTime(const std::string &circuit) const;
    float bestRaceTime(const std::string &circuit) const;

    // Top N times (ascending, best first) for a circuit. May return fewer
    // than N if not enough entries exist.
    std::vector<float> topLapTimes(const std::string &circuit, int n) const;
    std::vector<float> topRaceTimes(const std::string &circuit, int n) const;

    const std::string &path() const { return path_; }

private:
    std::string path_;
    std::map<std::string, std::vector<float>> lapTimes_;
    std::map<std::string, std::vector<float>> raceTimes_;

    static void insertSorted(
        std::vector<float> &times, float value, int cap);
    static std::vector<float> topN(
        const std::map<std::string, std::vector<float>> &table,
        const std::string &circuit, int n);
    static float bestOf(
        const std::map<std::string, std::vector<float>> &table,
        const std::string &circuit);
};

} // namespace racer::race

#endif /* !RACE_LEADERBOARD_HPP_ */
