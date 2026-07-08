/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Opt-in per-frame telemetry logger for vehicle feel tuning
**
** Disabled by default (near-zero cost: one bool check per call) so it
** never affects normal play or any other target. Enable it either by
** calling CarTelemetry::setEnabled(true) from tooling code, or by setting
** the RACER_TELEMETRY=1 environment variable before launching a binary
** that drives a Car through PlayerDriveSystem. Samples are buffered in
** memory and flushed to telemetry/session_<timestamp>.csv on request or
** at process exit, so a long drive session produces exactly one file.
*/

#ifndef CAR_TELEMETRY_HPP_
#define CAR_TELEMETRY_HPP_

#include <string>
#include <vector>

namespace racer {

struct TelemetrySample {
    float time = 0.0f;
    float speed = 0.0f;
    float lateralG = 0.0f;
    float throttle = 0.0f;
    float steer = 0.0f;
    bool handbrake = false;
    float surfaceGrip = 1.0f;
    float surfaceDrag = 1.0f;
    bool drifting = false;
    bool airborne = false;
};

// Fixed-interval sampler: accumulates dt and only records a sample once
// kSampleInterval has elapsed, so the log stays readable (~10 samples/s)
// regardless of the simulation's actual frame rate.
class CarTelemetry {
public:
    static constexpr float kSampleInterval = 0.1f;

    static void setEnabled(bool enabled);
    static bool isEnabled();

    // Call once per simulation frame with the values PlayerDriveSystem
    // already has on hand; internally throttled to kSampleInterval.
    static void record(float dt, float speed, float lateralG, float throttle,
        float steer, bool handbrake, float surfaceGrip, float surfaceDrag,
        bool drifting, bool airborne);

    // Writes every buffered sample to telemetry/session_<timestamp>.csv
    // and clears the buffer. Safe to call with an empty buffer (no-op).
    static void flushToFile();

    static const std::vector<TelemetrySample> &samples();
    static void clear();

private:
    static std::string makeSessionPath();
};

} // namespace racer

#endif /* !CAR_TELEMETRY_HPP_ */
