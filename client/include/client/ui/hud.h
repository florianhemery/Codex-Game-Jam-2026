#pragma once

#include "common/messages/reliable_messages.h"

namespace client {

constexpr int kHotbarSlotCount = 9;

// Dessine les 9 premiers slots de l'inventaire en bas de l'ecran, slot
// `selectedSlot` mis en surbrillance.
void DrawHotbar(const common::messages::InventoryUpdateMsg& inventory, int selectedSlot, int screenWidth, int screenHeight);

} // namespace client
