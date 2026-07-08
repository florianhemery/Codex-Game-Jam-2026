/*
** EPITECH PROJECT, 2026
** racer
** File description:
** RoadGraph implementation
*/

#include "World/Road/RoadGraph.hpp"

#include <algorithm>
#include <cmath>

namespace racer::world {

namespace {

float hashUnit(int a, int b)
{
    unsigned int h = static_cast<unsigned int>(a) * 374761393u
        + static_cast<unsigned int>(b) * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= h >> 16;
    return static_cast<float>(h & 0x00FFFFFFu)
        / static_cast<float>(0x01000000u);
}

} // namespace

int RoadGraph::addNode(Vector2 position)
{
    nodes_.push_back(RoadNode{position});
    return static_cast<int>(nodes_.size()) - 1;
}

void RoadGraph::addEdge(int from, int to, float speedLimit)
{
    RoadEdge edge{from, to, speedLimit, Vector2{0.0f, 0.0f}};

    if (from >= 0 && from < static_cast<int>(nodes_.size())
        && to >= 0 && to < static_cast<int>(nodes_.size())) {
        const RoadNode &a = nodes_[static_cast<size_t>(from)];
        const RoadNode &b = nodes_[static_cast<size_t>(to)];
        float dx = b.position.x - a.position.x;
        float dz = b.position.y - a.position.y;
        float length = std::sqrt(dx * dx + dz * dz);
        if (length > 0.001f) {
            float nx = -dz / length;
            float nz = dx / length;
            float side = hashUnit(from, to) > 0.5f ? 1.0f : -1.0f;
            float amount = side
                * std::clamp(length * 0.14f, 3.0f, 16.0f);
            float midX = (a.position.x + b.position.x) * 0.5f;
            float midZ = (a.position.y + b.position.y) * 0.5f;
            edge.control = Vector2{
                midX + nx * amount, midZ + nz * amount};
        }
    }

    edges_.push_back(edge);
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
    float u = 1.0f - t;
    return Vector2{
        u * u * a.position.x + 2.0f * u * t * edge.control.x
            + t * t * b.position.x,
        u * u * a.position.y + 2.0f * u * t * edge.control.y
            + t * t * b.position.y
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
