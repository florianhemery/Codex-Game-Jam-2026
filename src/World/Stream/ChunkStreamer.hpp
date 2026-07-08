/*
** EPITECH PROJECT, 2026
** racer
** File description:
** 3x3 chunk ring buffer around player
*/

#ifndef CHUNK_STREAMER_HPP_
#define CHUNK_STREAMER_HPP_

#include <future>
#include <unordered_map>
#include <vector>

#include "World/Chunk/ChunkData.hpp"
#include "World/Chunk/ChunkGenerator.hpp"

namespace racer::world {

// Chunk generation (heightmap sampling, road painting, prop scattering) is
// real work -- expensive enough to stutter the main thread if run
// sequentially for every chunk that enters the streaming ring in the same
// frame. ensureLoaded() dispatches generation for missing ring chunks onto
// background threads (std::async) in parallel, then resolves the ring
// before returning -- so a straight-line crossing (3 new chunks) or a
// diagonal one (up to 5) costs roughly one generate() worth of wall time
// instead of three to five. Direct point queries
// (sampleHeight/sampleSurface/isLoaded) also fall back to synchronous
// generation for chunks inside the current ring that aren't resolved yet,
// so results are always correct even if queried outside the normal
// ensureLoaded() cycle -- merely un-optimized on that rare call.
class ChunkStreamer {
public:
    ChunkStreamer() = default;
    // Any std::async futures still parked in pending_ block in their own
    // destructor until the background thread finishes, so tearing down the
    // containers below is enough to guarantee no dangling threads on
    // shutdown.
    ~ChunkStreamer() = default;

    void updateCenter(float worldX, float worldZ);
    void ensureLoaded();

    const ChunkData *findChunk(ChunkId id) const;
    const std::vector<ChunkData> &loadedChunks() const { return loaded_; }

    bool isLoaded(float worldX, float worldZ) const;
    float sampleHeight(float worldX, float worldZ) const;
    SurfaceKind sampleSurface(float worldX, float worldZ) const;
    BiomeId biomeAt(float worldX, float worldZ) const;

    ChunkId centerChunk() const { return center_; }

private:
    void loadChunk(ChunkId id);
    void unloadOutsideRing();
    void drainPending() const;
    const ChunkData *integrate(ChunkId id, ChunkData data) const;
    const ChunkData *findChunkUrgent(ChunkId id) const;
    const ChunkData *forceSyncLoad(ChunkId id) const;

    ChunkId center_{};
    mutable std::vector<ChunkData> loaded_;
    mutable std::unordered_map<ChunkId, size_t, ChunkIdHash> index_;
    mutable std::unordered_map<ChunkId, std::future<ChunkData>, ChunkIdHash>
        pending_;
};

} // namespace racer::world

#endif /* !CHUNK_STREAMER_HPP_ */
