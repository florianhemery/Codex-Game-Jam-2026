/*

** EPITECH PROJECT, 2026

** racer

** File description:

** TrafficSystem implementation

*/



#include "World/Sim/TrafficSystem.hpp"



#include <cmath>



namespace racer::world {



namespace {



constexpr float kSpawnRadius = 140.0f;

constexpr float kDespawnRadius = 200.0f;



float distXZ(float ax, float az, float bx, float bz)

{

    float dx = ax - bx;

    float dz = az - bz;

    return std::sqrt(dx * dx + dz * dz);

}



} // namespace



TrafficSystem::TrafficSystem(const RoadGraph &graph) : graph_(graph) {}



void TrafficSystem::spawnIfNeeded(float playerX, float playerZ,

    const ChunkStreamer &streamer)

{

    if (graph_.edges().empty()

        || static_cast<int>(vehicles_.size()) >= kMaxVehicles) {

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



void TrafficSystem::update(float dt, const ChunkStreamer &streamer,

    float playerX, float playerZ)

{

    spawnIfNeeded(playerX, playerZ, streamer);

    despawnFar(playerX, playerZ);



    for (TrafficVehicle &v : vehicles_) {

        float len = graph_.edgeLength(v.edgeIndex);

        if (len <= 0.001f) {

            continue;

        }

        v.t += (v.speed * dt) / len;

        if (v.t >= 1.0f) {

            v.t -= 1.0f;

            v.edgeIndex = (v.edgeIndex + 1)

                % static_cast<int>(graph_.edges().size());

        }

        Vector2 p = graph_.pointOnEdge(v.edgeIndex, v.t);

        float nextT = std::min(v.t + 0.05f, 1.0f);

        Vector2 p2 = graph_.pointOnEdge(v.edgeIndex, nextT);

        v.heading = std::atan2(p2.x - p.x, p2.y - p.y);

    }

}



} // namespace racer::world

