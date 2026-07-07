/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Entry point, menu and race game loop
*/

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

#include "raylib.h"

#include "Engine/Render/RenderPipeline.hpp"
#include "Race/RaceState.hpp"
#include "Render/CarRenderer.hpp"
#include "Render/Hud.hpp"
#include "Render/TrackRenderer.hpp"
#include "Render/VfxSystem.hpp"
#include "Track/Track.hpp"
#include "Vehicle/Car.hpp"

namespace {

enum class AppState { MENU, RACING };

using racer::engine::Ambiance;
using racer::engine::RenderPipeline;

struct LapTimerState {
    float currentLapTime = 0.0f;
    float lastLapTime = 0.0f;
    float bestLapTime = 0.0f;
    int lastLapCount = 0;
    float lastLapFlash = 0.0f;
};

class MainApp {
public:
    static int run();

private:
    struct Context {
        const int screenWidth = 1280;
        const int screenHeight = 720;
        const std::vector<racer::TrackDef> &presets;
        int selectedTrack = 0;
        AppState appState = AppState::MENU;
        std::unique_ptr<racer::RaceState> race;
        std::unique_ptr<racer::TrackRenderer> trackRenderer;
        std::unique_ptr<RenderPipeline> pipeline;
        racer::VfxSystem vfx;
        float steerSmoothed = 0.0f;
        float wheelSpin = 0.0f;
        LapTimerState lapTimer;
        Ambiance currentAmbiance = Ambiance::MIDI;
        bool confettiEmitted = false;
        Camera3D camera{};

        explicit Context(const std::vector<racer::TrackDef> &trackPresets)
            : presets(trackPresets)
        {
        }
    };

    class OpaquePass {
    public:
        explicit OpaquePass(Context &ctx) : ctx_(ctx) {}

        void operator()() const;

    private:
        Context &ctx_;
    };

    class LitPass {
    public:
        explicit LitPass(Context &ctx) : ctx_(ctx) {}

        void operator()() const;

    private:
        Context &ctx_;
    };

    class VfxPass {
    public:
        explicit VfxPass(Context &ctx) : ctx_(ctx) {}

        void operator()() const;

    private:
        Context &ctx_;
    };

