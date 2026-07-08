/*

** EPITECH PROJECT, 2026

** racer

** File description:

** TrafficSystem implementation

*/



#include "World/Sim/TrafficSystem.hpp"



#include <algorithm>

#include <cmath>



namespace racer::world {



namespace {



constexpr float kSpawnRadius = 140.0f;

constexpr float kDespawnRadius = 200.0f;
constexpr int kMinActiveVehicles = 2;

float distXZ(float ax, float az, float bx, float bz)

{

    float dx = ax - bx;

    float dz = az - bz;

    return std::sqrt(dx * dx + dz * dz);

}



} // namespace



TrafficSystem::TrafficSystem(const RoadGraph &graph) : graph_(graph) {}



void TrafficSystem::spawnIfNeeded(float playerX, float playerZ,

    const ChunkStreamer &streamer, int targetActiveCount)

{

    if (graph_.edges().empty()

        || static_cast<int>(vehicles_.size()) >= kMaxVehicles

        || activeCount() >= targetActiveCount) {

        return;

    }

    spawnTimer_ -= 0.016f;

    if (spawnTimer_ > 0.0f) {

        return;

    }

    spawnTimer_ = 1.2f;



    const int edgeCount = static_cast<int>(graph_.edges().size());

    int edgeIndex = static_cast<int>(vehicles_.size()) % edgeCount;

    for (int attempt = 0; attempt < edgeCount; ++attempt) {

        int candidate = (edgeIndex + attempt) % edgeCount;

        Vector2 p = graph_.pointOnEdge(candidate, 0.35f);

        if (distXZ(p.x, p.y, playerX, playerZ) > kSpawnRadius) {

            continue;

        }

        if (!streamer.isLoaded(p.x, p.y)) {

            continue;

        }

        TrafficVehicle v{};

        v.edgeIndex = candidate;

        v.t = 0.15f + 0.05f * static_cast<float>(vehicles_.size());

        v.speed = graph_.edges()[static_cast<size_t>(candidate)].speedLimit

            * 0.6f;

        v.color = Color{static_cast<unsigned char>(120 + candidate * 20),

            static_cast<unsigned char>(140),

            static_cast<unsigned char>(180), 255};

        vehicles_.push_back(v);

        return;

    }

}



void TrafficSystem::despawnFar(float playerX, float playerZ)

{

    std::vector<TrafficVehicle> kept;

    for (TrafficVehicle &v : vehicles_) {

        Vector2 p = graph_.pointOnEdge(v.edgeIndex, v.t);

        if (distXZ(p.x, p.y, playerX, playerZ) < kDespawnRadius) {

            kept.push_back(v);

        }

    }

    vehicles_ = std::move(kept);

}



int TrafficSystem::activeCount() const

{

    int count = 0;

    for (const TrafficVehicle &v : vehicles_) {

        if (v.active) {

            ++count;

        }

    }

    return count;

}



void TrafficSystem::refreshVisible()

{

    visibleVehicles_.clear();

    for (const TrafficVehicle &v : vehicles_) {

        if (v.active) {

            visibleVehicles_.push_back(v);

        }

    }

}



float TrafficSystem::computeDensityTarget(float timeOfDay, BiomeId biome)

{

    // Time-of-day shapes a rush-hour curve: quiet at night, ramping up at
    // dawn into a busy morning/midday peak, easing through the afternoon
    // and winding down again at dusk. Mirrors the bands used by
    // AureliaWorld::ambianceForTime()/fogDensity() for the same cycle.

    float timeFactor;

    if (timeOfDay < 0.22f || timeOfDay > 0.88f) {

        timeFactor = 0.30f; // dead of night: streets mostly empty

    } else if (timeOfDay < 0.32f) {

        float t = (timeOfDay - 0.22f) / 0.10f;

        timeFactor = 0.30f + t * 0.55f; // dawn ramp-up

    } else if (timeOfDay < 0.55f) {

        timeFactor = 1.0f; // morning/midday rush

    } else if (timeOfDay < 0.72f) {

        timeFactor = 0.70f; // afternoon lull

    } else {

        float t = (timeOfDay - 0.72f) / 0.16f;

        timeFactor = 0.70f - t * 0.40f; // dusk wind-down

    }



    // Biome flavor from the world bible: the Port's docks run delivery
    // traffic around the clock, the Volcano's treacherous roads keep most
    // drivers away, the Forest's backroads are quiet, and the Coast/Marina
    // is the baseline.

    float biomeFactor = 1.0f;

    switch (biome) {

    case BiomeId::PORT:

        biomeFactor = 1.3f;

        break;

    case BiomeId::VOLCANO:

        biomeFactor = 0.45f;

        break;

    case BiomeId::FOREST:

        biomeFactor = 0.75f;

        break;

    case BiomeId::COAST:

    default:

        biomeFactor = 1.0f;

        break;

    }



    return std::clamp(timeFactor * biomeFactor, 0.15f, 1.0f);

}



void TrafficSystem::update(float dt, const ChunkStreamer &streamer,

    float playerX, float playerZ, float timeOfDay, BiomeId biome)

{

    densityTarget_ = computeDensityTarget(timeOfDay, biome);

    int targetActiveCount = std::clamp(

        static_cast<int>(std::round(densityTarget_

            * static_cast<float>(kMaxVehicles))),

        kMinActiveVehicles, kMaxVehicles);



    spawnIfNeeded(playerX, playerZ, streamer, targetActiveCount);

    despawnFar(playerX, playerZ);



    // Reactivate parked vehicles first, cheaper than spawning a new one,
    // so a density rise (e.g. entering the Port) brings idle pool slots
    // back to life immediately instead of waiting on the spawn timer.

    for (TrafficVehicle &v : vehicles_) {

        if (activeCount() >= targetActiveCount) {

            break;

        }

        if (!v.active) {

            v.active = true;

        }

    }



    for (TrafficVehicle &v : vehicles_) {

        if (!v.active) {

            continue;

        }

        Vector2 tangent = graph_.edgeTangent(v.edgeIndex, v.t);

        float speedPerT = std::sqrt(
            tangent.x * tangent.x + tangent.y * tangent.y);

        if (speedPerT <= 0.001f) {

            continue;

        }

        // Advance t by desired world-space speed divided by the local
        // curve speed (|dBezier/dt|), so vehicles move at a constant
        // world-space speed even where the Bezier's speed-per-t varies
        // across the curve (e.g. faster near the bend's midpoint).
        v.t += (v.speed * dt) / speedPerT;

        if (v.t >= 1.0f) {

            v.t -= 1.0f;

            v.edgeIndex = (v.edgeIndex + 1)

                % static_cast<int>(graph_.edges().size());

            tangent = graph_.edgeTangent(v.edgeIndex, v.t);



            // Natural respawn point: a vehicle that just completed its
            // loop is parked instead of continuing onto the next edge
            // when the pool is over the current density target, thinning
            // traffic without destroying pooled instances.

            if (activeCount() > targetActiveCount) {

                v.active = false;

                continue;

            }

        }

        v.heading = std::atan2(tangent.x, tangent.y);

    }



    refreshVisible();

}



} // namespace racer::world

