/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Pause-menu encyclopedia screen — "plaques des Veilleurs" lore entries
*/

#include "Render/Hud/HudEncyclopedia.hpp"

#include <cstdio>

#include "Render/Hud/HudGfx.hpp"
#include "World/Aurelia/AureliaData.hpp"

namespace racer {

namespace {

constexpr int kColumns = 4;
constexpr int kRows = 5;
constexpr int kEntryCount = kColumns * kRows;

const char *const kColumnLabels[kColumns] = {
    "MARINA", "FORET", "PORT", "VOLCAN"
};

// Entry index -> region column (matches AureliaData::kLoreSpots ordering:
// 0-4 Marina, 5-9 Foret, 10-14 Port, 15-19 Volcan).
int columnOf(int index)
{
    return index / kRows;
}

} // namespace

HudEncyclopediaLayout HudEncyclopedia::computeLayout(int screenWidth,
    int screenHeight)
{
    HudEncyclopediaLayout layout;

    layout.panel = Rectangle{0.0f, 0.0f, static_cast<float>(screenWidth),
        static_cast<float>(screenHeight)};

    const float margin = 60.0f;
    const float top = 130.0f;
    const float bottom = static_cast<float>(screenHeight) - 90.0f;
    const float gridW = (static_cast<float>(screenWidth) - margin * 2.0f) *
        0.56f;
    const float gridX = margin;
    const float gridH = bottom - top;

    const float colGap = 14.0f;
    const float rowGap = 10.0f;
    const float colW = (gridW - colGap * static_cast<float>(kColumns - 1)) /
        static_cast<float>(kColumns);
    const float rowH = (gridH - 30.0f - rowGap * static_cast<float>(kRows - 1)) /
        static_cast<float>(kRows);

    layout.tiles.reserve(kEntryCount);
    for (int col = 0; col < kColumns; ++col) {
        for (int row = 0; row < kRows; ++row) {
            Rectangle tile{
                gridX + static_cast<float>(col) * (colW + colGap),
                top + 30.0f + static_cast<float>(row) * (rowH + rowGap),
                colW, rowH
            };
            layout.tiles.push_back(tile);
        }
    }

    layout.detailPanel = Rectangle{
        gridX + gridW + 30.0f, top,
        static_cast<float>(screenWidth) - margin - (gridX + gridW + 30.0f),
        gridH
    };

    const char *backLabel = "RETOUR";
    int backW = HudGfx::measureText(backLabel, 20) + 48;
    layout.backButton = Rectangle{
        margin, static_cast<float>(screenHeight) - 66.0f,
        static_cast<float>(backW), 42.0f
    };

    return layout;
}

int HudEncyclopedia::pickTile(const HudEncyclopediaLayout &layout,
    Vector2 mouse)
{
    for (size_t i = 0; i < layout.tiles.size(); ++i) {
        if (CheckCollisionPointRec(mouse, layout.tiles[i])) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool HudEncyclopedia::hitBack(const HudEncyclopediaLayout &layout,
    Vector2 mouse)
{
    return CheckCollisionPointRec(mouse, layout.backButton);
}

void HudEncyclopedia::draw(int screenWidth, int screenHeight,
    const HudEncyclopediaLayout &layout,
    const world::ProgressionState *progression, int selectedIndex)
{
    HudGfx::drawRectangle(0, 0, screenWidth, screenHeight,
        HudGfx::fade(BLACK, 0.86f));

    HudGfx::drawTextCentered("ENCYCLOPEDIE DES VEILLEURS", screenWidth / 2,
        44, 34, ORANGE);

    int collected = progression ? progression->loreCollected() : 0;
    char subtitle[64];
    std::snprintf(subtitle, sizeof(subtitle),
        "%d / %d fragments decouverts", collected, kEntryCount);
    HudGfx::drawTextCentered(subtitle, screenWidth / 2, 88, 18,
        HudGfx::fade(WHITE, 0.75f));

    Vector2 mouse = GetMousePosition();

    // Column headers.
    for (int col = 0; col < kColumns; ++col) {
        const Rectangle &firstTile = layout.tiles[static_cast<size_t>(
            col * kRows)];
        HudGfx::drawTextCentered(kColumnLabels[col],
            static_cast<int>(firstTile.x + firstTile.width * 0.5f),
            static_cast<int>(firstTile.y - 26.0f), 16,
            HudGfx::fade(ORANGE, 0.85f));
    }

    for (int i = 0; i < kEntryCount; ++i) {
        const Rectangle &tile = layout.tiles[static_cast<size_t>(i)];
        bool unlocked = progression && progression->loreCollectedAt(i);
        bool hovered = CheckCollisionPointRec(mouse, tile);
        bool selected = (i == selectedIndex);

        Color fill = unlocked
            ? (selected ? Color{90, 70, 20, 255} : Color{40, 34, 20, 230})
            : Color{22, 22, 26, 210};
        Color border = selected ? YELLOW
            : (unlocked ? HudGfx::fade(ORANGE, hovered ? 0.9f : 0.55f)
                        : HudGfx::fade(WHITE, hovered ? 0.30f : 0.14f));

        HudGfx::drawRectangleRounded(tile, 0.18f, 6, fill);
        HudGfx::drawRectangleRoundedLinesEx(tile, 0.18f, 6,
            selected ? 2.5f : 1.5f, border);

        char indexLabel[8];
        std::snprintf(indexLabel, sizeof(indexLabel), "%02d", i + 1);
        HudGfx::drawText(indexLabel, static_cast<int>(tile.x + 8.0f),
            static_cast<int>(tile.y + 6.0f), 13,
            HudGfx::fade(WHITE, unlocked ? 0.55f : 0.30f));

        if (unlocked) {
            HudWrappedTextParams params{
                static_cast<int>(tile.x + 8.0f),
                static_cast<int>(tile.y + tile.height * 0.5f - 12.0f),
                static_cast<int>(tile.width - 16.0f),
                14, 16, RAYWHITE
            };
            HudGfx::drawTextWrapped(world::AureliaData::loreTitle(i), params);
        } else {
            HudGfx::drawTextCentered("???",
                static_cast<int>(tile.x + tile.width * 0.5f),
                static_cast<int>(tile.y + tile.height * 0.5f - 8.0f), 18,
                HudGfx::fade(WHITE, 0.35f));
        }
    }

    // Detail panel.
    HudGfx::drawRectangleRounded(layout.detailPanel, 0.06f, 8,
        HudGfx::fade(BLACK, 0.55f));
    HudGfx::drawRectangleRoundedLinesEx(layout.detailPanel, 0.06f, 8, 2.0f,
        HudGfx::fade(ORANGE, 0.45f));

    bool selectedUnlocked = selectedIndex >= 0 && selectedIndex < kEntryCount
        && progression && progression->loreCollectedAt(selectedIndex);

    if (selectedIndex < 0) {
        HudGfx::drawTextCentered("Selectionnez un fragment pour le lire.",
            static_cast<int>(layout.detailPanel.x + layout.detailPanel.width
                * 0.5f),
            static_cast<int>(layout.detailPanel.y + layout.detailPanel.height
                * 0.5f - 10.0f),
            18, HudGfx::fade(WHITE, 0.55f));
    } else if (!selectedUnlocked) {
        HudGfx::drawTextCentered("Fragment non decouvert.",
            static_cast<int>(layout.detailPanel.x + layout.detailPanel.width
                * 0.5f),
            static_cast<int>(layout.detailPanel.y + 40.0f), 20,
            HudGfx::fade(WHITE, 0.45f));
        HudGfx::drawTextCentered(
            "Explorez le monde ouvert pour trouver cette plaque.",
            static_cast<int>(layout.detailPanel.x + layout.detailPanel.width
                * 0.5f),
            static_cast<int>(layout.detailPanel.y + 72.0f), 15,
            HudGfx::fade(WHITE, 0.35f));
    } else {
        const char *title = world::AureliaData::loreTitle(selectedIndex);
        const char *text = world::AureliaData::loreText(selectedIndex);

        HudGfx::drawTextCentered(title,
            static_cast<int>(layout.detailPanel.x + layout.detailPanel.width
                * 0.5f),
            static_cast<int>(layout.detailPanel.y + 22.0f), 22, ORANGE);
        HudGfx::drawLineEx(
            Vector2{layout.detailPanel.x + 24.0f, layout.detailPanel.y + 58.0f},
            Vector2{layout.detailPanel.x + layout.detailPanel.width - 24.0f,
                layout.detailPanel.y + 58.0f},
            1.5f, HudGfx::fade(ORANGE, 0.35f));

        HudWrappedTextParams params{
            static_cast<int>(layout.detailPanel.x + 24.0f),
            static_cast<int>(layout.detailPanel.y + 78.0f),
            static_cast<int>(layout.detailPanel.width - 48.0f),
            17, 24, HudGfx::fade(WHITE, 0.90f)
        };
        HudGfx::drawTextWrapped(text, params);
    }

    // Back button.
    bool hoverBack = CheckCollisionPointRec(mouse, layout.backButton);
    HudGfx::drawRectangleRounded(layout.backButton, 0.35f, 8,
        hoverBack ? HudGfx::fade(WHITE, 0.22f) : HudGfx::fade(WHITE, 0.10f));
    HudGfx::drawRectangleRoundedLinesEx(layout.backButton, 0.35f, 8, 2.0f,
        HudGfx::fade(WHITE, 0.35f));
    HudGfx::drawTextCentered("RETOUR",
        static_cast<int>(layout.backButton.x + layout.backButton.width
            * 0.5f),
        static_cast<int>(layout.backButton.y + 11.0f), 20, RAYWHITE);

    HudGfx::drawTextCentered(
        "Fleches/souris : naviguer   |   Echap : retour",
        screenWidth / 2, screenHeight - 26, 15, GRAY);
}

} // namespace racer
