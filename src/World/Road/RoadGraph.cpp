/*
** EPITECH PROJECT, 2026
** racer
** File description:
** RoadGraph implementation
*/

#include "World/Road/RoadGraph.hpp"

#include <cmath>

namespace racer::world {

int RoadGraph::addNode(Vector2 position)
{
    nodes_.push_back(RoadNode{position});
    return static_cast<int>(nodes_.size()) - 1;
}

void RoadGraph::addEdge(int from, int to, float speedLimit)
{
    edges_.push_back(RoadEdge{from, to, speedLimit});
}

Vector2 RoadGraph::pointOnEdge(int edgeIndex, float t) const
{
    if (edgeIndex < 0 || edgeIndex >= static_cast<int>(edges_.size())) {
        return Vector2{0.0f, 0.0f};
    }
    const RoadEdge &edge = edges_[static_cast<size_t>(edgeIndex)];
    if (edge.from < 0 || edge.from >= static_cast<int>(nodes_.size())
        || edge.to < 0 || edge.to >= static_cast<int>(nodes_.size())) {
        return Vector2{0.0f, 0.0f};
    }
    const RoadNode &a = nodes_[static_cast<size_t>(edge.from)];
    const RoadNode &b = nodes_[static_cast<size_t>(edge.to)];
    t = std::clamp(t, 0.0f, 1.0f);
    return Vector2{
        a.position.x + (b.position.x - a.position.x) * t,
        a.position.y + (b.position.y - a.position.y) * t
    };
}

float RoadGraph::edgeLength(int edgeIndex) const
{
    if (edgeIndex < 0 || edgeIndex >= static_cast<int>(edges_.size())) {
        return 0.0f;
    }
    const RoadEdge &edge = edges_[static_cast<size_t>(edgeIndex)];
    if (edge.from < 0 || edge.from >= static_cast<int>(nodes_.size())
        || edge.to < 0 || edge.to >= static_cast<int>(nodes_.size())) {
        return 0.0f;
    }
    const RoadNode &a = nodes_[static_cast<size_t>(edge.from)];
    const RoadNode &b = nodes_[static_cast<size_t>(edge.to)];
    float dx = b.position.x - a.position.x;
    float dz = b.position.y - a.position.y;
    return std::sqrt(dx * dx + dz * dz);
}

} // namespace racer::world
