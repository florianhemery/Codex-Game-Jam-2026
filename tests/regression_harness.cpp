/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Headless driving-feel / world-state regression harness
**
** Unlike world_chunk_test (which samples the terrain statically), this
** harness actually drives the player car through the open world via
** PlayerDriveSystem -- the exact runtime path used by the game -- and
** watches the resulting trajectory for the two classes of bug this
** session already hit once each: a road-height algorithm regression
** that silently turns a road into a cliff/canyon, and a physics/clamp
** regression that lets the car teleport, NaN out, leave the playable
** bounds, or run at an implausible speed. A pure "does it compile and
** the static height tests pass" check would not catch either, because
** both showed up only once the car was actually moving through the
** world frame by frame.
*/

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "Vehicle/Car.hpp"
#include "Vehicle/CarInput.hpp"
#include "World/Aurelia/AureliaBounds.hpp"
#include "World/Aurelia/AureliaData.hpp"
#include "World/Road/RoadGraph.hpp"
#include "World/Sim/PlayerDriveSystem.hpp"
#include "World/Stream/ChunkStreamer.hpp"

namespace {

int g_failures = 0;

void expectTrue(bool cond, const char *msg)
{
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s\n", msg);
        ++g_failures;
    }
}

constexpr float kDt = 1.0f / 60.0f;
// At the car's tuned top speed (~28 u/s, see CarTuning::maxSpeed) a single
// frame at kDt covers well under 1m of horizontal travel. A height jump of
// several meters in one frame -- regardless of direction of travel -- means
// the ground the wheels are glued to (PlayerDriveSystem snaps car.y to
// sampleHeight() every frame) teleported, which is exactly what an
// algorithm bug that carves a canyon or a cliff under a road produces.
// Legitimate steep terrain-following (volcano rim, etc.) still moves the
// car gradually frame to frame, so this threshold has ample margin without
// being blind to a real regression.
constexpr float kMaxYJumpPerFrame = 3.0f;
constexpr float kArrivalRadius = 6.0f;
constexpr float kSteerGain = 2.2f;

struct DriveStats {
    bool sawNaN = false;
    bool sawCliff = false;
    bool sawOutOfBounds = false;
    double speedSum = 0.0;
    int frames = 0;
};

bool isFiniteCar(const racer::Car &car)
{
    return std::isfinite(car.position().x) && std::isfinite(car.position().y)
        && std::isfinite(car.position().z) && std::isfinite(car.speed())
        && std::isfinite(car.heading());
}

float normalizeAngle(float angle)
{
    while (angle > PI)
        angle -= 2.0f * PI;
    while (angle < -PI)
        angle += 2.0f * PI;
    return angle;
}

racer::CarInput steerToward(const racer::Car &car, Vector2 target)
{
    float dx = target.x - car.position().x;
    float dz = target.y - car.position().z;
    float desiredHeading = std::atan2(dx, dz);
    float error = normalizeAngle(desiredHeading - car.heading());

    racer::CarInput input;
    input.steer = std::clamp(error * kSteerGain, -1.0f, 1.0f);
    input.throttle = 0.85f;
    return input;
}

// Drives `car` toward `target` for up to `maxSteps` frames, feeding every
// sample through the invariant checks. Stops early once the car arrives
// within kArrivalRadius of the target. Returns false if the step budget
// was exhausted without arriving (a "car got stuck" style regression).
bool driveToward(racer::Car &car, racer::world::ChunkStreamer &streamer,
    racer::world::PlayerDriveSystem &drive, Vector2 target, int maxSteps,
    DriveStats &stats, bool checkBounds)
{
    bool hasPrevY = false;
    float prevY = 0.0f;

    for (int step = 0; step < maxSteps; ++step) {
        racer::CarInput input = steerToward(car, target);
        float wheelSpin = 0.0f;

        streamer.updateCenter(car.position().x, car.position().z);
        streamer.ensureLoaded();
        drive.update(car, input, 0.0f, kDt, streamer, wheelSpin);

        if (!isFiniteCar(car)) {
            stats.sawNaN = true;
            return false;
        }
        if (checkBounds) {
            const float margin = 0.5f;
            bool inBounds = car.position().x
                    >= racer::world::WorldBounds::minX - margin
                && car.position().x
                    <= racer::world::WorldBounds::maxX + margin
                && car.position().z
                    >= racer::world::WorldBounds::minZ - margin
                && car.position().z
                    <= racer::world::WorldBounds::maxZ + margin;
            if (!inBounds)
                stats.sawOutOfBounds = true;
        }
        if (hasPrevY) {
            if (std::fabs(car.position().y - prevY) > kMaxYJumpPerFrame)
                stats.sawCliff = true;
        }
        prevY = car.position().y;
        hasPrevY = true;
        stats.speedSum += std::fabs(car.speed());
        ++stats.frames;

        float dx = target.x - car.position().x;
        float dz = target.y - car.position().z;
        if (std::sqrt(dx * dx + dz * dz) <= kArrivalRadius)
            return true;
    }
    return false;
}

