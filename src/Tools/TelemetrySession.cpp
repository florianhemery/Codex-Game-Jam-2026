/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Headless tool that drives the player car through Aurelia via
** PlayerDriveSystem -- the exact runtime path used by the game -- with
** CarTelemetry recording enabled, to produce a real per-frame CSV log
** for handling/feel tuning decisions instead of guesswork. Reuses the
** same "steer toward a target waypoint" driving loop already proven out
** in tests/regression_harness.cpp, but its purpose is telemetry capture,
** not pass/fail regression checking.
*/

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "Vehicle/Car.hpp"
#include "Vehicle/CarInput.hpp"
#include "Vehicle/CarTelemetry.hpp"
#include "World/Aurelia/AureliaBounds.hpp"
#include "World/Aurelia/AureliaData.hpp"
#include "World/Road/RoadGraph.hpp"
#include "World/Sim/PlayerDriveSystem.hpp"
#include "World/Stream/ChunkStreamer.hpp"

namespace {

constexpr float kDt = 1.0f / 60.0f;
constexpr float kArrivalRadius = 6.0f;
constexpr float kSteerGain = 2.2f;

float normalizeAngle(float angle)
{
    while (angle > PI)
        angle -= 2.0f * PI;
    while (angle < -PI)
        angle += 2.0f * PI;
    return angle;
}

racer::CarInput steerToward(const racer::Car &car, Vector2 target,
    bool handbrake)
{
    float dx = target.x - car.position().x;
    float dz = target.y - car.position().z;
    float desiredHeading = std::atan2(dx, dz);
    float error = normalizeAngle(desiredHeading - car.heading());

    racer::CarInput input;
    input.steer = std::clamp(error * kSteerGain, -1.0f, 1.0f);
    input.throttle = 0.9f;
    input.handbrake = handbrake;
    return input;
}

// Drives toward `target`, occasionally flicking the handbrake on sharp
// corners (large steer error) so the session captures drift behaviour too,
// not just steady cornering -- both matter for handling tuning.
void driveToward(racer::Car &car, racer::world::ChunkStreamer &streamer,
    racer::world::PlayerDriveSystem &drive, Vector2 target, int maxSteps)
{
    for (int step = 0; step < maxSteps; ++step) {
        float dx = target.x - car.position().x;
        float dz = target.y - car.position().z;
        float desiredHeading = std::atan2(dx, dz);
        float error = std::fabs(normalizeAngle(desiredHeading - car.heading()));
        bool handbrake = error > 0.9f && std::fabs(car.speed()) > 6.0f;
        racer::CarInput input = steerToward(car, target, handbrake);
        float wheelSpin = 0.0f;

        streamer.updateCenter(car.position().x, car.position().z);
        streamer.ensureLoaded();
        drive.update(car, input, 0.0f, kDt, streamer, wheelSpin);

        if (!std::isfinite(car.position().x) || !std::isfinite(car.speed()))
            return;
        if (std::sqrt(dx * dx + dz * dz) <= kArrivalRadius)
            return;
    }
}

} // namespace

int main()
{
    using racer::world::AureliaData;
    using racer::world::ChunkStreamer;
    using racer::world::PlayerDriveSystem;

    racer::CarTelemetry::setEnabled(true);
    racer::CarTelemetry::clear();

    const auto &graph = AureliaData::roadGraph();
    if (graph.nodes().empty()) {
        std::fprintf(stderr, "TelemetrySession: empty road graph\n");
        return 1;
    }

    ChunkStreamer streamer;
    PlayerDriveSystem drive;
    racer::Car car;

    // Drive every road-graph edge once (asphalt, mostly) then a long
    // straight run off-road toward the world edge (grass/sand/rock) so the
    // log captures both on-road cornering and surface transitions.
    for (size_t ei = 0; ei < graph.edges().size(); ++ei) {
        const auto &edge = graph.edges()[ei];
        Vector2 from = graph.nodes()[static_cast<size_t>(edge.from)].position;
        Vector2 to = graph.nodes()[static_cast<size_t>(edge.to)].position;

        streamer.updateCenter(from.x, from.y);
        streamer.ensureLoaded();
        float startY = streamer.sampleHeight(from.x, from.y);
        car.position() = Vector3{from.x, startY + 0.06f, from.y};
        float initialHeading = std::atan2(to.x - from.x, to.y - from.y);
        drive.reset(car, from.x, from.y, initialHeading);

        float edgeLen = std::max(graph.edgeLength(static_cast<int>(ei)), 1.0f);
        int maxSteps = static_cast<int>((edgeLen / 3.0f) * (1.0f / kDt))
            + 600;
        driveToward(car, streamer, drive, to, maxSteps);
    }

    using racer::world::WorldBounds;
    Vector2 offRoadTarget{WorldBounds::centerX, WorldBounds::maxZ + 200.0f};
    driveToward(car, streamer, drive, offRoadTarget,
        static_cast<int>(45.0f * (1.0f / kDt)));

    std::printf("TelemetrySession: captured %zu samples\n",
        racer::CarTelemetry::samples().size());
    racer::CarTelemetry::flushToFile();
    return 0;
}
