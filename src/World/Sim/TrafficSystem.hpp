/*
** EPITECH PROJECT, 2026
** racer
** File description:
** NPC traffic on road graph
*/

#ifndef TRAFFIC_SYSTEM_HPP_
#define TRAFFIC_SYSTEM_HPP_

#include <vector>

#include "World/Chunk/ChunkTypes.hpp"
#include "World/Road/RoadGraph.hpp"
#include "World/Stream/ChunkStreamer.hpp"

namespace racer::world {

struct TrafficVehicle {
    int edgeIndex = 0;
    float t = 0.0f;
    float speed = 12.0f;
    float heading = 0.0f;
    Color color = GRAY;
    // Inactive vehicles are "parked" out of the simulation (not advanced,
    // not drawn) so the traffic pool can be thinned/refilled live as the
    // density target changes with time-of-day and biome, without having to
    // allocate/destroy TrafficVehicle instances.
    bool active = true;
};

class TrafficSystem {
public:
    static constexpr int kMaxVehicles = 20;

    explicit TrafficSystem(const RoadGraph &graph);

    // timeOfDay in [0,1) and the biome the player currently drives through
    // drive a live density target: how many of the pooled vehicles should
    // be active this frame. Recomputed every call, so density follows the
    // day/night cycle and biome changes as the player roams the world.
    void update(float dt, const ChunkStreamer &streamer, float playerX,
        float playerZ, float timeOfDay, BiomeId biome);

    // Only active vehicles are exposed for rendering/collision purposes.
    const std::vector<TrafficVehicle> &vehicles() const
    {
        return visibleVehicles_;
    }

    float densityTarget() const { return densityTarget_; }

private:
    void spawnIfNeeded(float playerX, float playerZ,
        const ChunkStreamer &streamer, int targetActiveCount);
    void despawnFar(float playerX, float playerZ);
    int activeCount() const;
    void refreshVisible();
    static float computeDensityTarget(float timeOfDay, BiomeId biome);

    const RoadGraph &graph_;
    std::vector<TrafficVehicle> vehicles_;
    std::vector<TrafficVehicle> visibleVehicles_;
    float spawnTimer_ = 0.0f;
    float densityTarget_ = 1.0f;
};

} // namespace racer::world

#endif /* !TRAFFIC_SYSTEM_HPP_ */
