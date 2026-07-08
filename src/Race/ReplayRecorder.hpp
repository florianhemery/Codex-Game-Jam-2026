/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Lightweight fixed-interval capture of the player car's trajectory,
** enough to play back a "ghost" car later (rendering is a future task --
** this only captures/stores/reloads the data).
*/

#ifndef RACE_REPLAY_RECORDER_HPP_
#define RACE_REPLAY_RECORDER_HPP_

#include <string>
#include <vector>

namespace racer::race {

// One timestamped sample of the player car's pose/speed.
struct ReplaySample {
    float time = 0.0f;
    float x = 0.0f;
    float z = 0.0f;
    float heading = 0.0f;
    float speed = 0.0f;
};

// Captures the player's trajectory at a fixed sample interval (default
// 0.05s) from race start to finish. Decoupled from RaceState/GameLoop:
// callers feed it dt + the player's current pose every frame via
// update(), and call start()/stop() at the natural race-lifecycle edges.
//
// File format: simple line-based text (consistent with the project's
// other plain-text save files, e.g. racer::save::SaveSystem), one file
// per circuit at saves/replay_<circuit>.rep:
//
//   version=1
//   count=<N>
//   sample=<time>,<x>,<z>,<heading>,<speed>   (repeated N times)
class ReplayRecorder {
public:
    static constexpr float kDefaultSampleInterval = 0.05f;

    explicit ReplayRecorder(float sampleIntervalSeconds = kDefaultSampleInterval);

    // Begins a new capture: clears any previously recorded samples and
    // resets the internal clock.
    void start();

    // Ends the capture (further update() calls are ignored until the
    // next start()).
    void stop();

    bool active() const { return active_; }

    // Feeds one frame of simulation. Appends a new sample whenever the
    // accumulated time crosses the sample interval. No-op if !active().
    void update(float dt, float x, float z, float heading, float speed);

    const std::vector<ReplaySample> &samples() const { return samples_; }

    // Full relative path for a circuit's replay file, e.g.
    // "saves/replay_MyTrack.rep".
    static std::string pathForCircuit(const std::string &circuit);

    // Writes the currently recorded samples to `path`. Creates the save
    // directory if needed. Returns false on I/O failure.
    bool save(const std::string &path) const;

    // Reloads samples from `path` into `outSamples` (replacing its
    // contents). Returns false if the file is missing/unparseable, in
    // which case outSamples is left empty.
    static bool load(
        const std::string &path, std::vector<ReplaySample> &outSamples);

private:
    float sampleInterval_;
    float accumulator_ = 0.0f;
    float elapsed_ = 0.0f;
    bool active_ = false;
    std::vector<ReplaySample> samples_;
};

} // namespace racer::race

#endif /* !RACE_REPLAY_RECORDER_HPP_ */
