/*
** EPITECH PROJECT, 2026
** racer
** File description:
** HUD rendering for race and menu screens
*/

#include "Render/Hud/Hud.hpp"

#include <cstdio>
#include <cmath>

#include "Race/RaceState.hpp"
#include "Render/Hud/HudFinishScreen.hpp"
#include "Render/Hud/HudGfx.hpp"
#include "Render/Hud/HudMenu.hpp"
#include "Render/Hud/HudMinimap.hpp"
#include "Render/Hud/HudOpenWorldNav.hpp"
#include "Render/Hud/HudRaceOverlay.hpp"
#include "World/Aurelia/AureliaWorld.hpp"
#include "World/Chunk/ChunkTypes.hpp"

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
    HudRaceOverlay::drawSpeedGauge(player.car, screenHeight, extras);
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
    default:
        break;
    }
}

void Hud::drawMenu(const std::vector<TrackDef> &presets, int selectedIndex,
    int screenWidth, int screenHeight, bool showHowToPlay)
{
    HudGfx::clearBackground(Color{15, 17, 26, 255});
    HudGfx::drawRectangleGradientV(0, 0, screenWidth, screenHeight,
        Color{22, 26, 40, 255}, Color{10, 11, 18, 255});

    int centerX = screenWidth / 2;
    HudMenuLayout layout =
        HudMenu::computeLayout(presets, screenWidth, screenHeight);

    HudMenu::drawTitle(screenWidth);
    HudMenu::drawCards(presets, selectedIndex, screenWidth);
    HudMenu::drawStartButton(layout);
    HudGfx::drawTextCentered(
        "Haut/Bas ou clic : choisir   --   Entree : demarrer"
        "   --   F11 : plein ecran",
        centerX, screenHeight - 56, 20, GRAY);
    if (showHowToPlay) {
        HudMenu::drawHowToPlayOverlay(screenWidth, screenHeight);
    }
}

void Hud::drawMainMenu(int screenWidth, int screenHeight, bool showHowToPlay)
{
    HudGfx::clearBackground(Color{15, 17, 26, 255});
    HudGfx::drawRectangleGradientV(0, 0, screenWidth, screenHeight,
        Color{22, 26, 40, 255}, Color{10, 11, 18, 255});
    HudMenu::drawMainMenu(screenWidth, screenHeight, showHowToPlay);
}

void Hud::drawQuickRaceOverlay(const std::vector<TrackDef> &presets,
    int selectedIndex, int screenWidth, int screenHeight)
{
    HudMenuLayout layout =
        HudMenu::computeLayout(presets, screenWidth, screenHeight);
    HudMenu::drawTitle(screenWidth);
    HudMenu::drawCards(presets, selectedIndex, screenWidth);
    HudMenu::drawStartButton(layout);
    HudGfx::drawTextCentered(
        "Tab / Echap : fermer   --   Entree : demarrer",
        screenWidth / 2, screenHeight - 56, 18, GRAY);
}

namespace {

const char *safeLabel(const char *label, const char *fallback)
{
    if (label != nullptr && label[0] != '\0') {
        return label;
    }
    return fallback;
}

} // namespace

