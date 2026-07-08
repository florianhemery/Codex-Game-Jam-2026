/*
** EPITECH PROJECT, 2026
** racer
** File description:
** 3x3 chunk ring buffer around player
*/

#ifndef CHUNK_STREAMER_HPP_
#define CHUNK_STREAMER_HPP_

#include <unordered_map>
#include <vector>

#include "World/Chunk/ChunkData.hpp"
#include "World/Chunk/ChunkGenerator.hpp"

namespace racer::world {

class ChunkStreamer {
public:
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

    ChunkId center_{};
    std::vector<ChunkData> loaded_;
    std::unordered_map<ChunkId, size_t, ChunkIdHash> index_;
};

} // namespace racer::world

#endif /* !CHUNK_STREAMER_HPP_ */
