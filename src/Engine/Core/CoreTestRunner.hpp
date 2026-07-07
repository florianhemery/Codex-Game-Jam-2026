/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Headless core module test runner (World, JobSystem, SnapshotBuffer)
*/

#ifndef CORE_TEST_RUNNER_HPP_
#define CORE_TEST_RUNNER_HPP_

#include <cstddef>
#include <vector>

#include <entt/entt.hpp>

#include "Engine/Core/SnapshotBuffer.hpp"
#include "Engine/Core/World.hpp"

constexpr std::size_t K_ENTITY_COUNT = 100;

class CoreTestRunner {
public:
    void check(bool cond, int line, const char *expr);
    void populateWorld(racer::engine::World &world,
        std::vector<entt::entity> &entities);
    void verifyWorldTags(const racer::engine::World &world,
        const std::vector<entt::entity> &entities);
    void verifyWorldLifecycle(racer::engine::World &world);
    void verifyJobSystem();
    void verifyParallelFor(racer::engine::World &world,
        const std::vector<entt::entity> &entities);
    racer::engine::FrameSnapshot &verifySnapshot(
        racer::engine::World &world,
        racer::engine::SnapshotBuffer &buffer);
    void verifySecondFrame(racer::engine::World &world,
        const std::vector<entt::entity> &entities,
        racer::engine::SnapshotBuffer &buffer,
        const racer::engine::FrameSnapshot &firstWrite);
    int failures() const;

private:
    void addEntity(racer::engine::World &world,
        std::vector<entt::entity> &entities, std::size_t index);
    void tagEntity(racer::engine::World &world, entt::entity entity,
        std::size_t index);
    void addEntityComponents(racer::engine::World &world,
        entt::entity entity, std::size_t index);
    void applyParallelTransform(racer::engine::World &world,
        const std::vector<entt::entity> &entities, std::size_t index);
    void verifyTransformsAfterParallel(
        const racer::engine::World &world,
        const std::vector<entt::entity> &entities);
    void indexRenderItems(const racer::engine::FrameSnapshot &read,
        std::vector<const racer::engine::RenderItem *> &byMesh);
    void verifySnapshotItems(const racer::engine::FrameSnapshot &read);
    void verifyRenderItemPosition(std::size_t index,
        const racer::engine::RenderItem *item);
    void verifyRenderItemOrientation(std::size_t index,
        const racer::engine::RenderItem *item);
    void verifyRenderItemFields(std::size_t index,
        const racer::engine::RenderItem *item);
    void verifyRenderItemTint(std::size_t index,
        const racer::engine::RenderItem *item);
    void failOnParallelIndex(std::size_t index);
    bool findUpdatedMesh(const racer::engine::FrameSnapshot &read);

    int failures_ = 0;
};

#endif /* !CORE_TEST_RUNNER_HPP_ */
