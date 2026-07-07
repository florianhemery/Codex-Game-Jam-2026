/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Particle VFX system facade
*/

#include "Render/Vfx/VfxSystem.hpp"

#include "Render/Vfx/VfxDrawPass.hpp"
#include "Render/Vfx/VfxSystemImpl.hpp"
#include "Render/Vfx/VfxTextureFactory.hpp"

#include "rlgl.h"

namespace racer {

VfxSystem::VfxSystem() : impl_(std::make_unique<Impl>())
{
    impl_->texPuff_ = VfxTextureFactory::makePuffTexture();
    impl_->texStreak_ = VfxTextureFactory::makeStreakTexture();
    impl_->texSquare_ = VfxTextureFactory::makeSquareTexture();
}

VfxSystem::~VfxSystem()
{
    UnloadTexture(impl_->texPuff_);
    UnloadTexture(impl_->texStreak_);
    UnloadTexture(impl_->texSquare_);
}

void VfxSystem::update(float dt, Vector3 focus)
{
    if (dt <= 0.0f)
        return;
    dt = std::min(dt, 0.1f);
    impl_->updateRainIntensity(dt);
    impl_->updateRainSpawn(dt, focus);
    impl_->integrateAll(dt);
}

void VfxSystem::draw(const Camera3D &camera) const
{
    const Impl &s = *impl_;

    if (s.count_ == 0)
        return;
    VfxCamBasis cb = VfxDrawPass::makeCamBasis(camera);

    rlDrawRenderBatchActive();
    rlDisableDepthMask();
    rlDisableBackfaceCulling();
    BeginBlendMode(BLEND_ALPHA);
    VfxDrawPass::drawParticlesOfType(
        s.pool_.data(), s.count_, VfxPType::DRIFT_SMOKE, s.texPuff_, cb);
    VfxDrawPass::drawParticlesOfType(
        s.pool_.data(), s.count_, VfxPType::OFFROAD_DUST, s.texPuff_, cb);
    VfxDrawPass::drawParticlesOfType(
        s.pool_.data(), s.count_, VfxPType::CONFETTI, s.texSquare_, cb);
    VfxDrawPass::drawParticlesOfType(
        s.pool_.data(), s.count_, VfxPType::SPLASH, s.texStreak_, cb);
    VfxDrawPass::drawParticlesOfType(
        s.pool_.data(), s.count_, VfxPType::RAIN, s.texStreak_, cb);
    BeginBlendMode(BLEND_ADDITIVE);
    VfxDrawPass::drawParticlesOfType(
        s.pool_.data(), s.count_, VfxPType::NITRO_FLAME, s.texPuff_, cb);
    VfxDrawPass::drawParticlesOfType(
        s.pool_.data(), s.count_, VfxPType::SPARK, s.texStreak_, cb);
    EndBlendMode();
    rlDrawRenderBatchActive();
    rlEnableBackfaceCulling();
    rlEnableDepthMask();
}

void VfxSystem::emitDriftSmoke(Vector3 pos, Vector3 carVel)
{
    int n = GetRandomValue(2, 3);

    for (int i = 0; i < n; ++i) {
        VfxParticle *p = impl_->alloc();

        if (p == nullptr)
            break;
        impl_->initDriftSmoke(p, pos, carVel);
    }
}

void VfxSystem::emitOffroadDust(Vector3 pos, Vector3 carVel)
{
    int n = GetRandomValue(1, 2);

    for (int i = 0; i < n; ++i) {
        VfxParticle *p = impl_->alloc();

        if (p == nullptr)
            break;
        impl_->initOffroadDust(p, pos, carVel);
    }
}

void VfxSystem::emitNitroFlame(Vector3 pos, Vector3 backDir, Vector3 carVel)
{
    int n = GetRandomValue(4, 5);

    for (int i = 0; i < n; ++i) {
        VfxParticle *p = impl_->alloc();

        if (p == nullptr)
            break;
        impl_->initNitroFlame(p, pos, backDir, carVel);
    }
}

void VfxSystem::emitSparks(Vector3 pos, Vector3 dir)
{
    float dl = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
    Vector3 d = (dl > 0.001f)
        ? Vector3{dir.x / dl, dir.y / dl, dir.z / dl}
        : Vector3{0.0f, 1.0f, 0.0f};
    int n = GetRandomValue(10, 16);

    for (int i = 0; i < n; ++i) {
        VfxParticle *p = impl_->alloc();

        if (p == nullptr)
            break;
        impl_->initSpark(p, pos, d, VfxTextureFactory::frand(5.0f, 13.0f));
    }
}

void VfxSystem::emitConfetti(Vector3 pos)
{
    int n = GetRandomValue(45, 70);

    for (int i = 0; i < n; ++i) {
        VfxParticle *p = impl_->alloc();

        if (p == nullptr)
            break;
        impl_->initConfetti(p, pos);
    }
}

void VfxSystem::setRain(bool enabled)
{
    impl_->rainTarget_ = enabled ? 1.0f : 0.0f;
}

int VfxSystem::activeCount() const
{
    return impl_->count_;
}

void VfxSystem::clear()
{
    impl_->count_ = 0;
    impl_->rainAccum_ = 0.0f;
}

} // namespace racer
