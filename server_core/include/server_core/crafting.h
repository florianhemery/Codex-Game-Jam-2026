#pragma once

#include <array>
#include <cstdint>

namespace server_core {

// Grille 3x3, ligne-majeur (index = row*3+col), 0 = case vide. Recherche par
// egalite stricte -- pas d'invariance par decalage (la grille EST la zone de
// crafting, pas une sous-fenetre d'un inventaire plus grand).
struct CraftingRecipe {
    std::array<uint8_t, 9> pattern;
    uint8_t resultBlockId;
    uint16_t resultCount;
};

// Renvoie true et remplit outBlockId/outCount si la grille correspond a une
// recette connue.
bool TryCraft(const std::array<uint8_t, 9>& grid, uint8_t& outBlockId, uint16_t& outCount);

} // namespace server_core
