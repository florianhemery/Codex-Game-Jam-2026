#pragma once

#include <variant>

#include "common/world/chunk.h"

namespace common::messages {

enum class ReliableMsgType : uint8_t {
    ChunkData,
};

struct ChunkDataMsg {
    common::world::ChunkCoord coord;
    std::array<uint8_t, common::world::CHUNK_BLOCK_COUNT> blocks;
};

struct ReliableMessage {
    ReliableMsgType type;
    std::variant<ChunkDataMsg> payload;
};

} // namespace common::messages
