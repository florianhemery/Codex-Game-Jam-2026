/*

** EPITECH PROJECT, 2026

** racer

** File description:

** Menu and race frame loop

*/



#include <algorithm>

#include <cmath>

#include <memory>



#include "raylib.h"



#include "App/CameraController.hpp"

#include "App/GameLoop.hpp"

#include "App/GameLoopInput.hpp"
#include "App/RacerColors.hpp"

#include "World/Aurelia/AureliaWorld.hpp"
#include "World/Aurelia/AureliaTypes.hpp"

#include "Render/Car/CarWheelDraw.hpp"

#include "Render/CarRenderer.hpp"

#include "Render/Hud.hpp"

#include "Render/Hud/HudFinishScreen.hpp"

#include "Render/Hud/HudMenu.hpp"

#include "Render/Hud/HudRaceOverlay.hpp"



namespace racer {

namespace app {



void GameLoop::OpaquePass::operator()() const
{
    if (ctx_.aurelia) {
        ctx_.aurelia->drawOpaque();
        CarVisual vis{};
        vis.steer = ctx_.steerSmoothed;
        vis.wheelSpin = ctx_.aurelia->wheelSpin();
        CarRenderer::drawCarEx(
            ctx_.aurelia->playerCar(), vis, colorForRacerIndex(0, true),
            ctx_.pipeline->litShader());
        return;
    }
    if (!ctx_.trackRenderer) {
        return;
    }
    ctx_.trackRenderer->drawOpaqueGeometry();
}

void GameLoop::LitPass::operator()() const
{
    if (ctx_.aurelia) {
        ctx_.aurelia->drawOpaque();
        ctx_.aurelia->drawLit(static_cast<float>(GetTime()));
        ctx_.aurelia->drawTriggers();
        ctx_.aurelia->drawTraffic();
        return;
    }
    if (!ctx_.trackRenderer) {
        return;
    }
    ctx_.trackRenderer->draw(static_cast<float>(GetTime()));
}



void GameLoop::CarsPass::operator()() const
{
    if (!ctx_.aurelia && !ctx_.trackRenderer) {
        return;
    }
    const auto &pipeParams = ctx_.pipeline->params();

    if (ctx_.race) {
        const auto &racers = ctx_.race->racers();
        for (size_t i = 0; i < racers.size(); ++i) {
            const auto &r = racers[i];
            CarVisual vis = GameLoop::buildCarVisual(
                ctx_, r, pipeParams.headlights);
            CarRenderer::drawCarEx(
                r.car, vis, colorForRacerIndex(i, racers[i].isPlayer));
        }
        return;
    }
    if (!ctx_.aurelia) {
        return;
    }
    CarVisual vis{};
    vis.steer = ctx_.steerSmoothed;
    vis.wheelSpin = ctx_.aurelia->wheelSpin();
    vis.headlights = pipeParams.headlights;
    CarRenderer::drawCarEx(
        ctx_.aurelia->playerCar(), vis, colorForRacerIndex(0, true));
}



void GameLoop::UnlitPass::operator()() const

{

    CarsPass{ctx_}();

    VfxPass{ctx_}();

}



void GameLoop::VfxPass::operator()() const

{

    ctx_.vfx->draw(ctx_.camera);

}



GameLoop::Context::Context(const std::vector<TrackDef> &trackPresets)

    : presets(trackPresets)

{

}



void GameLoop::startRace(Context &ctx, int trackIndex, bool fromOpenWorld)
{
    const TrackDef &def = ctx.presets[static_cast<size_t>(trackIndex)];

    ctx.raceFromOpenWorld = fromOpenWorld;
    if (fromOpenWorld && ctx.aurelia) {
        ctx.openWorldProgressionBackup = ctx.aurelia->progression();
        ctx.openWorldRespawnRegion =
            racer::world::regionForBiome(ctx.aurelia->currentBiome());
        ctx.aurelia.reset();
    }
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

    ctx.racePaused = false;

    ctx.controlsHintTimer = 30.0f;

    ctx.driftCount = 0;

    ctx.wasDrifting = false;

    ctx.vfx->clear();

    ctx.vfx->setRain(ctx.currentAmbiance == engine::Ambiance::ORAGE);
    ctx.appState = AppState::RACING;
}



void GameLoop::pollShaders(Context &ctx)

{

    ctx.pipeline->pollShaderReload();

    if (ctx.trackRenderer)

        ctx.trackRenderer->applyShader(ctx.pipeline->litShader());

    if (ctx.aurelia)

        ctx.aurelia->applyShader(ctx.pipeline->litShader());

}



void GameLoop::updateDisplay(Context &ctx)

{

    const bool toggle = IsKeyPressed(KEY_F11)

        || (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_ENTER));