void Hud::drawOpenWorldHud(const world::AureliaWorld &world,
    const std::vector<TrackDef> &presets, int screenWidth, int screenHeight)
{
    (void)presets;
    HudGfx::drawText("AURELIA", 16, 16, 24, ORANGE);
    HudGfx::drawText("Tab : course rapide   |   M : grande carte   |   Echap : menu",
        16, 46, 16, HudGfx::fade(WHITE, 0.75f));
    HudGfx::drawText("Mini-carte en bas a gauche — fleche = circuit le plus proche",
        16, 66, 14, HudGfx::fade(GRAY, 0.85f));
    HudGfx::drawText(HudGfx::colorblindMode()
            ? "F1 : mode daltonien (actif)   |   F2 : remapper les touches"
            : "F1 : mode daltonien   |   F2 : remapper les touches",
        16, 172, 12, HudGfx::fade(GRAY, 0.7f));

    char biomeBuf[96];
    std::snprintf(biomeBuf, sizeof(biomeBuf), "Biome : %s",
        world::biomeLabel(world.currentBiome()));
    HudGfx::drawText(biomeBuf, 16, 88, 16, SKYBLUE);

    const world::PoiInstance *race = world.activeRacePoi();
    if (race) {
        char buf[96];
        std::snprintf(buf, sizeof(buf), "Circuit : %s",
            safeLabel(race->label, "Circuit"));
        HudGfx::drawTextCentered(buf, screenWidth / 2, screenHeight - 96, 24,
            YELLOW);
        HudGfx::drawTextCentered("Entree / E : lancer la course",
            screenWidth / 2, screenHeight - 64, 18,
            HudGfx::fade(WHITE, 0.80f));
    } else {
        float nearestDist = 0.0f;
        const world::PoiInstance *nearest = nullptr;
        Vector3 pos = world.playerCar().position();
        for (const world::PoiInstance &poi : world.pois()) {
            if (poi.type != world::PoiType::RACE_ENTRY) {
                continue;
            }
            if (poi.trackIndex == 4
                && !world.progression().cendresCircuitUnlocked()) {
                continue;
            }
            float dx = poi.worldX - pos.x;
            float dz = poi.worldZ - pos.z;
            float d = std::sqrt(dx * dx + dz * dz);
            if (nearest == nullptr || d < nearestDist) {
                nearestDist = d;
                nearest = &poi;
            }
        }
        if (nearest != nullptr) {
            char nbuf[128];
            std::snprintf(nbuf, sizeof(nbuf),
                "Suivez la fleche vers %s (%.0f m)",
                safeLabel(nearest->label, "Circuit"), nearestDist);
            HudGfx::drawTextCentered(nbuf, screenWidth / 2, screenHeight - 96,
                16, HudGfx::fade(WHITE, 0.70f));
        }
    }

    const world::PoiInstance *mission = world.activeMissionPoi();
    if (mission) {
        char mbuf[96];
        std::snprintf(mbuf, sizeof(mbuf), "Mission : %s (F pour accepter)",
            safeLabel(mission->label, "Mission"));
        HudGfx::drawTextCentered(mbuf, screenWidth / 2, screenHeight - 128, 16,
            GREEN);
    }

    const world::PoiInstance *garage = world.activeGaragePoi();
    if (garage) {
        world::RegionId garageRegion = garage->region;
        char gbuf[160];
        std::snprintf(gbuf, sizeof(gbuf),
            "Garage (rep %d) : G moteur(%d) / H grip(%d) / J freins(%d)",
            world.progression().reputation(garageRegion),
            world.tuningUpgradeCost(world::TuningCategory::ENGINE),
            world.tuningUpgradeCost(world::TuningCategory::GRIP),
            world.tuningUpgradeCost(world::TuningCategory::BRAKES));
        HudGfx::drawTextCentered(gbuf, screenWidth / 2, screenHeight - 150, 16,
            SKYBLUE);
    }

    int activeMission = world.missions().activeMissionIndex();
    if (activeMission >= 0) {
        char tbuf[64];
        std::snprintf(tbuf, sizeof(tbuf), "Chrono mission : %.0fs",
            world.missions().activeTimer());
        HudGfx::drawText(tbuf, 16, 112, 16, LIME);
    }

    world::RegionId region = world::regionForBiome(world.currentBiome());
    char repBuf[64];
    std::snprintf(repBuf, sizeof(repBuf), "Rep %s : %d",
        world::regionLabel(world.currentBiome()),
        world.progression().reputation(region));
    HudGfx::drawText(repBuf, 16, 134, 14, GOLD);

    char loreBuf[48];
    std::snprintf(loreBuf, sizeof(loreBuf), "Lore : %d/20",
        world.progression().loreCollected());
    HudGfx::drawText(loreBuf, 16, 152, 14, Color{255, 220, 120, 255});

    HudOpenWorldNav::drawPersistentMinimap(world, screenWidth, screenHeight);
    HudOpenWorldNav::drawCompass(world, screenWidth, screenHeight);
    HudOpenWorldNav::drawEdgeWarning(world, screenWidth, screenHeight);

    if (world.showWorldMap()) {
        HudOpenWorldNav::drawExpandedMap(world, screenWidth, screenHeight);
    }
}

void Hud::drawPauseOverlay(int screenWidth, int screenHeight)
{
    HudRaceOverlay::drawPauseOverlay(screenWidth, screenHeight);
}

} // namespace racer
