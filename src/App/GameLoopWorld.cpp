/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Open world + main menu loop helpers (Aurelia)
*/

#include <algorithm>
#include <cmath>

#include "raylib.h"

#include "App/AudioFeedback.hpp"
#include "App/CameraController.hpp"
#include "App/GameLoop.hpp"
#include "App/GameLoopInput.hpp"
#include "App/RacerColors.hpp"
#include "Engine/Input/InputBindings.hpp"
#include "Render/Hud.hpp"
#include "Render/Hud/HudDebugOverlay.hpp"
#include "Render/Hud/HudEncyclopedia.hpp"
#include "Render/Hud/HudMenu.hpp"
#include "Vehicle/Car.hpp"
#include "World/Aurelia/AureliaWorld.hpp"
#include "World/Aurelia/AureliaTypes.hpp"

namespace racer {
namespace app {

void GameLoop::enterOpenWorld(Context &ctx)
{
    const world::ProgressionState *saved = nullptr;
    if (ctx.openWorldProgressionBackup) {
        saved = &*ctx.openWorldProgressionBackup;
    }
    ctx.aurelia = std::make_unique<world::AureliaWorld>(
        ctx.presets, ctx.openWorldRespawnRegion, saved);
    if (ctx.pipeline) {
        ctx.aurelia->applyShader(ctx.pipeline->litShader());
    }
    ctx.openWorldProgressionBackup.reset();
    ctx.currentAmbiance = ctx.aurelia->ambianceForTime();
    if (ctx.pipeline) {
        ctx.pipeline->setAmbiance(ctx.currentAmbiance);
    }
    ctx.trackRenderer.reset();
    ctx.race.reset();
    ctx.steerSmoothed = 0.0f;
    ctx.wheelSpin = 0.0f;
    ctx.vfx->clear();
    ctx.vfx->setRain(false);
    ctx.racePaused = false;
    ctx.openWorldQuickRace = false;
    ctx.appState = AppState::OPEN_WORLD;
}

void GameLoop::leaveRace(Context &ctx)
{
    const bool toHub = ctx.raceFromOpenWorld;

    ctx.race.reset();
    ctx.raceFromOpenWorld = false;
    ctx.racePaused = false;
    if (toHub) {
        enterOpenWorld(ctx);
    } else {
        ctx.appState = AppState::MENU;
        ctx.menuScreen = MenuScreen::MAIN;
        ctx.trackRenderer.reset();
        ctx.aurelia.reset();
    }
}

bool GameLoop::handleMainMenuFrame(Context &ctx)
{
    HudMainMenuLayout layout = HudMenu::computeMainLayout(
        ctx.screenWidth, ctx.screenHeight);
    Vector2 mouse = GetMousePosition();

    if (IsKeyPressed(KEY_H) || charJustPressed('h', 'H')) {
        ctx.showHowToPlay = !ctx.showHowToPlay;
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !ctx.showHowToPlay) {
        if (HudMenu::hitOpenWorldButton(layout, mouse)) {
            enterOpenWorld(ctx);
            return true;
        }
        if (HudMenu::hitQuickRaceButton(layout, mouse)) {
            ctx.menuScreen = MenuScreen::QUICK_RACE;
        }
        if (HudMenu::hitEncyclopediaButton(layout, mouse)) {
            ctx.menuScreen = MenuScreen::ENCYCLOPEDIA;
            ctx.encyclopediaSelected = -1;
        }
        if (HudMenu::hitHelpButtonMain(layout, mouse)) {
            ctx.showHowToPlay = true;
        }
    }
    if (IsKeyPressed(KEY_O) || charJustPressed('o', 'O')) {
        enterOpenWorld(ctx);
        return true;
    }
    if (IsKeyPressed(KEY_Q) || charJustPressed('q', 'Q')) {
        ctx.menuScreen = MenuScreen::QUICK_RACE;
    }
    if (!ctx.showHowToPlay
        && (IsKeyPressed(KEY_E) || charJustPressed('e', 'E'))) {
        ctx.menuScreen = MenuScreen::ENCYCLOPEDIA;
        ctx.encyclopediaSelected = -1;
    }

    BeginDrawing();
    ClearBackground(Color{20, 24, 36, 255});
    Hud hud;
    hud.drawMainMenu(ctx.screenWidth, ctx.screenHeight, ctx.showHowToPlay);
    engine::input::InputBindings::instance().drawDebugRemapOverlay(
        ctx.screenWidth, ctx.screenHeight);
    hud::HudDebugOverlay::instance().draw(ctx.screenWidth, ctx.screenHeight);
    EndDrawing();
    return ctx.appState == AppState::MENU;
}

bool GameLoop::handleOpenWorldFrame(Context &ctx, float dt)
{
    if (ctx.openWorldQuickRace) {
        return handleQuickRaceOverlay(ctx);
    }
    if (IsKeyPressed(KEY_TAB)) {
        ctx.openWorldQuickRace = true;
        return true;
    }
    if (engine::input::InputBindings::instance().isPressed(
            engine::input::Action::ToggleMap)) {
        ctx.aurelia->toggleWorldMap();
    }
    if (engine::input::InputBindings::instance().isPressed(
            engine::input::Action::Menu)) {
        ctx.appState = AppState::MENU;
        ctx.menuScreen = MenuScreen::MAIN;
        ctx.aurelia.reset();
        ctx.trackRenderer.reset();
        return true;
    }

    CarInput input{};
    float steerTarget = readSteerTarget();

    if (engine::input::InputBindings::instance().isHeld(
            engine::input::Action::Accelerate)) {
        input.throttle = 1.0f;
    } else if (engine::input::InputBindings::instance().isHeld(
            engine::input::Action::Brake)) {
        input.throttle = -1.0f;
    }
    ctx.steerSmoothed += (steerTarget - ctx.steerSmoothed)
        * std::min(1.0f, 8.0f * dt);
    input.steer = ctx.steerSmoothed;
    input.handbrake = engine::input::InputBindings::instance().isHeld(
        engine::input::Action::Handbrake);
    input.nitro = engine::input::InputBindings::instance().isHeld(
        engine::input::Action::Nitro);
    ctx.aurelia->update(dt, input, ctx.steerSmoothed, ctx.vfx.get());

    AudioDriveSnapshot audioSnap{};
    const Car &owCar = ctx.aurelia->playerCar();
    audioSnap.input = input;
    audioSnap.speed = owCar.speed();
    audioSnap.heading = owCar.heading();
    audioSnap.velocityHeading = owCar.velocityHeading();
    audioSnap.maxSpeed = owCar.tuning().maxSpeed;
    audioSnap.surfaceDrag = owCar.surfaceDrag();
    audioSnap.nitroActive = input.nitro && owCar.nitroRemaining() > 0.0f;
    audioSnap.drifting = std::fabs(input.steer) > 0.35f && owCar.speed() > 6.0f;
    ctx.audio.updateOpenWorld(audioSnap, dt, ctx.aurelia->currentBiome(), false);

    ctx.currentAmbiance = ctx.aurelia->ambianceForTime();
    if (ctx.pipeline) {
        ctx.pipeline->setAmbiance(ctx.currentAmbiance);
        ctx.pipeline->setFogDensity(ctx.aurelia->pipelineFogDensity());
    }

    if (ctx.aurelia->activeRaceIndex() >= 0
        && (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_E))) {
        startRace(ctx, ctx.aurelia->activeRaceIndex(), true);
        return true;
    }

