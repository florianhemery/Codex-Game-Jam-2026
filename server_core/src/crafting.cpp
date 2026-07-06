#include "server_core/crafting.h"

#include <array>
#include <vector>

#include "common/world/block.h"

namespace server_core {

namespace {

using common::world::BlockId;

constexpr uint8_t B(BlockId id) { return static_cast<uint8_t>(id); }
constexpr uint8_t E = 0; // case vide (Air)

const std::vector<CraftingRecipe>& Recipes() {
    static const std::vector<CraftingRecipe> recipes = {
        // Une buche au centre, tout le reste vide -> 4 planches.
        {{E, E, E,
          E, B(BlockId::Wood), E,
          E, E, E},
         B(BlockId::Planks), 4},

        // Deux planches empilees dans la colonne du milieu -> 4 batons.
        {{E, B(BlockId::Planks), E,
          E, B(BlockId::Planks), E,
          E, E, E},
         B(BlockId::Stick), 4},
    };
    return recipes;
}

} // namespace

bool TryCraft(const std::array<uint8_t, 9>& grid, uint8_t& outBlockId, uint16_t& outCount) {
    for (const CraftingRecipe& recipe : Recipes()) {
        if (recipe.pattern == grid) {
            outBlockId = recipe.resultBlockId;
            outCount = recipe.resultCount;
            return true;
        }
    }
    return false;
}

} // namespace server_core
