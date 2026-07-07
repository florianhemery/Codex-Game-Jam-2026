/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Core test helpers — World entity population and lifecycle checks
*/

#include "Engine/Core/CoreTestRunner.hpp"

#include <cstdint>
#include <string>

#include "Engine/Core/AiTag.hpp"
#include "Engine/Core/KinematicsComponent.hpp"
#include "Engine/Core/LapProgressComponent.hpp"
#include "Engine/Core/NameComponent.hpp"
#include "Engine/Core/PlayerTag.hpp"
#include "Engine/Core/RenderMeshComponent.hpp"
#include "Engine/Core/TransformComponent.hpp"

void CoreTestRunner::tagEntity(racer::engine::World &world,
    entt::entity entity, std::size_t index)
{
    if (index == 0) {
        world.add<racer::engine::PlayerTag>(entity);
    }
    else {
        world.add<racer::engine::AiTag>(entity);
    }
}

void CoreTestRunner::addEntityComponents(racer::engine::World &world,
    entt::entity entity, std::size_t index)
{
    const float fi = static_cast<float>(index);
    const auto meshId = static_cast<std::uint32_t>(index);
    const auto matId = static_cast<std::uint32_t>(index % 4);
    const Color tint{
        static_cast<unsigned char>(index), 64, 128, 255};

    world.add<racer::engine::TransformComponent>(
        entity, Vector3{fi, 0.0f, -fi}, 0.0f, 0.0f, 0.0f);
    world.add<racer::engine::KinematicsComponent>(
        entity, fi * 0.5f, 0.0f, false);
    world.add<racer::engine::RenderMeshComponent>(
        entity, meshId, matId, tint);
    world.add<racer::engine::LapProgressComponent>(entity);
    world.add<racer::engine::NameComponent>(
        entity, "Racer " + std::to_string(index));
}

void CoreTestRunner::addEntity(racer::engine::World &world,
    std::vector<entt::entity> &entities, std::size_t index)
{
    const entt::entity entity = world.createEntity();

    addEntityComponents(world, entity, index);
    tagEntity(world, entity, index);
    entities.push_back(entity);
}

void CoreTestRunner::populateWorld(racer::engine::World &world,
    std::vector<entt::entity> &entities)
{
    entities.reserve(K_ENTITY_COUNT);
    for (std::size_t i = 0; i < K_ENTITY_COUNT; ++i) {
        addEntity(world, entities, i);
    }
}

void CoreTestRunner::verifyWorldTags(const racer::engine::World &world,
    const std::vector<entt::entity> &entities)
{
    check(world.has<racer::engine::PlayerTag>(entities[0]),
        __LINE__, "world.has<PlayerTag>(entities[0])");
    check(!world.has<racer::engine::AiTag>(entities[0]),
        __LINE__, "!world.has<AiTag>(entities[0])");
    check(world.has<racer::engine::AiTag>(entities[1]),
        __LINE__, "world.has<AiTag>(entities[1])");
    check(world.get<racer::engine::NameComponent>(entities[42]).name
        == "Racer 42", __LINE__,
        "world.get<NameComponent>(entities[42]).name == \"Racer 42\"");
    check(world.registry().view<racer::engine::PlayerTag>().size() == 1,
        __LINE__, "world.registry().view<PlayerTag>().size() == 1");
    check(world.registry().view<racer::engine::AiTag>().size()
        == K_ENTITY_COUNT - 1, __LINE__,
        "world.registry().view<AiTag>().size() == K_ENTITY_COUNT - 1");
}

void CoreTestRunner::verifyWorldLifecycle(racer::engine::World &world)
{
    const entt::entity temp = world.createEntity();

    check(world.registry().valid(temp), __LINE__,
        "world.registry().valid(temp)");
    world.destroyEntity(temp);
    check(!world.registry().valid(temp), __LINE__,
        "!world.registry().valid(temp)");
}