    const world::PoiInstance *missionPoi = ctx.aurelia->activeMissionPoi();
    if (missionPoi && missionPoi->missionIndex >= 0 && IsKeyPressed(KEY_F)) {
        ctx.aurelia->missions().tryStartMission(missionPoi->missionIndex);
    }

    if (ctx.aurelia->activeGaragePoi() != nullptr) {
        if (IsKeyPressed(KEY_G)) {
            ctx.aurelia->purchaseUpgrade(world::TuningCategory::ENGINE);
        }
        if (IsKeyPressed(KEY_H)) {
            ctx.aurelia->purchaseUpgrade(world::TuningCategory::GRIP);
        }
        if (IsKeyPressed(KEY_J)) {
            ctx.aurelia->purchaseUpgrade(world::TuningCategory::BRAKES);
        }
    }

    updateCamera(ctx.camera, ctx.aurelia->playerCar(), dt);

    BeginDrawing();
    renderWorld(ctx);
    Hud hud;
    hud.drawOpenWorldHud(*ctx.aurelia, ctx.presets, ctx.screenWidth,
        ctx.screenHeight);
    engine::input::InputBindings::instance().drawDebugRemapOverlay(
        ctx.screenWidth, ctx.screenHeight);
    hud::HudDebugOverlay::instance().draw(ctx.screenWidth, ctx.screenHeight);
    EndDrawing();
    return true;
}

bool GameLoop::handleQuickRaceOverlay(Context &ctx)
{
    int count = static_cast<int>(ctx.presets.size());
    HudMenuLayout layout = HudMenu::computeLayout(
        ctx.presets, ctx.screenWidth, ctx.screenHeight);
    Vector2 mouse = GetMousePosition();

    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_TAB)) {
        ctx.openWorldQuickRace = false;
        if (ctx.appState == AppState::MENU) {
            ctx.menuScreen = MenuScreen::MAIN;
        }
        return true;
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        int picked = HudMenu::pickCard(layout, mouse);
        if (picked >= 0) {
            ctx.selectedTrack = picked;
        } else if (HudMenu::hitStartButton(layout, mouse)) {
            if (ctx.appState == AppState::OPEN_WORLD) {
                startRace(ctx, ctx.selectedTrack, true);
            } else {
                startRace(ctx, ctx.selectedTrack, false);
            }
            ctx.openWorldQuickRace = false;
            return true;
        }
    }
    if (menuUpPressed()) {
        ctx.selectedTrack = (ctx.selectedTrack + count - 1) % count;
    }
    if (menuDownPressed()) {
        ctx.selectedTrack = (ctx.selectedTrack + 1) % count;
    }
    if (menuConfirmPressed()) {
        if (ctx.appState == AppState::OPEN_WORLD) {
            startRace(ctx, ctx.selectedTrack, true);
        } else {
            startRace(ctx, ctx.selectedTrack, false);
        }
        ctx.openWorldQuickRace = false;
        return true;
    }

    if (ctx.appState == AppState::OPEN_WORLD) {
        BeginDrawing();
        DrawRectangle(0, 0, ctx.screenWidth, ctx.screenHeight,
            Fade(BLACK, 0.55f));
        Hud hud;
        hud.drawQuickRaceOverlay(ctx.presets, ctx.selectedTrack,
            ctx.screenWidth, ctx.screenHeight);
        engine::input::InputBindings::instance().drawDebugRemapOverlay(
            ctx.screenWidth, ctx.screenHeight);
        EndDrawing();
    }
    return true;
}

