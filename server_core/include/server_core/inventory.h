#pragma once

#include <array>
#include <cstdint>

#include "common/messages/reliable_messages.h"

namespace server_core {

constexpr int kMaxStackSize = 64;

struct InventorySlot {
    uint8_t blockId = 0;
    uint16_t count = 0;
};

// Inventaire simple : 36 slots, pas de distinction hotbar/reste au niveau des
// donnees (les 9 premiers slots servent de hotbar cote client).
class Inventory {
public:
    // Ajoute jusqu'a `count` du bloc (stack existant en priorite, sinon
    // premier slot vide). Retourne la quantite reellement ajoutee.
    int AddBlock(uint8_t blockId, int count);

    // Retire `count` du bloc au slot indique. Echoue (retourne false, ne
    // modifie rien) si le slot ne contient pas assez de ce bloc precis.
    bool RemoveFromSlot(int slotIndex, uint8_t blockId, int count);

    const InventorySlot& Slot(int index) const { return slots_[static_cast<size_t>(index)]; }
    int SlotCount() const { return static_cast<int>(slots_.size()); }

    common::messages::InventoryUpdateMsg ToMessage() const;

    const std::array<InventorySlot, common::messages::kInventorySlotCount>& Slots() const { return slots_; }
    void SetSlots(const std::array<InventorySlot, common::messages::kInventorySlotCount>& slots) { slots_ = slots; }

private:
    std::array<InventorySlot, common::messages::kInventorySlotCount> slots_{};
};

} // namespace server_core
