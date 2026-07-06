#pragma once

#include "raylib.h"

#include "common/world/block.h"

namespace client {

inline Color ColorForBlock(common::world::BlockId id) {
    switch (id) {
        case common::world::BlockId::Stone: return GRAY;
        case common::world::BlockId::Dirt: return BROWN;
        case common::world::BlockId::Grass: return GREEN;
        case common::world::BlockId::Sand: return Color{237, 201, 175, 255};
        case common::world::BlockId::Air:
        default:
            return BLANK;
    }
}

} // namespace client
