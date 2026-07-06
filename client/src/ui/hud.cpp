#include "client/ui/hud.h"

#include <cstdio>

#include "raylib.h"

#include "client/render/block_colors.h"
#include "common/world/block.h"

namespace client {

void DrawHotbar(const common::messages::InventoryUpdateMsg& inventory, int selectedSlot, int screenWidth, int screenHeight) {
    const int slotSize = 56;
    const int padding = 6;
    const int totalWidth = kHotbarSlotCount * (slotSize + padding) - padding;
    const int startX = (screenWidth - totalWidth) / 2;
    const int y = screenHeight - slotSize - 20;

    for (int i = 0; i < kHotbarSlotCount; ++i) {
        int x = startX + i * (slotSize + padding);
        bool selected = (i == selectedSlot);

        DrawRectangle(x, y, slotSize, slotSize, Fade(BLACK, 0.4f));
        DrawRectangleLinesEx(Rectangle{static_cast<float>(x), static_cast<float>(y),
                                       static_cast<float>(slotSize), static_cast<float>(slotSize)},
                              selected ? 3.0f : 1.0f, selected ? YELLOW : LIGHTGRAY);

        const auto& slot = inventory.slots[static_cast<size_t>(i)];
        if (slot.blockId != 0 && slot.count > 0) {
            auto id = static_cast<common::world::BlockId>(slot.blockId);
            DrawRectangle(x + 8, y + 8, slotSize - 16, slotSize - 16, ColorForBlock(id));

            char countText[8];
            std::snprintf(countText, sizeof(countText), "%d", slot.count);
            DrawText(countText, x + slotSize - 20, y + slotSize - 20, 16, WHITE);
        }
    }
}

} // namespace client
