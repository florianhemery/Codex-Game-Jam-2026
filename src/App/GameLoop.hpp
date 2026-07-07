/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Menu and race frame loop
*/

#ifndef GAME_LOOP_HPP_
#define GAME_LOOP_HPP_

#include <cmath>
#include <memory>
#include <vector>

#include "raylib.h"

#include "App/LapTimer.hpp"
#include "App/RacerColors.hpp"
#include "Engine/Render/RenderPipeline.hpp"
#include "Race/RaceState.hpp"
#include "Render/CarRenderer.hpp"
#include "Render/CarVisual.hpp"
#include "Render/Hud.hpp"
#include "Render/Track/TrackRenderer.hpp"
#include "Render/VfxSystem.hpp"
#include "Track/Track.hpp"
#include "Vehicle/Car.hpp"

namespace racer {
namespace app {

enum class AppState { MENU, RACING };

class GameLoop {
public:
    struct Context {
        const int screenWidth = 1280;
        const int screenHeight = 720;
        const std::vector<TrackDef> &presets;
        int selectedTrack = 0;
        AppState appState = AppState::MENU;
        std::unique_ptr<RaceState> race;
        std::unique_ptr<TrackRenderer> trackRenderer;
        std::unique_ptr<engine::RenderPipeline> pipeline;
        VfxSystem vfx;
        float steerSmoothed = 0.0f;
        float wheelSpin = 0.0f;
        LapTimerState lapTimer;
        engine::Ambiance currentAmbiance = engine::Ambiance::MIDI;
        bool confettiEmitted = false;
        Camera3D camera{};

        explicit Context(const std::vector<TrackDef> &trackPresets);
    };

    static void run(Context &ctx);

private:
    class OpaquePass {
    public:
        explicit OpaquePass(Context &ctx) : ctx_(ctx) {}

        void operator()() const
        {
            if (!ctx_.trackRenderer)
                return;
            ctx_.trackRenderer->drawOpaqueGeometry();
            const auto &racers = ctx_.race->racers();

            for (size_t i = 0; i < racers.size(); ++i) {
                CarRenderer::drawCarEx(
                    racers[i].car, {},
                    colorForRacerIndex(i, racers[i].isPlayer));
            }
        }

    private:
        Context &ctx_;
    };

    class LitPass {
    public:
        explicit LitPass(Context &ctx) : ctx_(ctx) {}

        void operator()() const
        {
            if (!ctx_.trackRenderer)
                return;
            ctx_.trackRenderer->draw(static_cast<float>(GetTime()));
            const auto &racers = ctx_.race->racers();
            const auto &pipeParams = ctx_.pipeline->params();

            for (size_t i = 0; i < racers.size(); ++i) {
                const auto &r = racers[i];
                CarVisual vis = buildCarVisual(
                    ctx_, r, pipeParams.headlights);

                CarRenderer::drawCarEx(
                    r.car, vis, colorForRacerIndex(i, racers[i].isPlayer));
            }
        }

    private:
        Context &ctx_;
    };

    class VfxPass {
    public:
        explicit VfxPass(Context &ctx) : ctx_(ctx) {}

        void operator()() const
        {
            ctx_.vfx.draw(ctx_.camera);
        }

    private:
        Context &ctx_;
    };

