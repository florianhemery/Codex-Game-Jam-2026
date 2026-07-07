/*
** EPITECH PROJECT, 2026
** racer
** File description:
** sim/render frame snapshot types and double-buffer
*/

#ifndef FRAME_SNAPSHOT_H_
# define FRAME_SNAPSHOT_H_

# include <cstdint>
# include <mutex>
# include <vector>

# include "raylib.h"

namespace racer::engine {

class World;

// Element de rendu fige pour le thread rendu (sans acces ECS).
struct RenderItem {
    std::uint32_t meshId = 0;
    std::uint32_t materialId = 0;
    Vector3 position{0.0f, 0.0f, 0.0f};
    float heading = 0.0f;
    float roll = 0.0f;
    Color tint{255, 255, 255, 255};
};

// Photo complete d'une frame de simulation, consommable par le rendu.
struct FrameSnapshot {
    double simTime = 0.0;
    std::vector<RenderItem> items;
};

// Double buffering sim/rendu : WriteBegin/Publish cote sim, ReadLatest cote
// rendu. ReadLatest() reste valable jusqu'au Publish() suivant.
class SnapshotBuffer {
public:
    FrameSnapshot& WriteBegin();
    void Publish();
    const FrameSnapshot& ReadLatest() const;

private:
    FrameSnapshot buffers_[2];
    int writeIndex_ = 0;
    mutable std::mutex mutex_;
};

// Remplit snapshot.items depuis Transform+RenderMesh (simTime : appelant).
void CaptureSnapshot(World& world, FrameSnapshot& snapshot);

} // namespace racer::engine

#endif /* !FRAME_SNAPSHOT_H_ */
