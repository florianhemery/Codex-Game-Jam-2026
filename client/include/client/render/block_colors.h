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
        case common::world::BlockId::Gravel: return Color{136, 126, 118, 255};
        case common::world::BlockId::Water: return Color{64, 105, 224, 160};
        case common::world::BlockId::Wood: return Color{92, 64, 40, 255};
        case common::world::BlockId::Leaves: return Color{34, 139, 34, 255};
        case common::world::BlockId::Planks: return Color{188, 152, 98, 255};
        case common::world::BlockId::Stick: return Color{120, 90, 60, 255};
        case common::world::BlockId::Air:
        default:
            return BLANK;
    }
}

inline const char* NameForBlock(common::world::BlockId id) {
    switch (id) {
        case common::world::BlockId::Stone: return "Pierre";
        case common::world::BlockId::Dirt: return "Terre";
        case common::world::BlockId::Grass: return "Herbe";
        case common::world::BlockId::Sand: return "Sable";
        case common::world::BlockId::Gravel: return "Gravier";
        case common::world::BlockId::Water: return "Eau";
        case common::world::BlockId::Wood: return "Bois";
        case common::world::BlockId::Leaves: return "Feuilles";
        case common::world::BlockId::Planks: return "Planches";
        case common::world::BlockId::Stick: return "Baton";
        case common::world::BlockId::Air:
        default:
            return "";
    }
}

} // namespace client
