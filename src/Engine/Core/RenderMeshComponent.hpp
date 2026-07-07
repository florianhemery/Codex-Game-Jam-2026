/*
** EPITECH PROJECT, 2026
** racer
** File description:
** ECS render mesh component
*/

#ifndef RENDER_MESH_COMPONENT_HPP_
#define RENDER_MESH_COMPONENT_HPP_

#include <cstdint>

#include "raylib.h"

namespace racer::engine {

struct RenderMeshComponent {
    std::uint32_t meshId = 0;
    std::uint32_t materialId = 0;
    Color tint{255, 255, 255, 255};
};

} // namespace racer::engine

#endif /* !RENDER_MESH_COMPONENT_HPP_ */
