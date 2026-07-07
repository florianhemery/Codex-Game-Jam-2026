/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Core test helpers — SnapshotBuffer capture and render item checks
*/

#include "Engine/Core/CoreTestRunner.hpp"

#include <cstdint>

#include "Engine/Core/SnapshotBuffer.hpp"
#include "Engine/Core/TransformComponent.hpp"

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

void CoreTestRunner::verifyRenderItemPosition(std::size_t index,
    const racer::engine::RenderItem *item)
{
    const float fi = static_cast<float>(index);

    check(item->position.x == fi, __LINE__, "item->position.x == fi");
    check(item->position.y == fi * 2.0f, __LINE__,
        "item->position.y == fi * 2.0f");
    check(item->position.z == -fi, __LINE__, "item->position.z == -fi");
}

void CoreTestRunner::verifyRenderItemOrientation(std::size_t index,
    const racer::engine::RenderItem *item)
{
    const float fi = static_cast<float>(index);

    check(item->heading == fi * 0.01f, __LINE__,
        "item->heading == fi * 0.01f");
    check(item->roll == fi * 0.001f, __LINE__,
        "item->roll == fi * 0.001f");
    check(item->materialId == static_cast<std::uint32_t>(index % 4),
        __LINE__,
        "item->materialId == static_cast<std::uint32_t>(index % 4)");
}

void CoreTestRunner::verifyRenderItemFields(std::size_t index,
    const racer::engine::RenderItem *item)
{
    check(item != nullptr, __LINE__, "item != nullptr");
    if (item == nullptr) {
        return;
    }
    verifyRenderItemPosition(index, item);
    verifyRenderItemOrientation(index, item);
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

bool CoreTestRunner::findUpdatedMesh(
    const racer::engine::FrameSnapshot &read)
{
    for (const racer::engine::RenderItem &item : read.items) {
        if (item.meshId == 0) {
            return item.position.y == 999.0f;
        }
    }
    return false;
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
    check(findUpdatedMesh(read2), __LINE__, "findUpdatedMesh(read2)");
}
