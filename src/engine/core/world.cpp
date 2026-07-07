/*
** EPITECH PROJECT, 2026
** racer
** File description:
** World class implementation
*/

#include "engine/core/world.h"

namespace racer::engine {

entt::entity World::CreateEntity()
{
    return registry_.create();
}

void World::DestroyEntity(entt::entity entity)
{
    registry_.destroy(entity);
}

} // namespace racer::engine
