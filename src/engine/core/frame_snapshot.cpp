/*
** EPITECH PROJECT, 2026
** racer
** File description:
** snapshot buffer implementation and world capture
*/

#include "engine/core/frame_snapshot.h"

#include "engine/core/components.h"
#include "engine/core/world.h"

namespace racer::engine {

FrameSnapshot& SnapshotBuffer::WriteBegin()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return buffers_[writeIndex_];
}

void SnapshotBuffer::Publish()
{
    std::lock_guard<std::mutex> lock(mutex_);
    writeIndex_ = 1 - writeIndex_;
}

const FrameSnapshot& SnapshotBuffer::ReadLatest() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return buffers_[1 - writeIndex_];
}

void CaptureSnapshot(World& world, FrameSnapshot& snapshot)
{
    snapshot.items.clear();
    auto view = world.Registry().view<const TransformComponent,
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
