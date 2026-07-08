/*
** EPITECH PROJECT, 2026
** racer
** File description:
** ReplayRecorder implementation
*/

#include "Race/ReplayRecorder.hpp"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace racer::race {

namespace {

constexpr const char *kReplayDir = "saves";
constexpr int kFormatVersion = 1;

bool isSafeChar(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
        || (c >= '0' && c <= '9') || c == '_' || c == '-';
}

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

ReplayRecorder::ReplayRecorder(float sampleIntervalSeconds)
    : sampleInterval_(
        sampleIntervalSeconds > 0.0f ? sampleIntervalSeconds
                                      : kDefaultSampleInterval)
{
}

void ReplayRecorder::start()
{
    samples_.clear();
    accumulator_ = 0.0f;
    elapsed_ = 0.0f;
    active_ = true;
}

void ReplayRecorder::stop()
{
    active_ = false;
}

void ReplayRecorder::update(
    float dt, float x, float z, float heading, float speed)
{
    if (!active_) {
        return;
    }
    elapsed_ += dt;
    accumulator_ += dt;
    if (accumulator_ < sampleInterval_) {
        return;
    }
    accumulator_ -= sampleInterval_;
    samples_.push_back(ReplaySample{elapsed_, x, z, heading, speed});
}

std::string ReplayRecorder::pathForCircuit(const std::string &circuit)
{
    return std::string(kReplayDir) + "/replay_" + sanitize(circuit) + ".rep";
}

bool ReplayRecorder::save(const std::string &path) const
{
    std::error_code ec;
    std::filesystem::create_directories(kReplayDir, ec);
    if (ec) {
        return false;
    }

    std::ofstream out(path, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        return false;
    }

    // Full float precision (9 significant digits round-trips exactly)
    // so reloaded samples match the recorded ones bit-for-bit-ish.
    out << std::setprecision(9);
    out << "version=" << kFormatVersion << "\n";
    out << "count=" << samples_.size() << "\n";
    for (const auto &s : samples_) {
        out << "sample=" << s.time << "," << s.x << "," << s.z << ","
            << s.heading << "," << s.speed << "\n";
    }

    out.close();
    return !out.fail();
}

bool ReplayRecorder::load(
    const std::string &path, std::vector<ReplaySample> &outSamples)
{
    outSamples.clear();

    std::ifstream in(path);
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

        if (key == "version") {
            sawVersion = true;
            continue;
        }
        if (key == "count") {
            continue; // informational only; vector grows as parsed
        }
        if (key != "sample") {
            continue;
        }

        std::istringstream ss(value);
        std::string field;
        ReplaySample sample;
        try {
            if (!std::getline(ss, field, ',')) continue;
            sample.time = std::stof(field);
            if (!std::getline(ss, field, ',')) continue;
            sample.x = std::stof(field);
            if (!std::getline(ss, field, ',')) continue;
            sample.z = std::stof(field);
            if (!std::getline(ss, field, ',')) continue;
            sample.heading = std::stof(field);
            if (!std::getline(ss, field, ',')) continue;
            sample.speed = std::stof(field);
        } catch (const std::exception &) {
            continue;
        }
        outSamples.push_back(sample);
    }

    if (!sawVersion) {
        outSamples.clear();
        return false;
    }
    return true;
}

} // namespace racer::race
