/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Leaderboard implementation
*/

#include "Race/Leaderboard.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>

namespace racer::race {

namespace {

constexpr const char *kLeaderboardDir = "saves";
constexpr const char *kLeaderboardFile = "saves/leaderboard.sav";
constexpr int kFormatVersion = 1;

bool isSafeChar(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
        || (c >= '0' && c <= '9') || c == '_' || c == '-';
}

// Keeps circuit names safe to embed inside a "key=value" line: only
// alphanumeric/underscore/hyphen survive, everything else (spaces,
// accents, '=') collapses to '_'. Used identically on write and lookup
// so recordLap("Piste Cote", ...) and bestLapTime("Piste Cote") agree.
std::string sanitize(const std::string &name)
{
    std::string safe;
    safe.reserve(name.size());
    for (char c : name) {
        safe.push_back(isSafeChar(c) ? c : '_');
    }
    if (safe.empty()) {
        safe = "unknown";
    }
    return safe;
}

} // namespace

std::string Leaderboard::defaultPath()
{
    return kLeaderboardFile;
}

Leaderboard::Leaderboard(std::string path) : path_(std::move(path)) {}

void Leaderboard::insertSorted(std::vector<float> &times, float value, int cap)
{
    auto it = std::lower_bound(times.begin(), times.end(), value);
    times.insert(it, value);
    if (static_cast<int>(times.size()) > cap) {
        times.resize(static_cast<size_t>(cap));
    }
}

void Leaderboard::recordLap(const std::string &circuit, float lapTimeSeconds)
{
    if (lapTimeSeconds <= 0.0f) {
        return;
    }
    insertSorted(
        lapTimes_[sanitize(circuit)], lapTimeSeconds, kMaxEntriesPerCircuit);
}

void Leaderboard::recordRace(const std::string &circuit, float raceTimeSeconds)
{
    if (raceTimeSeconds <= 0.0f) {
        return;
    }
    insertSorted(
        raceTimes_[sanitize(circuit)], raceTimeSeconds, kMaxEntriesPerCircuit);
}

float Leaderboard::bestOf(
    const std::map<std::string, std::vector<float>> &table,
    const std::string &circuit)
{
    auto it = table.find(sanitize(circuit));
    if (it == table.end() || it->second.empty()) {
        return -1.0f;
    }
    return it->second.front();
}

float Leaderboard::bestLapTime(const std::string &circuit) const
{
    return bestOf(lapTimes_, circuit);
}

float Leaderboard::bestRaceTime(const std::string &circuit) const
{
    return bestOf(raceTimes_, circuit);
}

std::vector<float> Leaderboard::topN(
    const std::map<std::string, std::vector<float>> &table,
    const std::string &circuit, int n)
{
    auto it = table.find(sanitize(circuit));
    if (it == table.end()) {
        return {};
    }
    int count = std::min(n, static_cast<int>(it->second.size()));
    if (count <= 0) {
        return {};
    }
    return std::vector<float>(
        it->second.begin(), it->second.begin() + count);
}

std::vector<float> Leaderboard::topLapTimes(
    const std::string &circuit, int n) const
{
    return topN(lapTimes_, circuit, n);
}

std::vector<float> Leaderboard::topRaceTimes(
    const std::string &circuit, int n) const
{
    return topN(raceTimes_, circuit, n);
}

bool Leaderboard::save() const
{
    std::error_code ec;
    std::filesystem::create_directories(kLeaderboardDir, ec);
    if (ec) {
        return false;
    }

    std::ofstream out(path_, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        return false;
    }

    out << std::setprecision(9);
    out << "version=" << kFormatVersion << "\n";
    for (const auto &[circuit, times] : lapTimes_) {
        for (size_t i = 0; i < times.size(); ++i) {
            out << "lap_" << circuit << "_" << i << "=" << times[i] << "\n";
        }
    }
    for (const auto &[circuit, times] : raceTimes_) {
        for (size_t i = 0; i < times.size(); ++i) {
            out << "race_" << circuit << "_" << i << "=" << times[i] << "\n";
        }
    }

    out.close();
    return !out.fail();
}

namespace {

// Splits a key like "lap_MyTrack_3" (prefix already stripped, so we get
// "MyTrack_3") into {"MyTrack", 3} by finding the last '_'-separated
// numeric suffix. Returns false if no numeric suffix is present.
bool splitCircuitIndex(const std::string &rest, std::string &circuit, int &idx)
{
    size_t pos = rest.rfind('_');
    if (pos == std::string::npos || pos + 1 >= rest.size()) {
        return false;
    }
    std::string idxStr = rest.substr(pos + 1);
    for (char c : idxStr) {
        if (c < '0' || c > '9') {
            return false;
        }
    }
    circuit = rest.substr(0, pos);
    try {
        idx = std::stoi(idxStr);
    } catch (const std::exception &) {
        return false;
    }
    return true;
}

} // namespace

bool Leaderboard::load()
{
    lapTimes_.clear();
    raceTimes_.clear();

    std::ifstream in(path_);
    if (!in.is_open()) {
        return false;
    }

    bool sawVersion = false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) {
            continue;
        }
        size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);
        if (key.empty() || value.empty()) {
            continue;
        }

        try {
            if (key == "version") {
                sawVersion = true;
            } else if (key.rfind("lap_", 0) == 0) {
                std::string circuit;
                int idx = 0;
                if (!splitCircuitIndex(key.substr(4), circuit, idx)) {
                    continue;
                }
                lapTimes_[circuit].push_back(std::stof(value));
            } else if (key.rfind("race_", 0) == 0) {
                std::string circuit;
                int idx = 0;
                if (!splitCircuitIndex(key.substr(5), circuit, idx)) {
                    continue;
                }
                raceTimes_[circuit].push_back(std::stof(value));
            }
        } catch (const std::exception &) {
            continue;
        }
    }

    // Entries were appended in file order (already ascending, since
    // save() writes idx 0..N-1 in sorted order), but re-sort defensively
    // in case of hand-edited/corrupt files.
    for (auto &[circuit, times] : lapTimes_) {
        std::sort(times.begin(), times.end());
        if (times.size() > static_cast<size_t>(kMaxEntriesPerCircuit)) {
            times.resize(static_cast<size_t>(kMaxEntriesPerCircuit));
        }
        (void)circuit;
    }
    for (auto &[circuit, times] : raceTimes_) {
        std::sort(times.begin(), times.end());
        if (times.size() > static_cast<size_t>(kMaxEntriesPerCircuit)) {
            times.resize(static_cast<size_t>(kMaxEntriesPerCircuit));
        }
        (void)circuit;
    }

    if (!sawVersion) {
        lapTimes_.clear();
        raceTimes_.clear();
        return false;
    }
    return true;
}

} // namespace racer::race
