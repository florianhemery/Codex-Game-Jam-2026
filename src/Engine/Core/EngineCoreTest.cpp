/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Headless core module test (World, JobSystem, SnapshotBuffer)
*/

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <future>
#include <string>
#include <vector>

#include "Engine/Core/Components.hpp"
#include "Engine/Core/SnapshotBuffer.hpp"
#include "Engine/Core/JobSystem.hpp"
#include "Engine/Core/World.hpp"

namespace {

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
    void applyParallelTransform(racer::engine::World &world,
        const std::vector<entt::entity> &entities, std::size_t index);
    void verifyTransformsAfterParallel(
        const racer::engine::World &world,
        const std::vector<entt::entity> &entities);
    void indexRenderItems(const racer::engine::FrameSnapshot &read,
        std::vector<const racer::engine::RenderItem *> &byMesh);
    void verifySnapshotItems(const racer::engine::FrameSnapshot &read);
    void verifyRenderItemFields(std::size_t index,
        const racer::engine::RenderItem *item);
    void verifyRenderItemTint(std::size_t index,
        const racer::engine::RenderItem *item);
    void failOnParallelIndex(std::size_t index);

    int failures_ = 0;
};

void CoreTestRunner::check(bool cond, int line, const char *expr)
{
    if (!cond) {
        std::fprintf(stderr, "ECHEC ligne %d : %s\n", line, expr);
        ++failures_;
    }
}

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

void CoreTestRunner::addEntity(racer::engine::World &world,
    std::vector<entt::entity> &entities, std::size_t index)
{
    const entt::entity entity = world.createEntity();
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

void CoreTestRunner::indexRenderItems(
    const racer::engine::FrameSnapshot &read,
    std::vector<const racer::engine::RenderItem *> &byMesh)
{
    for (const racer::engine::RenderItem &item : read.items) {
        check(item.meshId < K_ENTITY_COUNT, __LINE__,
            "item.meshId < K_ENTITY_COUNT");
        if (item.meshId < K_ENTITY_COUNT) {
            check(byMesh[item.meshId] == nullptr, __LINE__,
                "byMesh[item.meshId] == nullptr");
            byMesh[item.meshId] = &item;
        }
    }
}

void CoreTestRunner::verifyRenderItemFields(std::size_t index,
    const racer::engine::RenderItem *item)
{
    const float fi = static_cast<float>(index);

    check(item != nullptr, __LINE__, "item != nullptr");
    if (item == nullptr) {
        return;
    }
    check(item->position.x == fi, __LINE__, "item->position.x == fi");
    check(item->position.y == fi * 2.0f, __LINE__,
        "item->position.y == fi * 2.0f");
    check(item->position.z == -fi, __LINE__, "item->position.z == -fi");
    check(item->heading == fi * 0.01f, __LINE__,
        "item->heading == fi * 0.01f");
    check(item->roll == fi * 0.001f, __LINE__,
        "item->roll == fi * 0.001f");
    check(item->materialId == static_cast<std::uint32_t>(index % 4),
        __LINE__,
        "item->materialId == static_cast<std::uint32_t>(index % 4)");
}

void CoreTestRunner::verifyRenderItemTint(std::size_t index,
    const racer::engine::RenderItem *item)
{
    if (item == nullptr) {
        return;
    }
    check(item->tint.r == static_cast<unsigned char>(index), __LINE__,
        "item->tint.r == static_cast<unsigned char>(index)");
    check(item->tint.g == 64, __LINE__, "item->tint.g == 64");
    check(item->tint.b == 128, __LINE__, "item->tint.b == 128");
    check(item->tint.a == 255, __LINE__, "item->tint.a == 255");
}

void CoreTestRunner::verifySnapshotItems(
    const racer::engine::FrameSnapshot &read)
{
    std::vector<const racer::engine::RenderItem *> byMesh(
        K_ENTITY_COUNT, nullptr);

    indexRenderItems(read, byMesh);
    for (std::size_t i = 0; i < K_ENTITY_COUNT; ++i) {
        verifyRenderItemFields(i, byMesh[i]);
        verifyRenderItemTint(i, byMesh[i]);
    }
}

racer::engine::FrameSnapshot &CoreTestRunner::verifySnapshot(
    racer::engine::World &world, racer::engine::SnapshotBuffer &buffer)
{
    check(buffer.readLatest().items.empty(), __LINE__,
        "buffer.readLatest().items.empty()");
    racer::engine::FrameSnapshot &write = buffer.writeBegin();
    write.simTime = 1.25;
    racer::engine::SnapshotBuffer::capture(world, write);
    buffer.publish();

    const racer::engine::FrameSnapshot &read = buffer.readLatest();
    check(&read == &write, __LINE__, "&read == &write");
    check(read.simTime == 1.25, __LINE__, "read.simTime == 1.25");
    check(read.items.size() == K_ENTITY_COUNT, __LINE__,
        "read.items.size() == K_ENTITY_COUNT");
    verifySnapshotItems(read);
    return write;
}

void CoreTestRunner::verifySecondFrame(
    racer::engine::World &world,
    const std::vector<entt::entity> &entities,
    racer::engine::SnapshotBuffer &buffer,
    const racer::engine::FrameSnapshot &firstWrite)
{
    world.get<racer::engine::TransformComponent>(entities[0]).position.y =
        999.0f;
    racer::engine::FrameSnapshot &write2 = buffer.writeBegin();
    check(&write2 != &firstWrite, __LINE__, "&write2 != &firstWrite");
    write2.simTime = 2.5;
    racer::engine::SnapshotBuffer::capture(world, write2);
    buffer.publish();

    const racer::engine::FrameSnapshot &read2 = buffer.readLatest();
    check(&read2 == &write2, __LINE__, "&read2 == &write2");
    check(read2.simTime == 2.5, __LINE__, "read2.simTime == 2.5");
    check(read2.items.size() == K_ENTITY_COUNT, __LINE__,
        "read2.items.size() == K_ENTITY_COUNT");

    bool foundUpdated = false;
    for (const racer::engine::RenderItem &item : read2.items) {
        if (item.meshId == 0) {
            foundUpdated = (item.position.y == 999.0f);
        }
    }
    check(foundUpdated, __LINE__, "foundUpdated");
}

int CoreTestRunner::failures() const
{
    return failures_;
}

} // namespace

int main()
{
    CoreTestRunner runner;
    racer::engine::World world;
    std::vector<entt::entity> entities;
    racer::engine::SnapshotBuffer buffer;

    runner.populateWorld(world, entities);
    runner.verifyWorldTags(world, entities);
    runner.verifyWorldLifecycle(world);
    runner.verifyJobSystem();
    runner.verifyParallelFor(world, entities);
    racer::engine::FrameSnapshot &firstWrite =
        runner.verifySnapshot(world, buffer);
    runner.verifySecondFrame(world, entities, buffer, firstWrite);

    if (runner.failures() != 0) {
        std::fprintf(stderr, "%d verification(s) en echec\n",
            runner.failures());
        return 1;
    }
    std::printf("OK\n");
    return 0;
}