    static Color colorForRacerIndex(size_t index, bool isPlayer);
    static Ambiance ambianceForTrack(
        int trackIndex, const racer::TrackDef &def);
    static void updateLapTimer(
        LapTimerState &timer, const racer::RacerEntry &player, float dt,
        racer::RacePhase phase);
    static void initCamera(Context &ctx);
    static void startRace(Context &ctx, int trackIndex);
    static void pollShaders(Context &ctx);
    static bool handleMenuFrame(Context &ctx);
    static bool shouldReturnToMenu(Context &ctx);
    static void handleRaceRestart(Context &ctx);
    static void readPlayerInput(Context &ctx, float dt);
    static void updateWheelSpin(Context &ctx, float dt);
    static void updateDriftVfx(Context &ctx, const racer::RacerEntry &entry,
        Vector3 vel, float speed);
    static void updateNitroVfx(Context &ctx, const racer::RacerEntry &entry,
        Vector3 backDir, Vector3 vel);
    static void updateOffroadVfx(Context &ctx, const racer::RacerEntry &entry,
        Vector3 vel);
    static void updateRacerVfx(Context &ctx, const racer::RacerEntry &entry);
    static racer::CarVisual buildCarVisual(Context &ctx,
        const racer::RacerEntry &entry, bool headlights);
    static void updateConfetti(Context &ctx, const racer::Car &playerCar);
    static void updateCamera(
        Context &ctx, const racer::Car &playerCar, float dt);
    static void setupHeadlights(Context &ctx);
    static RenderPipeline::PostParams buildPostParams(Context &ctx);
    static void renderWorld(Context &ctx);
    static void buildHudExtras(Context &ctx, racer::HudExtras &extras);
    static void drawFinishedHint(Context &ctx);
    static void simulateRace(Context &ctx, float dt);
    static void drawRaceFrame(Context &ctx);
    static void processRaceFrame(Context &ctx, float dt);
};

Color MainApp::colorForRacerIndex(size_t index, bool isPlayer)
{
    if (isPlayer)
        return RED;
    constexpr Color palette[] = {BLUE, DARKGREEN, ORANGE, PURPLE};

    return palette[index % (sizeof(palette) / sizeof(palette[0]))];
}

Ambiance MainApp::ambianceForTrack(int trackIndex, const racer::TrackDef &def)
{
    if (def.surfaceStyle == racer::SurfaceStyle::ABIMEE)
        return Ambiance::ORAGE;
    switch (trackIndex % 3) {
    case 0:
        return Ambiance::MIDI;
    case 1:
        return Ambiance::AUBE_DOREE;
    default:
        return Ambiance::CREPUSCULE;
    }
}

void MainApp::updateLapTimer(
    LapTimerState &timer, const racer::RacerEntry &player, float dt,
    racer::RacePhase phase)
{
    if (phase != racer::RacePhase::RACING)
        return;
    timer.currentLapTime += dt;
    if (player.lap > timer.lastLapCount) {
        timer.lastLapTime = timer.currentLapTime;
        timer.currentLapTime = 0.0f;
        if (timer.bestLapTime <= 0.0f || timer.lastLapTime < timer.bestLapTime)
            timer.bestLapTime = timer.lastLapTime;
        timer.lastLapFlash = 3.0f;
        timer.lastLapCount = player.lap;
    }
    timer.lastLapFlash = std::max(0.0f, timer.lastLapFlash - dt);
}

void MainApp::initCamera(Context &ctx)
{
    ctx.camera.up = {0.0f, 1.0f, 0.0f};
    ctx.camera.fovy = 65.0f;
    ctx.camera.projection = CAMERA_PERSPECTIVE;
    ctx.camera.position = {0.0f, 8.0f, -12.0f};
    ctx.camera.target = {0.0f, 0.0f, 0.0f};
}

void MainApp::startRace(Context &ctx, int trackIndex)
{
    const racer::TrackDef &def = ctx.presets[static_cast<size_t>(trackIndex)];

    ctx.race = std::make_unique<racer::RaceState>(
        racer::Track::make(def), 3, 3);
    ctx.currentAmbiance = ambianceForTrack(trackIndex, def);
    if (ctx.pipeline)
        ctx.pipeline->setAmbiance(ctx.currentAmbiance);
    ctx.trackRenderer = std::make_unique<racer::TrackRenderer>(
        ctx.race->getTrack(), def);
    if (ctx.pipeline)
        ctx.trackRenderer->applyShader(ctx.pipeline->litShader());
    ctx.steerSmoothed = 0.0f;
    ctx.wheelSpin = 0.0f;
    ctx.lapTimer = {};
    ctx.confettiEmitted = false;
    ctx.vfx.clear();
    ctx.vfx.setRain(ctx.currentAmbiance == Ambiance::ORAGE);
}

void MainApp::pollShaders(Context &ctx)
{
    ctx.pipeline->pollShaderReload();
    if (ctx.trackRenderer)
        ctx.trackRenderer->applyShader(ctx.pipeline->litShader());
}

bool MainApp::handleMenuFrame(Context &ctx)
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
    racer::drawMenu(ctx.presets, ctx.selectedTrack, ctx.screenWidth,
        ctx.screenHeight);
    EndDrawing();
    return true;
}

bool MainApp::shouldReturnToMenu(Context &ctx)
{
    if (ctx.race->phase() != racer::RacePhase::FINISHED)
        return false;
    if (!IsKeyPressed(KEY_M))
        return false;
    ctx.appState = AppState::MENU;
    return true;
}

void MainApp::handleRaceRestart(Context &ctx)
{
    if (IsKeyPressed(KEY_R))
        startRace(ctx, ctx.selectedTrack);
}

void MainApp::readPlayerInput(Context &ctx, float dt)
{
    racer::CarInput input;
    float steerTarget = 0.0f;

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
        input.throttle = 1.0f;
    else if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
        input.throttle = -1.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
        steerTarget = 1.0f;
    else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
        steerTarget = -1.0f;
    ctx.steerSmoothed += (steerTarget - ctx.steerSmoothed)
        * std::min(1.0f, 8.0f * dt);
    input.steer = ctx.steerSmoothed;
    input.handbrake = IsKeyDown(KEY_SPACE);
    input.nitro = IsKeyDown(KEY_LEFT_SHIFT);
    ctx.race->update(dt, input);
}

void MainApp::updateWheelSpin(Context &ctx, float dt)
{
    const racer::RacerEntry &player =
        ctx.race->racers()[static_cast<size_t>(ctx.race->playerIndex())];

    ctx.wheelSpin += player.car.speed * dt / racer::kWheelRadius;
}

