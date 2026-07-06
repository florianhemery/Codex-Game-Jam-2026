#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "common/messages/reliable_messages.h"

namespace client {

struct CraftingUIState {
    bool open = false;
    std::array<uint8_t, 9> gridSlots{}; // kCraftSlotEmpty = case vide
    std::string statusMessage;
    float statusMessageTimer = 0.0f;
};

void OpenCraftingUI(CraftingUIState& state);
void CloseCraftingUI(CraftingUIState& state);

// Gere les clics sur la grille et le bouton "Crafter". Renvoie true si une
// CraftRequestMsg doit etre envoyee (remplit outRequest dans ce cas).
bool UpdateCraftingUI(CraftingUIState& state, int selectedHotbarSlot, int screenWidth, int screenHeight,
                      common::messages::CraftRequestMsg& outRequest);

void OnCraftResponse(CraftingUIState& state, const common::messages::CraftResponseMsg& response);

void DrawCraftingUI(const CraftingUIState& state, const common::messages::InventoryUpdateMsg& inventory,
                    int screenWidth, int screenHeight);

} // namespace client
