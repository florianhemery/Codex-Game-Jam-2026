#include "server_core/chunk_storage.h"

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>

namespace server_core {

namespace {
constexpr uint32_t kChunkFileMagic = 0x564F5843; // 'V','O','X','C'
constexpr uint8_t kChunkFileVersion = 1;
} // namespace

ChunkStorage::ChunkStorage(std::string rootDir) : rootDir_(std::move(rootDir)) {
    std::filesystem::create_directories(std::filesystem::path(rootDir_) / "chunks");
}

std::string ChunkStorage::PathFor(common::world::ChunkCoord coord) const {
    return (std::filesystem::path(rootDir_) / "chunks" /
            (std::to_string(coord.x) + "_" + std::to_string(coord.z) + ".chunk"))
        .string();
}

bool ChunkStorage::Load(common::world::ChunkCoord coord, common::world::Chunk& outChunk) const {
    std::ifstream file(PathFor(coord), std::ios::binary);
    if (!file.is_open()) return false;

    uint32_t magic = 0;
    uint8_t version = 0;
    int32_t cx = 0, cz = 0;

    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    file.read(reinterpret_cast<char*>(&version), sizeof(version));
    file.read(reinterpret_cast<char*>(&cx), sizeof(cx));
    file.read(reinterpret_cast<char*>(&cz), sizeof(cz));

    if (!file || magic != kChunkFileMagic || version != kChunkFileVersion) return false;

    outChunk.coord = {cx, cz};
    file.read(reinterpret_cast<char*>(outChunk.blocks.data()),
              static_cast<std::streamsize>(outChunk.blocks.size()));

    return static_cast<bool>(file);
}

void ChunkStorage::Save(const common::world::Chunk& chunk) const {
    std::ofstream file(PathFor(chunk.coord), std::ios::binary | std::ios::trunc);
    if (!file.is_open()) return;

    uint32_t magic = kChunkFileMagic;
    uint8_t version = kChunkFileVersion;

    file.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
    file.write(reinterpret_cast<const char*>(&version), sizeof(version));
    file.write(reinterpret_cast<const char*>(&chunk.coord.x), sizeof(chunk.coord.x));
    file.write(reinterpret_cast<const char*>(&chunk.coord.z), sizeof(chunk.coord.z));
    file.write(reinterpret_cast<const char*>(chunk.blocks.data()),
               static_cast<std::streamsize>(chunk.blocks.size()));
}

} // namespace server_core
