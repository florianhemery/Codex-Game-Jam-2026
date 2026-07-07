/*
** EPITECH PROJECT, 2026
** racer
** File description:
** HUD rendering for race and menu screens
*/

#include "Render/Hud/Hud.hpp"

#include "Race/RaceState.hpp"
#include "Render/Hud/HudFinishScreen.hpp"
#include "Render/Hud/HudGfx.hpp"
#include "Render/Hud/HudMenu.hpp"
#include "Render/Hud/HudMinimap.hpp"
#include "Render/Hud/HudRaceOverlay.hpp"

namespace racer {

void Hud::draw(const RaceState &race, int screenWidth, int screenHeight)
{
    drawHudEx(race, screenWidth, screenHeight, HudExtras{});
}

void Hud::drawHudEx(const RaceState &race, int screenWidth, int screenHeight,
    const HudExtras &extras)
{
    const std::vector<RacerEntry> &racers = race.racers();
    const RacerEntry &player = racers[static_cast<size_t>(race.playerIndex())];

    HudRaceOverlay::drawStandingsPanel(race, extras);
    HudRaceOverlay::drawTimersPanel(race, extras, screenWidth);
    HudRaceOverlay::drawSpeedGauge(player.car, screenHeight);
    HudMinimap::draw(race, extras, screenWidth, screenHeight);

    switch (race.phase()) {
        case RacePhase::COUNTDOWN:
            HudRaceOverlay::drawCountdown(race, screenWidth, screenHeight);
            break;
        case RacePhase::RACING:
            HudRaceOverlay::drawGoFlash(race, screenWidth, screenHeight);
            break;
        case RacePhase::FINISHED:
            HudFinishScreen::draw(race, extras, screenWidth, screenHeight);
            break;
    }
}

void Hud::drawMenu(const std::vector<TrackDef> &presets, int selectedIndex,
    int screenWidth, int screenHeight)
{
    HudGfx::clearBackground(Color{15, 17, 26, 255});
    HudGfx::drawRectangleGradientV(0, 0, screenWidth, screenHeight,
        Color{22, 26, 40, 255}, Color{10, 11, 18, 255});

    int centerX = screenWidth / 2;

    HudMenu::drawTitle(screenWidth);
    HudMenu::drawCards(presets, selectedIndex, screenWidth);
    HudGfx::drawTextCentered(
        "Haut/Bas : choisir   --   Entree : demarrer", centerX,
        screenHeight - 56, 20, GRAY);
}

} // namespace racer
