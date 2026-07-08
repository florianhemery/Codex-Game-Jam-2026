/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Playable peninsula bounds for Aurelia open world
*/

#ifndef AURELIA_BOUNDS_HPP_
#define AURELIA_BOUNDS_HPP_

#include <algorithm>
#include <cmath>

#include "Vehicle/Car.hpp"

namespace racer::world {

struct WorldBounds {
    static constexpr float minX = -120.0f;
    static constexpr float maxX = 220.0f;
    static constexpr float minZ = -180.0f;
    static constexpr float maxZ = 220.0f;
    static constexpr float centerX = 50.0f;
    static constexpr float centerZ = 20.0f;
    static constexpr float softMargin = 4.0f;
    static constexpr float edgeBand = 30.0f;

    static constexpr float width() { return maxX - minX; }
    static constexpr float height() { return maxZ - minZ; }
};

inline float clampWorldX(float x)
{
    return std::clamp(x, WorldBounds::minX, WorldBounds::maxX);
}

inline float clampWorldZ(float z)
{
    return std::clamp(z, WorldBounds::minZ, WorldBounds::maxZ);
}

inline float distanceToWorldEdge(float worldX, float worldZ)
{
    float dLeft = worldX - WorldBounds::minX;
    float dRight = WorldBounds::maxX - worldX;
    float dSouth = worldZ - WorldBounds::minZ;
    float dNorth = WorldBounds::maxZ - worldZ;
    return std::min({dLeft, dRight, dSouth, dNorth});
}

inline void softClampCar(Car &car, float dt)
{
    Vector3 &p = car.position();

    float edge = distanceToWorldEdge(p.x, p.z);
    if (edge < WorldBounds::edgeBand) {
        float t = 1.0f - std::clamp(edge / WorldBounds::edgeBand, 0.0f, 1.0f);
        float damping = std::clamp(t * t * 3.0f * dt, 0.0f, 0.9f);
        car.speed() *= (1.0f - damping);
    }

    p.x = clampWorldX(p.x);
    p.z = clampWorldZ(p.z);
}

} // namespace racer::world

#endif /* !AURELIA_BOUNDS_HPP_ */
