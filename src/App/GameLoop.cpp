/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Menu and race frame loop
*/

#include <algorithm>
#include <memory>

#include "raylib.h"

#include "App/CameraController.hpp"
#include "App/GameLoop.hpp"

namespace racer {
namespace app {

GameLoop::Context::Context(const std::vector<TrackDef> &trackPresets)
    : presets(trackPresets)
{
}

void GameLoop::startRace(Context &ctx, int trackIndex)
{
    const TrackDef &def = ctx.presets[static_cast<size_t>(trackIndex)];

    ctx.race = std::make_unique<RaceState>(Track::make(def), 3, 3);
    ctx.currentAmbiance = ambianceForTrack(trackIndex, def);
    if (ctx.pipeline)
        ctx.pipeline->setAmbiance(ctx.currentAmbiance);
    ctx.trackRenderer = std::make_unique<TrackRenderer>(
        ctx.race->getTrack(), def);
    if (ctx.pipeline)
        ctx.trackRenderer->applyShader(ctx.pipeline->litShader());
    ctx.steerSmoothed = 0.0f;
    ctx.wheelSpin = 0.0f;
    ctx.lapTimer = {};
    ctx.confettiEmitted = false;
    ctx.vfx.clear();
    ctx.vfx.setRain(ctx.currentAmbiance == engine::Ambiance::ORAGE);
}

void GameLoop::pollShaders(Context &ctx)
{
    ctx.pipeline->pollShaderReload();
    if (ctx.trackRenderer)
        ctx.trackRenderer->applyShader(ctx.pipeline->litShader());
}

bool GameLoop::handleMenuFrame(Context &ctx)
{
    int count = static_cast<int>(ctx.presets.size());

    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        ctx.selectedTrack = (ctx.selectedTrack + count - 1) % count;
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
        ctx.selectedTrack = (ctx.selectedTrack + 1) % count;
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        startRace(ctx, ctx.selectedTrack);
        ctx.appState = AppState::RACING;
    }
    BeginDrawing();
    ClearBackground(Color{20, 24, 36, 255});
    Hud hud;
    hud.drawMenu(ctx.presets, ctx.selectedTrack, ctx.screenWidth,
        ctx.screenHeight);
    EndDrawing();
    return true;
}

bool GameLoop::shouldReturnToMenu(Context &ctx)
{
    if (ctx.race->phase() != RacePhase::FINISHED)
        return false;
    if (!IsKeyPressed(KEY_M))
        return false;
    ctx.appState = AppState::MENU;
    return true;
}

void GameLoop::handleRaceRestart(Context &ctx)
{
    if (IsKeyPressed(KEY_R))
        startRace(ctx, ctx.selectedTrack);
}

namespace {

float readSteerTarget()
{
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
        return 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
        return -1.0f;
    return 0.0f;
}

} // namespace

void GameLoop::readPlayerInput(Context &ctx, float dt)
{
    CarInput input;
    float steerTarget = readSteerTarget();

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
        input.throttle = 1.0f;
    else if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
        input.throttle = -1.0f;
    ctx.steerSmoothed += (steerTarget - ctx.steerSmoothed)
        * std::min(1.0f, 8.0f * dt);
    input.steer = ctx.steerSmoothed;
    input.handbrake = IsKeyDown(KEY_SPACE);
    input.nitro = IsKeyDown(KEY_LEFT_SHIFT);
    ctx.race->update(dt, input);
}

void GameLoop::updateWheelSpin(Context &ctx, float dt)
{
    const RacerEntry &player =
        ctx.race->racers()[static_cast<size_t>(ctx.race->playerIndex())];

    ctx.wheelSpin += player.car.speed() * dt / kWheelRadius;
}

void GameLoop::updateConfetti(Context &ctx, const Car &playerCar)
{
    if (ctx.race->phase() != RacePhase::FINISHED)
        return;
    if (ctx.confettiEmitted)
        return;
    Vector3 pos{playerCar.position().x, 3.0f, playerCar.position().z};

    ctx.vfx.emitConfetti(pos);
    ctx.confettiEmitted = true;
}

void GameLoop::renderWorld(Context &ctx)
{
    if (ctx.trackRenderer)
        ctx.trackRenderer->flushSkidMarks();
    setupHeadlights(ctx);
    ctx.pipeline->frame(
        ctx.camera, OpaquePass(ctx), LitPass(ctx), VfxPass(ctx),
        buildPostParams(ctx));
}

void GameLoop::buildHudExtras(Context &ctx, HudExtras &extras)
{
    extras.currentLapTime = ctx.lapTimer.currentLapTime;
    extras.lastLapTime = ctx.lapTimer.lastLapFlash > 0.0f
        ? ctx.lapTimer.lastLapTime : 0.0f;
    extras.bestLapTime = ctx.lapTimer.bestLapTime;
    for (size_t i = 0; i < ctx.race->racers().size(); ++i) {
        extras.racerColors.push_back(
            colorForRacerIndex(i, ctx.race->racers()[i].isPlayer));
    }
}

void GameLoop::drawFinishedHint(Context &ctx)
{
    const char *menuHint = "M : retour au menu";
    int hw = MeasureText(menuHint, 20);

    DrawText(menuHint, ctx.screenWidth / 2 - hw / 2,
        ctx.screenHeight / 2 + 50, 20, LIGHTGRAY);
}

void GameLoop::simulateRace(Context &ctx, float dt)
{
    const RacerEntry &player =
        ctx.race->racers()[static_cast<size_t>(ctx.race->playerIndex())];

    readPlayerInput(ctx, dt);
    updateLapTimer(ctx.lapTimer, player, dt, ctx.race->phase());
    updateWheelSpin(ctx, dt);
    for (const auto &r : ctx.race->racers())
        updateRacerVfx(ctx, r);
    updateConfetti(ctx, player.car);
    ctx.vfx.update(dt, player.car.position());
    updateCamera(ctx.camera, player.car, dt);
}

void GameLoop::drawRaceFrame(Context &ctx)
{
    HudExtras extras;

    BeginDrawing();
    renderWorld(ctx);
    buildHudExtras(ctx, extras);
    Hud hud;
    hud.drawHudEx(*ctx.race, ctx.screenWidth, ctx.screenHeight, extras);
    if (ctx.race->phase() == RacePhase::FINISHED)
        drawFinishedHint(ctx);
    DrawFPS(ctx.screenWidth - 90, ctx.screenHeight - 30);
    EndDrawing();
}

void GameLoop::processRaceFrame(Context &ctx, float dt)
{
    simulateRace(ctx, dt);
    drawRaceFrame(ctx);
}

void GameLoop::run(Context &ctx)
{
    while (!WindowShouldClose()) {
        float dt = std::min(GetFrameTime(), 0.1f);

        pollShaders(ctx);
        if (ctx.appState == AppState::MENU) {
            if (handleMenuFrame(ctx))
                continue;
        }
        if (shouldReturnToMenu(ctx))
            continue;
        handleRaceRestart(ctx);
        processRaceFrame(ctx, dt);
    }
}

} // namespace app
} // namespace racer
