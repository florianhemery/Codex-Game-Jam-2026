/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Core test helpers — JobSystem submit and parallelFor checks
*/

#include "Engine/Core/CoreTestRunner.hpp"

#include <atomic>
#include <future>

#include "Engine/Core/JobSystem.hpp"
#include "Engine/Core/TransformComponent.hpp"

void CoreTestRunner::verifyJobSystem()
{
    racer::engine::JobSystem jobs;

    check(jobs.workerCount() >= 1, __LINE__, "jobs.workerCount() >= 1");
    std::atomic<int> counter{0};
    std::future<void> done = jobs.submit([&counter] {
        counter.fetch_add(1);
    });
    done.get();
    check(counter.load() == 1, __LINE__, "counter.load() == 1");
}

void CoreTestRunner::applyParallelTransform(
    racer::engine::World &world,
    const std::vector<entt::entity> &entities, std::size_t index)
{
    racer::engine::TransformComponent &transform =
        world.get<racer::engine::TransformComponent>(entities[index]);
    const float fi = static_cast<float>(index);

    transform.position.y = fi * 2.0f;
    transform.heading = fi * 0.01f;
    transform.roll = fi * 0.001f;
}

void CoreTestRunner::verifyTransformsAfterParallel(
    const racer::engine::World &world,
    const std::vector<entt::entity> &entities)
{
    for (std::size_t i = 0; i < K_ENTITY_COUNT; ++i) {
        const float expected = static_cast<float>(i) * 2.0f;
        const auto &transform =
            world.get<racer::engine::TransformComponent>(entities[i]);

        check(transform.position.y == expected, __LINE__,
            "transform.position.y == expected");
    }
}

void CoreTestRunner::failOnParallelIndex(std::size_t index)
{
    (void)index;
    check(false, __LINE__, "false");
}

void CoreTestRunner::verifyParallelFor(
    racer::engine::World &world,
    const std::vector<entt::entity> &entities)
{
    racer::engine::JobSystem jobs;

    jobs.parallelFor(0, K_ENTITY_COUNT, 7,
        [this, &world, &entities](std::size_t index) {
            applyParallelTransform(world, entities, index);
        });
    verifyTransformsAfterParallel(world, entities);
    jobs.parallelFor(5, 5, 4,
        [this](std::size_t index) {
            failOnParallelIndex(index);
        });
}
