/*
** EPITECH PROJECT, 2026
** racer
** File description:
** NPC traffic on road graph
*/

#ifndef TRAFFIC_SYSTEM_HPP_
#define TRAFFIC_SYSTEM_HPP_

#include <vector>

#include "World/Road/RoadGraph.hpp"
#include "World/Stream/ChunkStreamer.hpp"

namespace racer::world {

struct TrafficVehicle {
    int edgeIndex = 0;
    float t = 0.0f;
    float speed = 12.0f;
    float heading = 0.0f;
    Color color = GRAY;
};

class TrafficSystem {
public:
    static constexpr int kMaxVehicles = 20;

    explicit TrafficSystem(const RoadGraph &graph);

    void update(float dt, const ChunkStreamer &streamer, float playerX,
        float playerZ);

    const std::vector<TrafficVehicle> &vehicles() const { return vehicles_; }

private:
    void spawnIfNeeded(float playerX, float playerZ,
        const ChunkStreamer &streamer);
    void despawnFar(float playerX, float playerZ);

    const RoadGraph &graph_;
    std::vector<TrafficVehicle> vehicles_;
    float spawnTimer_ = 0.0f;
};

} // namespace racer::world

#endif /* !TRAFFIC_SYSTEM_HPP_ */