    static void startRace(Context &ctx, int trackIndex);
    static void pollShaders(Context &ctx);
    static bool handleMenuFrame(Context &ctx);
    static bool shouldReturnToMenu(Context &ctx);
    static void handleRaceRestart(Context &ctx);
    static void readPlayerInput(Context &ctx, float dt);
    static void updateWheelSpin(Context &ctx, float dt);
    static void updateDriftVfx(Context &ctx, const RacerEntry &entry,
        Vector3 vel, float speed)
    {
        Vector3 smokePos{
            entry.car.position().x, 0.12f, entry.car.position().z};

        ctx.vfx.emitDriftSmoke(smokePos, vel);
        if (speed <= 0.1f || !ctx.trackRenderer)
            return;
        Vector3 markPos{
            entry.car.position().x - vel.x / speed * 1.0f, 0.05f,
            entry.car.position().z - vel.z / speed * 1.0f};

        ctx.trackRenderer->queueSkidMark(markPos, vel, 0.32f, 0.7f);
    }
    static void updateNitroVfx(Context &ctx, const RacerEntry &entry,
        Vector3 backDir, Vector3 vel)
    {
        auto lights = CarRenderer::getCarLightPoints(entry.car);

        ctx.vfx.emitNitroFlame(lights.exhaust, backDir, vel);
    }
    static void updateOffroadVfx(Context &ctx, const RacerEntry &entry,
        Vector3 vel)
    {
        Vector3 dustPos{entry.car.position().x, 0.1f, entry.car.position().z};

        ctx.vfx.emitOffroadDust(dustPos, vel);
    }
    static void updateRacerVfx(Context &ctx, const RacerEntry &entry)
    {
        Vector3 vel = entry.car.velocity();
        float speed = std::sqrt(vel.x * vel.x + vel.z * vel.z);
        Vector3 backDir = entry.car.forward();

        if (speed > 0.1f)
            backDir = Vector3{-vel.x / speed, 0.0f, -vel.z / speed};
        if (entry.car.isDrifting() && std::fabs(entry.car.speed()) > 6.0f)
            updateDriftVfx(ctx, entry, vel, speed);
        if (entry.lastInput.nitro && entry.car.nitroRemaining() > 0.0f)
            updateNitroVfx(ctx, entry, backDir, vel);
        if (std::fabs(entry.car.surfaceDrag()) > 1.5f
            && std::fabs(entry.car.speed()) > 5.0f)
            updateOffroadVfx(ctx, entry, vel);
    }
    static CarVisual buildCarVisual(Context &ctx, const RacerEntry &entry,
        bool headlights)
    {
        CarVisual vis;

        vis.steer = entry.isPlayer ? ctx.steerSmoothed : 0.0f;
        vis.wheelSpin = entry.isPlayer ? ctx.wheelSpin : 0.0f;
        vis.braking = entry.lastInput.throttle < -0.01f
            && entry.car.speed() > 1.0f;
        vis.nitro = entry.lastInput.nitro
            && entry.car.nitroRemaining() > 0.0f;
        vis.headlights = headlights;
        vis.drifting = entry.car.isDrifting();
        return vis;
    }
    static void updateConfetti(Context &ctx, const Car &playerCar);
    static void setupHeadlights(Context &ctx)
    {
        const auto &racers = ctx.race->racers();
        const auto &pipeParams = ctx.pipeline->params();

        ctx.pipeline->clearLights();
        if (!pipeParams.headlights)
            return;
        for (size_t i = 0; i < racers.size(); ++i) {
            auto lp = CarRenderer::getCarLightPoints(racers[i].car);

            ctx.pipeline->addLight(lp.headL, Vector3{2.5f, 2.4f, 2.0f});
            ctx.pipeline->addLight(lp.headR, Vector3{2.5f, 2.4f, 2.0f});
        }
    }
    static engine::RenderPipeline::PostParams buildPostParams(Context &ctx)
    {
        const RacerEntry &player =
            ctx.race->racers()[static_cast<size_t>(ctx.race->playerIndex())];
        float maxSpeed = player.car.tuning().maxSpeed;
        float speedRatio = std::clamp(
            std::fabs(player.car.speed()) / maxSpeed, 0.0f, 1.0f);
        bool nitroActive = player.lastInput.nitro
            && player.car.nitroRemaining() > 0.0f;

        return engine::RenderPipeline::PostParams{speedRatio, nitroActive};
    }
    static void renderWorld(Context &ctx);
    static void buildHudExtras(Context &ctx, HudExtras &extras);
    static void drawFinishedHint(Context &ctx);
    static void simulateRace(Context &ctx, float dt);
    static void drawRaceFrame(Context &ctx);
    static void processRaceFrame(Context &ctx, float dt);
};

} // namespace app
} // namespace racer

#endif
