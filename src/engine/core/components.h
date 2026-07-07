/*
** EPITECH PROJECT, 2026
** racer
** File description:
** ECS component definitions
*/

#ifndef COMPONENTS_H_
#define COMPONENTS_H_

#include <cstdint>
#include <string>

#include "raylib.h"

namespace racer::engine {

struct TransformComponent {
    Vector3 position{0.0f, 0.0f, 0.0f};
    float heading = 0.0f;
    float roll = 0.0f;
    float pitch = 0.0f;
};

struct KinematicsComponent {
    float speed = 0.0f;
    float velocityHeading = 0.0f;
    bool isDrifting = false;
};

struct RenderMeshComponent {
    std::uint32_t meshId = 0;
    std::uint32_t materialId = 0;
    Color tint{255, 255, 255, 255};
};

struct LapProgressComponent {
    int lap = 0;
    int lastSegment = 0;
    bool passedMidpoint = false;
    bool finished = false;
    float finishTime = 0.0f;
};

struct NameComponent {
    std::string name;
};

struct PlayerTag {};

struct AiTag {};

} // namespace racer::engine

#endif /* !COMPONENTS_H_ */