bool GameLoop::handleEncyclopediaFrame(Context &ctx)
{
    HudEncyclopediaLayout layout = HudEncyclopedia::computeLayout(
        ctx.screenWidth, ctx.screenHeight);
    Vector2 mouse = GetMousePosition();

    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE)) {
        ctx.menuScreen = MenuScreen::MAIN;
        return true;
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        int picked = HudEncyclopedia::pickTile(layout, mouse);
        if (picked >= 0) {
            ctx.encyclopediaSelected = picked;
        } else if (HudEncyclopedia::hitBack(layout, mouse)) {
            ctx.menuScreen = MenuScreen::MAIN;
            return true;
        }
    }

    // Keyboard grid navigation (4 region columns x 5 rows per region).
    constexpr int kCols = 4;
    constexpr int kRows = 5;
    int sel = ctx.encyclopediaSelected < 0 ? 0 : ctx.encyclopediaSelected;
    int col = sel / kRows;
    int row = sel % kRows;
    bool moved = false;

    if (IsKeyPressed(KEY_LEFT)) {
        col = (col + kCols - 1) % kCols;
        moved = true;
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        col = (col + 1) % kCols;
        moved = true;
    }
    if (IsKeyPressed(KEY_UP)) {
        row = (row + kRows - 1) % kRows;
        moved = true;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        row = (row + 1) % kRows;
        moved = true;
    }
    if (moved) {
        ctx.encyclopediaSelected = col * kRows + row;
    }

    const world::ProgressionState *progression = nullptr;
    if (ctx.aurelia) {
        progression = &ctx.aurelia->progression();
    } else if (ctx.openWorldProgressionBackup) {
        progression = &*ctx.openWorldProgressionBackup;
    }

    BeginDrawing();
    ClearBackground(Color{20, 24, 36, 255});
    HudEncyclopedia::draw(ctx.screenWidth, ctx.screenHeight, layout,
        progression, ctx.encyclopediaSelected);
    EndDrawing();
    return ctx.appState == AppState::MENU;
}

} // namespace app
} // namespace racer
