#include "client/ui/crafting_ui.h"

#include <cstdio>

#include "raylib.h"

#include "client/render/block_colors.h"
#include "common/world/block.h"

namespace client {

namespace {
constexpr int kCellSize = 56;
constexpr int kCellPadding = 6;
constexpr int kGridCols = 3;
constexpr int kGridRows = 3;

Rectangle CellRect(int index, int screenWidth, int screenHeight) {
    int col = index % kGridCols;
    int row = index / kGridCols;
    int gridWidth = kGridCols * (kCellSize + kCellPadding) - kCellPadding;
    int startX = (screenWidth - gridWidth) / 2;
    int startY = screenHeight / 2 - (kGridRows * (kCellSize + kCellPadding)) / 2;
    return Rectangle{
        static_cast<float>(startX + col * (kCellSize + kCellPadding)),
        static_cast<float>(startY + row * (kCellSize + kCellPadding)),
        static_cast<float>(kCellSize),
        static_cast<float>(kCellSize),
    };
}

Rectangle CraftButtonRect(int screenWidth, int screenHeight) {
    int gridWidth = kGridCols * (kCellSize + kCellPadding) - kCellPadding;
    int startX = (screenWidth - gridWidth) / 2;
    int startY = screenHeight / 2 - (kGridRows * (kCellSize + kCellPadding)) / 2;
    int gridBottom = startY + kGridRows * (kCellSize + kCellPadding);
    return Rectangle{static_cast<float>(startX), static_cast<float>(gridBottom + 16), static_cast<float>(gridWidth), 36.0f};
}
} // namespace

void OpenCraftingUI(CraftingUIState& state) {
    state.open = true;
    state.gridSlots.fill(common::messages::kCraftSlotEmpty);
}

void CloseCraftingUI(CraftingUIState& state) {
    state.open = false;
}

bool UpdateCraftingUI(CraftingUIState& state, int selectedHotbarSlot, int screenWidth, int screenHeight,
                      common::messages::CraftRequestMsg& outRequest) {
    if (!state.open) return false;

    if (state.statusMessageTimer > 0.0f) state.statusMessageTimer -= GetFrameTime();

    Vector2 mouse = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        for (int i = 0; i < 9; ++i) {
            if (CheckCollisionPointRec(mouse, CellRect(i, screenWidth, screenHeight))) {
                if (state.gridSlots[static_cast<size_t>(i)] == common::messages::kCraftSlotEmpty) {
                    state.gridSlots[static_cast<size_t>(i)] = static_cast<uint8_t>(selectedHotbarSlot);
                } else {
                    state.gridSlots[static_cast<size_t>(i)] = common::messages::kCraftSlotEmpty;
                }
                return false;
            }
        }

        if (CheckCollisionPointRec(mouse, CraftButtonRect(screenWidth, screenHeight))) {
            outRequest.gridSlots = state.gridSlots;
            return true;
        }
    }

    return false;
}

void OnCraftResponse(CraftingUIState& state, const common::messages::CraftResponseMsg& response) {
    state.statusMessageTimer = 2.5f;
    if (response.success) {
        auto id = static_cast<common::world::BlockId>(response.resultBlockId);
        char buf[96];
        std::snprintf(buf, sizeof(buf), "Fabrique : %d x %s", response.resultCount, NameForBlock(id));
        state.statusMessage = buf;
        state.gridSlots.fill(common::messages::kCraftSlotEmpty); // les items ont ete consommes
    } else {
        state.statusMessage = "Recette non reconnue";
    }
}

void DrawCraftingUI(const CraftingUIState& state, const common::messages::InventoryUpdateMsg& inventory,
                    int screenWidth, int screenHeight) {
    if (!state.open) return;

    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.5f));

    for (int i = 0; i < 9; ++i) {
        Rectangle rect = CellRect(i, screenWidth, screenHeight);
        DrawRectangleRec(rect, Fade(BLACK, 0.5f));
        DrawRectangleLinesEx(rect, 2.0f, LIGHTGRAY);

        uint8_t slotIdx = state.gridSlots[static_cast<size_t>(i)];
        if (slotIdx != common::messages::kCraftSlotEmpty && slotIdx < inventory.slots.size()) {
            const auto& slot = inventory.slots[slotIdx];
            if (slot.blockId != 0 && slot.count > 0) {
                auto id = static_cast<common::world::BlockId>(slot.blockId);
                DrawRectangle(static_cast<int>(rect.x) + 8, static_cast<int>(rect.y) + 8, kCellSize - 16, kCellSize - 16,
                              ColorForBlock(id));
            }
        }
    }

    Rectangle craftBtn = CraftButtonRect(screenWidth, screenHeight);
    DrawRectangleRec(craftBtn, Fade(DARKGREEN, 0.8f));
    DrawRectangleLinesEx(craftBtn, 2.0f, LIGHTGRAY);
    DrawText("Crafter", static_cast<int>(craftBtn.x) + 10, static_cast<int>(craftBtn.y) + 8, 20, WHITE);

    DrawText("C pour fermer -- clic sur une case: pose/retire le slot hotbar selectionne",
             static_cast<int>(craftBtn.x), static_cast<int>(craftBtn.y) + 50, 16, LIGHTGRAY);

    if (state.statusMessageTimer > 0.0f && !state.statusMessage.empty()) {
        DrawText(state.statusMessage.c_str(), static_cast<int>(craftBtn.x), static_cast<int>(craftBtn.y) - 30, 20, YELLOW);
    }
}

} // namespace client
