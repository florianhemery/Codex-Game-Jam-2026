/*
** EPITECH PROJECT, 2026
** racer
** File description:
** ECS transform component
*/

#ifndef TRANSFORM_COMPONENT_HPP_
#define TRANSFORM_COMPONENT_HPP_

#include "raylib.h"

namespace racer::engine {

struct TransformComponent {
    Vector3 position{0.0f, 0.0f, 0.0f};
    float heading = 0.0f;
    float roll = 0.0f;
    float pitch = 0.0f;
};

} // namespace racer::engine

#endif /* !TRANSFORM_COMPONENT_HPP_ */