void MainApp::updateDriftVfx(Context &ctx, const racer::RacerEntry &entry,
    Vector3 vel, float speed)
{
    Vector3 smokePos{entry.car.position.x, 0.12f, entry.car.position.z};

    ctx.vfx.emitDriftSmoke(smokePos, vel);
    if (speed <= 0.1f || !ctx.trackRenderer)
        return;
    Vector3 markPos{
        entry.car.position.x - vel.x / speed * 1.0f, 0.05f,
        entry.car.position.z - vel.z / speed * 1.0f};

    ctx.trackRenderer->queueSkidMark(markPos, vel, 0.32f, 0.7f);
}

void MainApp::updateNitroVfx(Context &ctx, const racer::RacerEntry &entry,
    Vector3 backDir, Vector3 vel)
{
    auto lights = racer::getCarLightPoints(entry.car);

    ctx.vfx.emitNitroFlame(lights.exhaust, backDir, vel);
}

void MainApp::updateOffroadVfx(Context &ctx, const racer::RacerEntry &entry,
    Vector3 vel)
{
    Vector3 dustPos{entry.car.position.x, 0.1f, entry.car.position.z};

    ctx.vfx.emitOffroadDust(dustPos, vel);
}

void MainApp::updateRacerVfx(Context &ctx, const racer::RacerEntry &entry)
{
    Vector3 vel = entry.car.velocity();
    float speed = std::sqrt(vel.x * vel.x + vel.z * vel.z);
    Vector3 backDir = entry.car.forward();

    if (speed > 0.1f)
        backDir = Vector3{-vel.x / speed, 0.0f, -vel.z / speed};
    if (entry.car.isDrifting && std::fabs(entry.car.speed) > 6.0f)
        updateDriftVfx(ctx, entry, vel, speed);
    if (entry.lastInput.nitro && entry.car.nitroRemaining > 0.0f)
        updateNitroVfx(ctx, entry, backDir, vel);
    if (std::fabs(entry.car.surfaceDrag) > 1.5f
        && std::fabs(entry.car.speed) > 5.0f)
        updateOffroadVfx(ctx, entry, vel);
}

racer::CarVisual MainApp::buildCarVisual(Context &ctx,
    const racer::RacerEntry &entry, bool headlights)
{
    racer::CarVisual vis;

    vis.steer = entry.isPlayer ? ctx.steerSmoothed : 0.0f;
    vis.wheelSpin = entry.isPlayer ? ctx.wheelSpin : 0.0f;
    vis.braking = entry.lastInput.throttle < -0.01f && entry.car.speed > 1.0f;
    vis.nitro = entry.lastInput.nitro && entry.car.nitroRemaining > 0.0f;
    vis.headlights = headlights;
    vis.drifting = entry.car.isDrifting;
    return vis;
}

void MainApp::updateConfetti(Context &ctx, const racer::Car &playerCar)
{
    if (ctx.race->phase() != racer::RacePhase::FINISHED)
        return;
    if (ctx.confettiEmitted)
        return;
    Vector3 pos{playerCar.position.x, 3.0f, playerCar.position.z};

    ctx.vfx.emitConfetti(pos);
    ctx.confettiEmitted = true;
}

void MainApp::updateCamera(
    Context &ctx, const racer::Car &playerCar, float dt)
{
    Vector3 forward = playerCar.forward();
    Vector3 desiredCamPos{
        playerCar.position.x - forward.x * 9.0f,
        playerCar.position.y + 4.5f,
        playerCar.position.z - forward.z * 9.0f,
    };
    Vector3 desiredTarget{
        playerCar.position.x + forward.x * 4.0f,
        playerCar.position.y + 1.0f,
        playerCar.position.z + forward.z * 4.0f,
    };
    float camLerp = std::min(1.0f, 6.0f * dt);

    ctx.camera.position.x += (desiredCamPos.x - ctx.camera.position.x)
        * camLerp;
    ctx.camera.position.y += (desiredCamPos.y - ctx.camera.position.y)
        * camLerp;
    ctx.camera.position.z += (desiredCamPos.z - ctx.camera.position.z)
        * camLerp;
    ctx.camera.target.x += (desiredTarget.x - ctx.camera.target.x) * camLerp;
    ctx.camera.target.y += (desiredTarget.y - ctx.camera.target.y) * camLerp;
    ctx.camera.target.z += (desiredTarget.z - ctx.camera.target.z) * camLerp;
}

void MainApp::setupHeadlights(Context &ctx)
{
    const auto &racers = ctx.race->racers();
    const auto &pipeParams = ctx.pipeline->params();

    ctx.pipeline->clearLights();
    if (!pipeParams.headlights)
        return;
    for (size_t i = 0; i < racers.size(); ++i) {
        auto lp = racer::getCarLightPoints(racers[i].car);

        ctx.pipeline->addLight(lp.headL, Vector3{2.5f, 2.4f, 2.0f});
        ctx.pipeline->addLight(lp.headR, Vector3{2.5f, 2.4f, 2.0f});
    }
}

