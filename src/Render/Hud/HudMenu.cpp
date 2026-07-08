/*
** EPITECH PROJECT, 2026
** racer
** File description:
** HUD track selection menu
*/

#include "Render/Hud/HudMenu.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

#include "Track/Track.hpp"
#include "Render/Hud/HudGfx.hpp"
#include "Render/Hud/HudMinimap.hpp"

namespace racer {

namespace {

const char *ambianceLabelForTrack(int trackIndex, const TrackDef &def)
{
    if (def.surfaceStyle == SurfaceStyle::ABIMEE) {
        return "Pluie / route abimee";
    }
    switch (trackIndex % 3) {
    case 0:
        return "Midi ensoleille";
    case 1:
        return "Aube doree";
    default:
        return "Crepuscule";
    }
}

} // namespace

const HudTrackPreview &HudMenu::getTrackPreview(const TrackDef &def)
{
    static std::vector<HudTrackPreview> cache;

    for (const HudTrackPreview &entry : cache) {
        if (entry.key == def.name) {
            return entry;
        }
    }
    HudTrackPreview entry;

    entry.key = def.name;
    entry.points = Track::make(def).waypoints();
    cache.push_back(std::move(entry));
    return cache.back();
}

void HudMenu::drawTrackCardPreview(const TrackDef &def,
    const HudTrackPreview &preview,
    Rectangle inset, bool selected)
{
    if (preview.points.size() < 2) {
        return;
    }
    Rectangle area{
        inset.x + 10.0f, inset.y + 10.0f,
        inset.width - 20.0f, inset.height - 20.0f
    };
    HudMapProjection proj =
        HudMinimap::fitTrackInRect(preview.points, area, true);
    float road = std::clamp(def.width * proj.scale, 2.0f, 6.0f);

    HudMinimap::drawTrackPolyline(
        preview.points, proj, road, HudGfx::fade(WHITE, 0.14f));
    HudMinimap::drawTrackPolyline(
        preview.points, proj, 1.5f,
        HudGfx::fade(WHITE, selected ? 0.60f : 0.40f));
    HudGfx::drawCircleV(proj.apply(preview.points[0]), 3.5f,
        Color{88, 214, 104, 255});
}

void HudMenu::drawTrackCardBadge(const TrackDef &def, Rectangle card)
{
    if (def.surfaceStyle != SurfaceStyle::ABIMEE) {
        return;
    }
    const Color amber{255, 178, 48, 255};
    const char *tag = "ROUTE ABIMEE";
    int tagWidth = HudGfx::measureText(tag, 13);
    Rectangle badge{
        card.x + (card.width - static_cast<float>(tagWidth) - 20.0f) * 0.5f,
        card.y + card.height - 40.0f,
        static_cast<float>(tagWidth) + 20.0f, 22.0f
    };

    HudGfx::drawRectangleRounded(badge, 0.6f, 6, HudGfx::fade(amber, 0.16f));
    HudGfx::drawRectangleRoundedLinesEx(badge, 0.6f, 6, 1.0f,
        HudGfx::fade(amber, 0.85f));
    HudGfx::drawText(tag, static_cast<int>(badge.x + 10.0f),
        static_cast<int>(badge.y + 5.0f), 13, amber);
}

void HudMenu::drawTrackCard(const TrackDef &def, Rectangle card, bool selected,
    int trackIndex)
{
    HudGfx::drawRectangleRounded(card, 0.10f, 8,
        HudGfx::fade(BLACK, selected ? 0.55f : 0.42f));
    if (selected) {
        HudGfx::drawRectangleRoundedLinesEx(card, 0.10f, 8, 3.0f, YELLOW);
        Rectangle glow{
            card.x - 4.0f, card.y - 4.0f,
            card.width + 8.0f, card.height + 8.0f
        };
        HudGfx::drawRectangleRoundedLinesEx(glow, 0.10f, 8, 1.0f,
            HudGfx::fade(YELLOW, 0.30f));
    } else {
        HudGfx::drawRectangleRoundedLinesEx(card, 0.10f, 8, 1.0f,
            HudGfx::fade(WHITE, 0.12f));
    }

    float insetW = card.width - 90.0f;
    float insetH = insetW * 0.70f;
    Rectangle inset{
        card.x + (card.width - insetW) * 0.5f,
        card.y + 16.0f, insetW, insetH
    };

    HudGfx::drawRectangleRounded(inset, 0.12f, 6, HudGfx::fade(BLACK, 0.40f));
    HudGfx::drawRectangleRoundedLinesEx(inset, 0.12f, 6, 1.0f,
        HudGfx::fade(WHITE, 0.10f));

    const HudTrackPreview &preview = getTrackPreview(def);

    drawTrackCardPreview(def, preview, inset, selected);

    float textTop = inset.y + inset.height;

    HudGfx::drawTextCentered(def.name.c_str(),
        static_cast<int>(card.x + card.width * 0.5f),
        static_cast<int>(textTop + 14.0f),
        selected ? 22 : 20, selected ? YELLOW : RAYWHITE);
    HudWrappedTextParams wrapped{
        static_cast<int>(card.x + 16.0f),
        static_cast<int>(textTop + 46.0f),
        static_cast<int>(card.width - 32.0f),
        15, 19, HudGfx::fade(WHITE, 0.62f)
    };

    HudGfx::drawTextWrapped(def.description.c_str(), wrapped);

    const char *goal = "3 tours — 3 adversaires";
    HudGfx::drawTextCentered(goal,
        static_cast<int>(card.x + card.width * 0.5f),
        static_cast<int>(textTop + 92.0f), 14, HudGfx::fade(YELLOW, 0.85f));
    const char *ambiance = ambianceLabelForTrack(trackIndex, def);
    HudGfx::drawTextCentered(ambiance,
        static_cast<int>(card.x + card.width * 0.5f),
        static_cast<int>(textTop + 112.0f), 13, HudGfx::fade(WHITE, 0.50f));
    drawTrackCardBadge(def, card);
}

HudMenuLayout HudMenu::computeLayout(
    const std::vector<TrackDef> &presets, int screenWidth, int screenHeight)
{
    HudMenuLayout layout;
    int count = static_cast<int>(presets.size());

    if (count <= 0) {
        return layout;
    }
    const float gap = 20.0f;
    float cardW = std::min(250.0f,
        (static_cast<float>(screenWidth) - 60.0f -
            gap * static_cast<float>(count - 1)) /
        static_cast<float>(count));
    const float cardH = 280.0f;
    float totalW = cardW * static_cast<float>(count) +
        gap * static_cast<float>(count - 1);
    float x0 = (static_cast<float>(screenWidth) - totalW) * 0.5f;
    const float y0 = 246.0f;

    layout.cards.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
        Rectangle card{x0 + static_cast<float>(i) * (cardW + gap), y0,
            cardW, cardH};

        layout.cards.push_back(card);
    }
    const char *label = "DEMARRER";
    int labelW = HudGfx::measureText(label, 24);
    float btnW = static_cast<float>(labelW) + 48.0f;
    float btnH = 46.0f;