// Drives the player car along every edge of the Aurelia road network, one
// leg at a time (resetting to the leg's start node between edges -- that
// reset is an intentional new-leg placement, not a mid-drive teleport, so
// it is excluded from the per-frame height-jump check via hasPrevY).
void testRoadNetworkDriveSanity()
{
    using racer::world::AureliaData;
    using racer::world::ChunkStreamer;
    using racer::world::PlayerDriveSystem;

    const auto &graph = AureliaData::roadGraph();
    expectTrue(!graph.nodes().empty(), "road graph has nodes to drive");

    ChunkStreamer streamer;
    PlayerDriveSystem drive;
    racer::Car car;
    DriveStats stats;
    int stuckLegs = 0;

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
        // Generous budget: even crawling at 3 u/s the leg should finish
        // well inside this many frames; running out means the car is
        // stuck (a different, but equally real, driving-feel regression).
        int maxSteps = static_cast<int>((edgeLen / 3.0f) * (1.0f / kDt))
            + 600;
        bool arrived
            = driveToward(car, streamer, drive, to, maxSteps, stats, true);
        if (!arrived && !stats.sawNaN)
            ++stuckLegs;
        if (stats.sawNaN)
            break;
    }

    expectTrue(!stats.sawNaN,
        "player car position/speed/heading stays finite while driving "
        "the road network");
    expectTrue(!stats.sawCliff,
        "player car height never jumps more than kMaxYJumpPerFrame "
        "between consecutive frames while driving the road network "
        "(catches a canyon/cliff road-height regression)");
    expectTrue(!stats.sawOutOfBounds,
        "player car stays within WorldBounds (soft-clamped) while "
        "driving the road network");
    expectTrue(stuckLegs == 0,
        "player car reaches every road-graph waypoint instead of getting "
        "stuck");

    float avgSpeed = stats.frames > 0
        ? static_cast<float>(stats.speedSum / stats.frames)
        : 0.0f;
    expectTrue(stats.frames > 200,
        "road network drive ran a meaningful number of frames");
    expectTrue(avgSpeed > 1.5f,
        "average speed on the road network is not near-zero (catches a "
        "frozen/broken-input regression)");
    expectTrue(avgSpeed < 40.0f,
        "average speed on the road network stays within a plausible "
        "range for CarTuning::maxSpeed (catches a runaway physics "
        "regression)");
}

// Drives straight off-road from the marina spawn toward the world edge,
// covering terrain that isn't necessarily under a painted road -- a height
// algorithm bug is not guaranteed to be road-specific, so this is a
// complementary sweep of open terrain rather than a duplicate of the
// road-network test above.
void testOffRoadStraightLineSanity()
{
    using racer::world::AureliaData;
    using racer::world::ChunkStreamer;
    using racer::world::PlayerDriveSystem;
    using racer::world::WorldBounds;

    ChunkStreamer streamer;
    PlayerDriveSystem drive;
    racer::Car car;
    DriveStats stats;

    const auto &graph = AureliaData::roadGraph();
    Vector2 spawn = graph.nodes().empty()
        ? Vector2{WorldBounds::centerX, WorldBounds::centerZ}
        : graph.nodes()[0].position;

    streamer.updateCenter(spawn.x, spawn.y);
    streamer.ensureLoaded();
    float startY = streamer.sampleHeight(spawn.x, spawn.y);
    car.position() = Vector3{spawn.x, startY + 0.06f, spawn.y};
    drive.reset(car, spawn.x, spawn.y, 0.0f);

    // Aim well past the world edge -- softClampCar is expected to hold the
    // car inside WorldBounds, and this run verifies that holds true while
    // crossing open terrain, not just on the road network.
    Vector2 farTarget{WorldBounds::centerX, WorldBounds::maxZ + 200.0f};
    int maxSteps = static_cast<int>(30.0f * (1.0f / kDt));
    driveToward(car, streamer, drive, farTarget, maxSteps, stats, true);

    expectTrue(!stats.sawNaN,
        "player car stays finite while driving straight across open "
        "(non-road) terrain");
    expectTrue(!stats.sawCliff,
        "terrain height never jumps more than kMaxYJumpPerFrame between "
        "consecutive frames off-road (catches a canyon regression that "
        "isn't limited to painted roads)");
    expectTrue(!stats.sawOutOfBounds,
        "player car is held within WorldBounds by the soft clamp even "
        "when driven straight at the world edge");
}

} // namespace

int main()
{
    testRoadNetworkDriveSanity();
    testOffRoadStraightLineSanity();

    if (g_failures > 0) {
        std::fprintf(stderr, "%d regression check(s) failed\n", g_failures);
        return 1;
    }
    std::printf("OK: all regression harness checks passed\n");
    return 0;
}