    if (toggle) {

        if (IsWindowFullscreen()) {

            ToggleFullscreen();

            SetWindowSize(ctx.windowedWidth, ctx.windowedHeight);

        } else {

            ctx.windowedWidth = GetScreenWidth();

            ctx.windowedHeight = GetScreenHeight();

            const int monitor = GetCurrentMonitor();



            SetWindowSize(GetMonitorWidth(monitor),

                GetMonitorHeight(monitor));

            ToggleFullscreen();

        }

    }

    if (!toggle && !IsWindowResized())

        return;

    const int newW = GetScreenWidth();

    const int newH = GetScreenHeight();



    if (newW < 1 || newH < 1)

        return;

    ctx.screenWidth = newW;

    ctx.screenHeight = newH;

    if (ctx.pipeline)

        ctx.pipeline->resize(newW, newH);

}



namespace {

AudioDriveSnapshot buildAudioSnapshot(const RacerEntry &player)
{
    AudioDriveSnapshot snap{};

    snap.input = player.lastInput;
    snap.speed = player.car.speed();
    snap.heading = player.car.heading();
    snap.velocityHeading = player.car.velocityHeading();
    snap.maxSpeed = player.car.tuning().maxSpeed;
    snap.surfaceDrag = player.car.surfaceDrag();
    snap.nitroActive = player.lastInput.nitro
        && player.car.nitroRemaining() > 0.0f;
    snap.drifting = player.car.isDrifting()
        && std::fabs(snap.speed) > 6.0f;
    return snap;
}

} // namespace



bool GameLoop::handleMenuFrame(Context &ctx)

{

    int count = static_cast<int>(ctx.presets.size());

    HudMenuLayout layout = HudMenu::computeLayout(

        ctx.presets, ctx.screenWidth, ctx.screenHeight);

    Vector2 mouse = GetMousePosition();



    if (IsKeyPressed(KEY_ESCAPE)) {
        ctx.menuScreen = MenuScreen::MAIN;
        ctx.showHowToPlay = false;
    }
    if (IsKeyPressed(KEY_H) || charJustPressed('h', 'H')) {

        ctx.showHowToPlay = !ctx.showHowToPlay;

    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {

        if (ctx.showHowToPlay) {

            const float panelW = 620.0f;

            const float panelH = 360.0f;

            Rectangle panel{

                (static_cast<float>(ctx.screenWidth) - panelW) * 0.5f,

                (static_cast<float>(ctx.screenHeight) - panelH) * 0.5f,

                panelW, panelH

            };



            if (!CheckCollisionPointRec(mouse, panel)) {

                ctx.showHowToPlay = false;

            }

        } else {

            int picked = HudMenu::pickCard(layout, mouse);



            if (picked >= 0) {

                ctx.selectedTrack = picked;

            } else if (HudMenu::hitStartButton(layout, mouse)) {
                startRace(ctx, ctx.selectedTrack, false);
            } else if (HudMenu::hitHelpButton(layout, mouse)) {

                ctx.showHowToPlay = true;

            }

        }

    }

    if (!ctx.showHowToPlay) {

        if (menuUpPressed()) {

            ctx.selectedTrack = (ctx.selectedTrack + count - 1) % count;

        }

        if (menuDownPressed()) {

            ctx.selectedTrack = (ctx.selectedTrack + 1) % count;

        }

        if (menuConfirmPressed()) {
            startRace(ctx, ctx.selectedTrack, false);
        }

    }

    BeginDrawing();

    ClearBackground(Color{20, 24, 36, 255});

    Hud hud;

    hud.drawMenu(ctx.presets, ctx.selectedTrack, ctx.screenWidth,

        ctx.screenHeight, ctx.showHowToPlay);

    EndDrawing();

    return true;

}



bool GameLoop::shouldReturnToMenu(Context &ctx)

{

    if (ctx.race->phase() != RacePhase::FINISHED) {

        return false;

    }

    HudFinishLayout layout = HudFinishScreen::computeLayout(

        *ctx.race, ctx.screenWidth, ctx.screenHeight);



    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {

        Vector2 mouse = GetMousePosition();



        if (HudFinishScreen::hitMenu(layout, mouse)) {
            leaveRace(ctx);
            return true;
        }
    }
    if (!menuReturnPressed()) {
        return false;
    }
    leaveRace(ctx);
    return true;
}



void GameLoop::handleRaceRestart(Context &ctx)

{

    if (ctx.race->phase() != RacePhase::FINISHED) {

        return;

    }

    HudFinishLayout layout = HudFinishScreen::computeLayout(

        *ctx.race, ctx.screenWidth, ctx.screenHeight);



    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {

        Vector2 mouse = GetMousePosition();



        if (HudFinishScreen::hitRestart(layout, mouse)) {
            startRace(ctx, ctx.selectedTrack, ctx.raceFromOpenWorld);
            return;
        }
    }
    if (IsKeyPressed(KEY_R) || menuConfirmPressed()) {
        startRace(ctx, ctx.selectedTrack, ctx.raceFromOpenWorld);
    }
}



void GameLoop::handlePauseInput(Context &ctx)
{
    const RacePhase phase = ctx.race->phase();
    const bool pausable = phase == RacePhase::COUNTDOWN
        || phase == RacePhase::RACING
        || phase == RacePhase::WRAP_UP;

    if (!pausable) {
        ctx.racePaused = false;
        return;
    }
    if (!ctx.racePaused && IsKeyPressed(KEY_ESCAPE)) {
        ctx.racePaused = true;
        return;
    }
    if (!ctx.racePaused) {
        return;
    }
    if (IsKeyPressed(KEY_ESCAPE) || menuConfirmPressed()) {
        ctx.racePaused = false;
        return;
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouse = GetMousePosition();

        if (HudRaceOverlay::hitPauseResume(
                ctx.screenWidth, ctx.screenHeight, mouse)) {
            ctx.racePaused = false;
            return;
        }
        if (HudRaceOverlay::hitPauseMenu(
                ctx.screenWidth, ctx.screenHeight, mouse)) {
            ctx.racePaused = false;
            leaveRace(ctx);
        }
    }
}



void GameLoop::readPlayerInput(Context &ctx, float dt)

{

    if (ctx.racePaused) {

        return;

    }

    CarInput input;

    float steerTarget = readSteerTarget();



    if (keyHeld(KEY_W, KEY_Z) || keyHeld(KEY_UP)) {

        input.throttle = 1.0f;

    } else if (keyHeld(KEY_S) || keyHeld(KEY_DOWN)) {

        input.throttle = -1.0f;

    }

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



    ctx.vfx->emitConfetti(pos);

    ctx.confettiEmitted = true;

}



void GameLoop::renderWorld(Context &ctx)

{

    if (ctx.trackRenderer)

        ctx.trackRenderer->flushSkidMarks();

    setupHeadlights(ctx);

    ctx.pipeline->frame(

        ctx.camera, OpaquePass(ctx), LitPass(ctx), UnlitPass(ctx),

        buildPostParams(ctx));

}



void GameLoop::buildHudExtras(Context &ctx, HudExtras &extras)

{

    const RacerEntry &player =

        ctx.race->racers()[static_cast<size_t>(ctx.race->playerIndex())];



    extras.currentLapTime = ctx.lapTimer.currentLapTime;

    extras.lastLapTime = ctx.lapTimer.lastLapFlash > 0.0f

        ? ctx.lapTimer.lastLapTime : 0.0f;

    extras.bestLapTime = ctx.lapTimer.bestLapTime;

    extras.lastLapFlash = ctx.lapTimer.lastLapFlash;

    extras.lapBannerTimer = ctx.lapTimer.lapBannerTimer;

    extras.lapBannerLap = ctx.lapTimer.lapBannerLap;

    extras.lapBannerIsFinal = ctx.lapTimer.lapBannerIsFinal;

    extras.driftCount = ctx.driftCount;
    extras.startBoostActive = ctx.race->startBoostRemaining() > 0.0f;
    extras.startBoostRemaining = ctx.race->startBoostRemaining();
    extras.showControlsBanner = ctx.controlsHintTimer > 0.0f

        && player.lap < 1;

    extras.nitroActive = player.lastInput.nitro

        && player.car.nitroRemaining() > 0.0f;

    extras.drifting = player.car.isDrifting()

        && std::fabs(player.car.speed()) > 6.0f;

    extras.offroad = std::fabs(player.car.surfaceDrag()) > 1.5f

        && std::fabs(player.car.speed()) > 5.0f;

    for (size_t i = 0; i < ctx.race->racers().size(); ++i) {

        extras.racerColors.push_back(

            colorForRacerIndex(i, ctx.race->racers()[i].isPlayer));

    }

}



void GameLoop::simulateRace(Context &ctx, float dt)

{

    const RacerEntry &player =

        ctx.race->racers()[static_cast<size_t>(ctx.race->playerIndex())];

    const RacePhase phase = ctx.race->phase();

    bool drifting = player.car.isDrifting()
        && std::fabs(player.car.speed()) > 6.0f;

    if (phase == RacePhase::RACING && !ctx.racePaused) {

        ctx.controlsHintTimer = std::max(0.0f, ctx.controlsHintTimer - dt);

        if (drifting && !ctx.wasDrifting) {

            ctx.driftCount += 1;

        }

    }

    ctx.wasDrifting = drifting;



    if (ctx.racePaused) {
        ctx.audio.update(*ctx.race, buildAudioSnapshot(player), dt, true);
        return;
    }

    readPlayerInput(ctx, dt);

    updateLapTimer(ctx.lapTimer, player, dt, phase, ctx.race->lapsToWin());
    updateWheelSpin(ctx, dt);
    for (const auto &r : ctx.race->racers())
        updateRacerVfx(ctx, r);
    updateConfetti(ctx, player.car);
    ctx.vfx->update(dt, player.car.position());
    if (phase != RacePhase::FINISHED) {
        updateCamera(ctx.camera, player.car, dt);
    }

    ctx.audio.update(*ctx.race, buildAudioSnapshot(player), dt, false);
}



void GameLoop::drawRaceFrame(Context &ctx)

{

    HudExtras extras;



    BeginDrawing();

    renderWorld(ctx);

    buildHudExtras(ctx, extras);

    Hud hud;

    hud.drawHudEx(*ctx.race, ctx.screenWidth, ctx.screenHeight, extras);

    if (ctx.racePaused) {
        hud.drawPauseOverlay(ctx.screenWidth, ctx.screenHeight);
    }
    EndDrawing();
}



void GameLoop::processRaceFrame(Context &ctx, float dt)
{
    handlePauseInput(ctx);
    simulateRace(ctx, dt);
    drawRaceFrame(ctx);
}



void GameLoop::run(Context &ctx)

{

    while (!WindowShouldClose()) {

        float dt = std::min(GetFrameTime(), 0.1f);



        updateDisplay(ctx);

        pollShaders(ctx);

        if (ctx.appState == AppState::MENU) {
            if (ctx.menuScreen == MenuScreen::MAIN) {
                if (handleMainMenuFrame(ctx))
                    continue;
            } else if (handleMenuFrame(ctx)) {
                continue;
            }
        }
        if (ctx.appState == AppState::OPEN_WORLD) {
            handleOpenWorldFrame(ctx, dt);
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

