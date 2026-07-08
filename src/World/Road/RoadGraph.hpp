/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Road network for traffic and navigation
*/

#ifndef ROAD_GRAPH_HPP_
#define ROAD_GRAPH_HPP_

#include <vector>

#include "raylib.h"

namespace racer::world {

struct RoadNode {
    Vector2 position{};
};

struct RoadEdge {
    int from = 0;
    int to = 0;
    float speedLimit = 35.0f;
};

class RoadGraph {
public:
    int addNode(Vector2 position);
    void addEdge(int from, int to, float speedLimit);
    bool empty() const { return nodes_.empty(); }

    const std::vector<RoadNode> &nodes() const { return nodes_; }
    const std::vector<RoadEdge> &edges() const { return edges_; }

    Vector2 pointOnEdge(int edgeIndex, float t) const;
    float edgeLength(int edgeIndex) const;

private:
    std::vector<RoadNode> nodes_;
    std::vector<RoadEdge> edges_;
};

} // namespace racer::world

#endif /* !ROAD_GRAPH_HPP_ */
