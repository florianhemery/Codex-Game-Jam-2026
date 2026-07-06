#pragma once

#include <variant>

#include "common/world/chunk.h"

namespace common::messages {

enum class ReliableMsgType : uint8_t {
    ChunkData,
    ChunkUnload,
};

struct ChunkDataMsg {
    common::world::ChunkCoord coord;
    std::array<uint8_t, common::world::CHUNK_BLOCK_COUNT> blocks;
};

struct ChunkUnloadMsg {
    common::world::ChunkCoord coord;
};

struct ReliableMessage {
    ReliableMsgType type;
    std::variant<ChunkDataMsg, ChunkUnloadMsg> payload;
};

} // namespace common::messages
