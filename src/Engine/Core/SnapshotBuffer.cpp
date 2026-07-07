/*
** EPITECH PROJECT, 2026
** racer
** File description:
** snapshot buffer implementation and world capture
*/

#include "Engine/Core/SnapshotBuffer.hpp"

#include "Engine/Core/Components.hpp"
#include "Engine/Core/World.hpp"

namespace racer::engine {

FrameSnapshot& SnapshotBuffer::writeBegin()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return buffers_[writeIndex_];
}

void SnapshotBuffer::publish()
{
    std::lock_guard<std::mutex> lock(mutex_);
    writeIndex_ = 1 - writeIndex_;
}

const FrameSnapshot& SnapshotBuffer::readLatest() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return buffers_[1 - writeIndex_];
}

void SnapshotBuffer::capture(World& world, FrameSnapshot& snapshot)
{
    snapshot.items.clear();
    auto view = world.registry().view<const TransformComponent,
        const RenderMeshComponent>();
    snapshot.items.reserve(view.size_hint());
    view.each([&snapshot](const TransformComponent& transform,
        const RenderMeshComponent& mesh) {
        RenderItem item;
        item.meshId = mesh.meshId;
        item.materialId = mesh.materialId;
        item.position = transform.position;
        item.heading = transform.heading;
        item.roll = transform.roll;
        item.tint = mesh.tint;
        snapshot.items.push_back(item);
    });
}

} // namespace racer::engine
