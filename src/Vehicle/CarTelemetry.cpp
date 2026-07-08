/*
** EPITECH PROJECT, 2026
** racer
** File description:
** CarTelemetry implementation
*/

#include "Vehicle/CarTelemetry.hpp"

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <filesystem>

namespace racer {

namespace {

bool g_enabled = false;
float g_accum = CarTelemetry::kSampleInterval;
float g_clock = 0.0f;
std::vector<TelemetrySample> g_samples;

bool envRequestsTelemetry()
{
    const char *value = std::getenv("RACER_TELEMETRY");

    return value != nullptr && value[0] == '1';
}

} // namespace

void CarTelemetry::setEnabled(bool enabled)
{
    g_enabled = enabled;
}

bool CarTelemetry::isEnabled()
{
    // Lazily pick up the env var the first time anyone asks, so a headless
    // tool can enable logging without any source change -- set
    // RACER_TELEMETRY=1 in the environment before launching it.
    static bool checkedEnv = false;

    if (!checkedEnv) {
        checkedEnv = true;
        if (envRequestsTelemetry())
            g_enabled = true;
    }
    return g_enabled;
}

void CarTelemetry::record(float dt, float speed, float lateralG,
    float throttle, float steer, bool handbrake, float surfaceGrip,
    float surfaceDrag, bool drifting, bool airborne)
{
    if (!isEnabled())
        return;
    g_clock += dt;
    g_accum += dt;
    if (g_accum < kSampleInterval)
        return;
    g_accum = 0.0f;

    TelemetrySample sample;
    sample.time = g_clock;
    sample.speed = speed;
    sample.lateralG = lateralG;
    sample.throttle = throttle;
    sample.steer = steer;
    sample.handbrake = handbrake;
    sample.surfaceGrip = surfaceGrip;
    sample.surfaceDrag = surfaceDrag;
    sample.drifting = drifting;
    sample.airborne = airborne;
    g_samples.push_back(sample);
}

const std::vector<TelemetrySample> &CarTelemetry::samples()
{
    return g_samples;
}

void CarTelemetry::clear()
{
    g_samples.clear();
    g_accum = kSampleInterval;
    g_clock = 0.0f;
}

std::string CarTelemetry::makeSessionPath()
{
    std::filesystem::create_directories("telemetry");

    std::time_t now = std::time(nullptr);
    char stamp[32];
    std::snprintf(stamp, sizeof(stamp), "%lld",
        static_cast<long long>(now));
    return std::string("telemetry/session_") + stamp + ".csv";
}

void CarTelemetry::flushToFile()
{
    if (g_samples.empty())
        return;

    std::string path = makeSessionPath();
    std::FILE *file = std::fopen(path.c_str(), "w");

    if (file == nullptr) {
        std::fprintf(stderr, "CarTelemetry: unable to open %s for write\n",
            path.c_str());
        return;
    }
    std::fprintf(file,
        "time_s,speed,lateral_g,throttle,steer,handbrake,surface_grip,"
        "surface_drag,drifting,airborne\n");
    for (const TelemetrySample &s : g_samples) {
        std::fprintf(file, "%.3f,%.4f,%.4f,%.3f,%.3f,%d,%.3f,%.3f,%d,%d\n",
            s.time, s.speed, s.lateralG, s.throttle, s.steer,
            s.handbrake ? 1 : 0, s.surfaceGrip, s.surfaceDrag,
            s.drifting ? 1 : 0, s.airborne ? 1 : 0);
    }
    std::fclose(file);
    std::printf("CarTelemetry: wrote %zu samples to %s\n", g_samples.size(),
        path.c_str());
    g_samples.clear();
}

} // namespace racer