RenderPipeline::PostParams MainApp::buildPostParams(Context &ctx)
{
    const racer::RacerEntry &player =
        ctx.race->racers()[static_cast<size_t>(ctx.race->playerIndex())];
    float speedRatio = std::clamp(
        std::fabs(player.car.speed) / player.car.tuning.maxSpeed, 0.0f, 1.0f);
    bool nitroActive = player.lastInput.nitro
        && player.car.nitroRemaining > 0.0f;

    return RenderPipeline::PostParams{speedRatio, nitroActive};
}

void MainApp::OpaquePass::operator()() const
{
    if (!ctx_.trackRenderer)
        return;
    ctx_.trackRenderer->drawOpaqueGeometry();
    const auto &racers = ctx_.race->racers();

    for (size_t i = 0; i < racers.size(); ++i) {
        racer::drawCarEx(
            racers[i].car, {},
            colorForRacerIndex(i, racers[i].isPlayer));
    }
}

void MainApp::LitPass::operator()() const
{
    if (!ctx_.trackRenderer)
        return;
    ctx_.trackRenderer->draw(static_cast<float>(GetTime()));
    const auto &racers = ctx_.race->racers();
    const auto &pipeParams = ctx_.pipeline->params();

    for (size_t i = 0; i < racers.size(); ++i) {
        const auto &r = racers[i];
        racer::CarVisual vis = buildCarVisual(
            ctx_, r, pipeParams.headlights);

        racer::drawCarEx(
            r.car, vis, colorForRacerIndex(i, racers[i].isPlayer));
    }
}

void MainApp::VfxPass::operator()() const
{
    ctx_.vfx.draw(ctx_.camera);
}

void MainApp::renderWorld(Context &ctx)
{
    if (ctx.trackRenderer)
        ctx.trackRenderer->flushSkidMarks();
    setupHeadlights(ctx);
    ctx.pipeline->frame(
        ctx.camera, OpaquePass(ctx), LitPass(ctx), VfxPass(ctx),
        buildPostParams(ctx));
}

void MainApp::buildHudExtras(Context &ctx, racer::HudExtras &extras)
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

void MainApp::drawFinishedHint(Context &ctx)
{
    const char *menuHint = "M : retour au menu";
    int hw = MeasureText(menuHint, 20);

    DrawText(menuHint, ctx.screenWidth / 2 - hw / 2,
        ctx.screenHeight / 2 + 50, 20, LIGHTGRAY);
}

void MainApp::simulateRace(Context &ctx, float dt)
{
    const racer::RacerEntry &player =
        ctx.race->racers()[static_cast<size_t>(ctx.race->playerIndex())];

    readPlayerInput(ctx, dt);
    updateLapTimer(ctx.lapTimer, player, dt, ctx.race->phase());
    updateWheelSpin(ctx, dt);
    for (const auto &r : ctx.race->racers())
        updateRacerVfx(ctx, r);
    updateConfetti(ctx, player.car);
    ctx.vfx.update(dt, player.car.position);
    updateCamera(ctx, player.car, dt);
}

void MainApp::drawRaceFrame(Context &ctx)
{
    racer::HudExtras extras;

    BeginDrawing();
    renderWorld(ctx);
    buildHudExtras(ctx, extras);
    racer::drawHudEx(*ctx.race, ctx.screenWidth, ctx.screenHeight, extras);
    if (ctx.race->phase() == racer::RacePhase::FINISHED)
        drawFinishedHint(ctx);
    DrawFPS(ctx.screenWidth - 90, ctx.screenHeight - 30);
    EndDrawing();
}

void MainApp::processRaceFrame(Context &ctx, float dt)
{
    simulateRace(ctx, dt);
    drawRaceFrame(ctx);
}

int MainApp::run()
{
    const std::vector<racer::TrackDef> &presets = racer::Track::presets();
    Context ctx(presets);

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(ctx.screenWidth, ctx.screenHeight, "racer");
    SetTargetFPS(60);
    initCamera(ctx);
    ctx.pipeline = std::make_unique<RenderPipeline>(
        ctx.screenWidth, ctx.screenHeight);
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
    ctx.pipeline.reset();
    CloseWindow();
    return 0;
}

} // namespace

int main()
{
    return MainApp::run();
}