    layout.startButton = Rectangle{
        (static_cast<float>(screenWidth) - btnW) * 0.5f - 110.0f,
        static_cast<float>(screenHeight) - 108.0f, btnW, btnH,
    };
    layout.helpButton = Rectangle{
        (static_cast<float>(screenWidth) - btnW) * 0.5f + 110.0f,
        static_cast<float>(screenHeight) - 108.0f, btnW, btnH,
    };
    return layout;
}

int HudMenu::pickCard(const HudMenuLayout &layout, Vector2 mouse)
{
    for (size_t i = 0; i < layout.cards.size(); ++i) {
        if (CheckCollisionPointRec(mouse, layout.cards[i])) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool HudMenu::hitStartButton(const HudMenuLayout &layout, Vector2 mouse)
{
    return CheckCollisionPointRec(mouse, layout.startButton);
}

bool HudMenu::hitHelpButton(const HudMenuLayout &layout, Vector2 mouse)
{
    return CheckCollisionPointRec(mouse, layout.helpButton);
}

void HudMenu::drawStartButton(const HudMenuLayout &layout)
{
    const char *label = "DEMARRER";
    bool hover = CheckCollisionPointRec(GetMousePosition(), layout.startButton);
    Color fill = hover ? Color{255, 196, 40, 255} : Color{230, 150, 24, 255};

    HudGfx::drawRectangleRounded(layout.startButton, 0.35f, 8, fill);
    HudGfx::drawRectangleRoundedLinesEx(
        layout.startButton, 0.35f, 8, 2.0f, Fade(BLACK, 0.35f));
    HudGfx::drawTextCentered(label,
        static_cast<int>(layout.startButton.x + layout.startButton.width * 0.5f),
        static_cast<int>(layout.startButton.y + 12.0f), 24, BLACK);
}

void HudMenu::drawHelpButton(const HudMenuLayout &layout)
{
    const char *label = "COMMENT JOUER";
    bool hover = CheckCollisionPointRec(GetMousePosition(), layout.helpButton);
    Color fill = hover ? HudGfx::fade(WHITE, 0.22f) : HudGfx::fade(WHITE, 0.10f);

    HudGfx::drawRectangleRounded(layout.helpButton, 0.35f, 8, fill);
    HudGfx::drawRectangleRoundedLinesEx(
        layout.helpButton, 0.35f, 8, 2.0f, HudGfx::fade(WHITE, 0.35f));
    HudGfx::drawTextCentered(label,
        static_cast<int>(layout.helpButton.x + layout.helpButton.width * 0.5f),
        static_cast<int>(layout.helpButton.y + 12.0f), 20, RAYWHITE);
}

void HudMenu::drawHowToPlayOverlay(int screenWidth, int screenHeight)
{
    HudGfx::drawRectangle(0, 0, screenWidth, screenHeight,
        HudGfx::fade(BLACK, 0.78f));

    const float panelW = 620.0f;
    const float panelH = 360.0f;
    Rectangle panel{
        (static_cast<float>(screenWidth) - panelW) * 0.5f,
        (static_cast<float>(screenHeight) - panelH) * 0.5f,
        panelW, panelH
    };

    HudGfx::drawRectangleRounded(panel, 0.08f, 8, HudGfx::fade(BLACK, 0.70f));
    HudGfx::drawRectangleRoundedLinesEx(panel, 0.08f, 8, 2.0f,
        HudGfx::fade(ORANGE, 0.55f));
    HudGfx::drawTextCentered("Comment jouer", screenWidth / 2,
        static_cast<int>(panel.y + 18.0f), 32, ORANGE);

    const char *lines[] = {
        "Objectif : finir 3 tours en tete contre 3 adversaires.",
        "",
        "Z / W : accelerer          S : marche arriere",
        "Q / A : gauche             D : droite",
        "Shift : nitro (quand la jauge est pleine)",
        "Espace : drift / frein a main",
        "",
        "Echap : pause en course      F11 / Alt+Entree : plein ecran",
        "",
        "H ou clic hors panneau : fermer"
    };
    int y = static_cast<int>(panel.y + 72.0f);

    for (const char *line : lines) {
        HudGfx::drawTextCentered(line, screenWidth / 2, y, 18,
            HudGfx::fade(WHITE, line[0] == '\0' ? 0.0f : 0.82f));
        y += (line[0] == '\0') ? 12 : 26;
    }
}

void HudMenu::drawTitle(int screenWidth)
{
    const char *title = "RACER";
    const int titleSize = 78;
    int centerX = screenWidth / 2;
    int titleWidth = HudGfx::measureText(title, titleSize);

    HudGfx::drawText(title, centerX - titleWidth / 2 + 6, 46 + 6, titleSize,
        Color{110, 45, 16, 255});
    HudGfx::drawText(title, centerX - titleWidth / 2, 46, titleSize, ORANGE);
    HudGfx::drawLineEx(
        Vector2{static_cast<float>(centerX - titleWidth / 2), 134.0f},
        Vector2{static_cast<float>(centerX + titleWidth / 2), 134.0f},
        3.0f, HudGfx::fade(ORANGE, 0.55f));
    HudGfx::drawTextCentered("Choisissez un circuit", centerX, 152, 22,
        LIGHTGRAY);
}

void HudMenu::drawCards(const std::vector<TrackDef> &presets, int selectedIndex,
    int screenWidth)
{
    HudMenuLayout layout = computeLayout(presets, screenWidth, GetScreenHeight());
    int count = static_cast<int>(presets.size());

    for (int i = 0; i < count; ++i) {
        Rectangle card = layout.cards[static_cast<size_t>(i)];
        bool selected = (i == selectedIndex);

        if (selected) {
            card.x -= 7.0f;
            card.y -= 7.0f;
            card.width += 14.0f;
            card.height += 14.0f;
        }
        drawTrackCard(presets[static_cast<size_t>(i)], card, selected, i);
    }
}

HudMainMenuLayout HudMenu::computeMainLayout(int screenWidth, int screenHeight)
{
    HudMainMenuLayout layout;
    const float btnW = 340.0f;
    const float btnH = 52.0f;
    const float gap = 18.0f;
    float x = (static_cast<float>(screenWidth) - btnW) * 0.5f;
    float y = static_cast<float>(screenHeight) * 0.42f;

    layout.openWorldButton = Rectangle{x, y, btnW, btnH};
    layout.quickRaceButton = Rectangle{x, y + btnH + gap, btnW, btnH};
    layout.helpButton = Rectangle{x, y + (btnH + gap) * 2.0f, btnW, btnH};
    return layout;
}

bool HudMenu::hitOpenWorldButton(const HudMainMenuLayout &layout, Vector2 mouse)
{
    return CheckCollisionPointRec(mouse, layout.openWorldButton);
}

bool HudMenu::hitQuickRaceButton(const HudMainMenuLayout &layout, Vector2 mouse)
{
    return CheckCollisionPointRec(mouse, layout.quickRaceButton);
}

bool HudMenu::hitHelpButtonMain(const HudMainMenuLayout &layout, Vector2 mouse)
{
    return CheckCollisionPointRec(mouse, layout.helpButton);
}

void HudMenu::drawMainMenu(int screenWidth, int screenHeight, bool showHowToPlay)
{
    HudMainMenuLayout layout = computeMainLayout(screenWidth, screenHeight);
    Vector2 mouse = GetMousePosition();

    drawTitle(screenWidth);
    HudGfx::drawTextCentered("Choisissez votre mode de jeu", screenWidth / 2, 168,
        22, LIGHTGRAY);

    bool hoverWorld = CheckCollisionPointRec(mouse, layout.openWorldButton);
    bool hoverQuick = CheckCollisionPointRec(mouse, layout.quickRaceButton);
    bool hoverHelp = CheckCollisionPointRec(mouse, layout.helpButton);

    HudGfx::drawRectangleRounded(layout.openWorldButton, 0.35f, 8,
        hoverWorld ? Color{255, 196, 40, 255} : Color{230, 150, 24, 255});
    HudGfx::drawTextCentered("MONDE OUVERT",
        static_cast<int>(layout.openWorldButton.x + layout.openWorldButton.width * 0.5f),
        static_cast<int>(layout.openWorldButton.y + 14.0f), 26, BLACK);

    HudGfx::drawRectangleRounded(layout.quickRaceButton, 0.35f, 8,
        hoverQuick ? HudGfx::fade(WHITE, 0.22f) : HudGfx::fade(WHITE, 0.10f));
    HudGfx::drawTextCentered("COURSE RAPIDE",
        static_cast<int>(layout.quickRaceButton.x + layout.quickRaceButton.width * 0.5f),
        static_cast<int>(layout.quickRaceButton.y + 14.0f), 26, RAYWHITE);

    HudGfx::drawRectangleRounded(layout.helpButton, 0.35f, 8,
        hoverHelp ? HudGfx::fade(WHITE, 0.18f) : HudGfx::fade(WHITE, 0.08f));
    HudGfx::drawTextCentered("COMMENT JOUER",
        static_cast<int>(layout.helpButton.x + layout.helpButton.width * 0.5f),
        static_cast<int>(layout.helpButton.y + 16.0f), 22, RAYWHITE);

    HudGfx::drawTextCentered(
        "O : monde ouvert   |   Q : course rapide   |   H : aide",
        screenWidth / 2, screenHeight - 48, 18, GRAY);
    if (showHowToPlay) {
        drawHowToPlayOverlay(screenWidth, screenHeight);
    }
}

} // namespace racer
