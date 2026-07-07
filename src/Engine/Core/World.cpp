/*
** EPITECH PROJECT, 2026
** racer
** File description:
** World class implementation
*/

#include "Engine/Core/World.hpp"

namespace racer::engine {

entt::entity World::createEntity()
{
    return registry_.create();
}

void World::destroyEntity(entt::entity entity)
{
    registry_.destroy(entity);
}

} // namespace racer::engine
