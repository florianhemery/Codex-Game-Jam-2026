/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Game loop VFX and post-process helpers
*/

#include "App/GameLoop.hpp"

#include "Render/CarRenderer.hpp"
#include "World/Aurelia/AureliaWorld.hpp"
#include "App/RacerColors.hpp"
#include "Render/CarRenderer.hpp"

#include <algorithm>
#include <cmath>

namespace racer {
namespace app {

void GameLoop::updateDriftVfx(Context &ctx, const RacerEntry &entry,
    Vector3 vel, float speed)
{
    Vector3 smokePos{entry.car.position().x, 0.12f, entry.car.position().z};

    ctx.vfx->emitDriftSmoke(smokePos, vel);
    if (speed <= 0.1f || !ctx.trackRenderer)
        return;
    Vector3 markPos{
        entry.car.position().x - vel.x / speed * 1.0f, 0.05f,
        entry.car.position().z - vel.z / speed * 1.0f};

    ctx.trackRenderer->queueSkidMark(markPos, vel, 0.32f, 0.7f);
}

void GameLoop::updateNitroVfx(Context &ctx, const RacerEntry &entry,
    Vector3 backDir, Vector3 vel)
{
    auto lights = CarRenderer::getCarLightPoints(entry.car);

    ctx.vfx->emitNitroFlame(lights.exhaust, backDir, vel);
}

void GameLoop::updateOffroadVfx(Context &ctx, const RacerEntry &entry,
    Vector3 vel)
{
    Vector3 dustPos{entry.car.position().x, 0.1f, entry.car.position().z};

    ctx.vfx->emitOffroadDust(dustPos, vel);
}

void GameLoop::updateRacerVfx(Context &ctx, const RacerEntry &entry)
{
    Vector3 vel = entry.car.velocity();
    float speed = std::sqrt(vel.x * vel.x + vel.z * vel.z);
    Vector3 fwd = entry.car.forward();
    Vector3 backDir{-fwd.x, 0.0f, -fwd.z};

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

CarVisual GameLoop::buildCarVisual(Context &ctx, const RacerEntry &entry,
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

void GameLoop::setupHeadlights(Context &ctx)
{
    const auto &pipeParams = ctx.pipeline->params();

    ctx.pipeline->clearLights();
    if (!pipeParams.headlights) {
        return;
    }
    if (ctx.race) {
        const auto &racers = ctx.race->racers();
        for (size_t i = 0; i < racers.size(); ++i) {
            auto lp = CarRenderer::getCarLightPoints(racers[i].car);
            ctx.pipeline->addLight(lp.headL, Vector3{2.5f, 2.4f, 2.0f});
            ctx.pipeline->addLight(lp.headR, Vector3{2.5f, 2.4f, 2.0f});
        }
        return;
    }
    if (!ctx.aurelia) {
        return;
    }
    auto lp = CarRenderer::getCarLightPoints(ctx.aurelia->playerCar());
    ctx.pipeline->addLight(lp.headL, Vector3{2.5f, 2.4f, 2.0f});
    ctx.pipeline->addLight(lp.headR, Vector3{2.5f, 2.4f, 2.0f});
}

engine::RenderPipeline::PostParams GameLoop::buildPostParams(Context &ctx)
{
    const Car *car = nullptr;
    bool nitroActive = false;
    float maxSpeed = 1.0f;

    if (ctx.race) {
        const RacerEntry &player =
            ctx.race->racers()[static_cast<size_t>(ctx.race->playerIndex())];
        car = &player.car;
        maxSpeed = player.car.tuning().maxSpeed;
        nitroActive = player.lastInput.nitro
            && player.car.nitroRemaining() > 0.0f;
    } else if (ctx.aurelia) {
        car = &ctx.aurelia->playerCar();
        maxSpeed = car->tuning().maxSpeed;
    }
    if (!car) {
        return engine::RenderPipeline::PostParams{ctx.smoothedSpeedRatio, false};
    }
    if (nitroActive) {
        maxSpeed += car->tuning().nitroMaxSpeedBonus;
    }
    float targetRatio = std::clamp(
        std::fabs(car->speed()) / maxSpeed, 0.0f, 1.0f);
    float dt = std::min(GetFrameTime(), 0.1f);
    float blend = std::min(1.0f, 8.0f * dt);

    ctx.smoothedSpeedRatio += (targetRatio - ctx.smoothedSpeedRatio) * blend;

    return engine::RenderPipeline::PostParams{
        ctx.smoothedSpeedRatio, nitroActive};
}

} // namespace app
} // namespace racer

