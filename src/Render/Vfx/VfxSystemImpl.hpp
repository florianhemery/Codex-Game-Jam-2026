/*
** EPITECH PROJECT, 2026
** racer
** File description:
** VFX system private implementation state
*/

#ifndef VFX_SYSTEM_IMPL_HPP_
#define VFX_SYSTEM_IMPL_HPP_

#include <array>

#include "raylib.h"

#include "Render/Vfx/VfxTypes.hpp"

namespace racer {

struct VfxSystem::Impl {
    std::array<VfxParticle, kMaxVfxParticles> pool_{};
    int count_ = 0;

    Texture2D texPuff_{};
    Texture2D texStreak_{};
    Texture2D texSquare_{};

    float rainIntensity_ = 0.0f;
    float rainTarget_ = 0.0f;
    float rainAccum_ = 0.0f;

    VfxParticle *alloc();
    void resetScalars(VfxParticle *p);
    void initRainDrop(VfxParticle *p, Vector3 focus);
    void initSplashDrop(VfxParticle *p, Vector3 at);
    void spawnRainDrop(Vector3 focus);
    void spawnSplash(Vector3 at);
    void initDriftSmoke(VfxParticle *p, Vector3 pos, Vector3 carVel);
    void initOffroadDust(VfxParticle *p, Vector3 pos, Vector3 carVel);
    void initNitroFlame(
        VfxParticle *p, Vector3 pos, Vector3 backDir, Vector3 carVel);
    void initSpark(VfxParticle *p, Vector3 pos, Vector3 dir, float sp);
    void initConfetti(VfxParticle *p, Vector3 pos);
    void setConfettiColors(VfxParticle *p);
    bool tickVfxParticle(
        int index, float dt,
        std::array<Vector3, kVfxMaxImpactsParFrame> &impacts,
        int &impactCount);
    void updateRainIntensity(float dt);
    void updateRainSpawn(float dt, Vector3 focus);
    void applyDriftSmoke(VfxParticle &p, float dt);
    void applyOffroadDust(VfxParticle &p, float dt);
    void applyNitroFlame(VfxParticle &p, float dt);
    void applySpark(VfxParticle &p, float dt);
    void applyConfetti(VfxParticle &p, float dt);
    void applyTypePhysics(VfxParticle &p, float dt);
    void integrateMotion(VfxParticle &p, float dt);
    bool resolveCollision(
        VfxParticle &p,
        std::array<Vector3, kVfxMaxImpactsParFrame> &impacts,
        int &impactCount);
    void integrateAll(float dt);
};


} // namespace racer

#endif /* !VFX_SYSTEM_IMPL_HPP_ */
