#include "server_core/inventory.h"

namespace server_core {

int Inventory::AddBlock(uint8_t blockId, int count) {
    int remaining = count;

    for (auto& slot : slots_) {
        if (remaining <= 0) break;
        if (slot.blockId == blockId && slot.count < kMaxStackSize) {
            int room = kMaxStackSize - slot.count;
            int added = room < remaining ? room : remaining;
            slot.count = static_cast<uint16_t>(slot.count + added);
            remaining -= added;
        }
    }

    for (auto& slot : slots_) {
        if (remaining <= 0) break;
        if (slot.blockId == 0) {
            int added = remaining < kMaxStackSize ? remaining : kMaxStackSize;
            slot.blockId = blockId;
            slot.count = static_cast<uint16_t>(added);
            remaining -= added;
        }
    }

    return count - remaining;
}

bool Inventory::RemoveFromSlot(int slotIndex, uint8_t blockId, int count) {
    if (slotIndex < 0 || slotIndex >= static_cast<int>(slots_.size())) return false;

    InventorySlot& slot = slots_[static_cast<size_t>(slotIndex)];
    if (slot.blockId != blockId || slot.count < count) return false;

    slot.count = static_cast<uint16_t>(slot.count - count);
    if (slot.count == 0) slot.blockId = 0;
    return true;
}

common::messages::InventoryUpdateMsg Inventory::ToMessage() const {
    common::messages::InventoryUpdateMsg msg;
    for (size_t i = 0; i < slots_.size(); ++i) {
        msg.slots[i].blockId = slots_[i].blockId;
        msg.slots[i].count = slots_[i].count;
    }
    return msg;
}

} // namespace server_core
