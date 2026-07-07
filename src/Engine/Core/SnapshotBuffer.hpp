/*
** EPITECH PROJECT, 2026
** racer
** File description:
** sim/render frame snapshot types and double-buffer
*/

#ifndef SNAPSHOT_BUFFER_HPP_
# define SNAPSHOT_BUFFER_HPP_

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

// Double buffering sim/rendu : writeBegin/publish cote sim, readLatest cote
// rendu. readLatest() reste valable jusqu'au publish() suivant.
class SnapshotBuffer {
public:
    FrameSnapshot& writeBegin();
    void publish();
    const FrameSnapshot& readLatest() const;
    static void capture(World& world, FrameSnapshot& snapshot);

private:
    FrameSnapshot buffers_[2];
    int writeIndex_ = 0;
    mutable std::mutex mutex_;
};

} // namespace racer::engine

#endif /* !SNAPSHOT_BUFFER_HPP_ */
