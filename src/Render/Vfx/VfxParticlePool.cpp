/*
** EPITECH PROJECT, 2026
** racer
** File description:
** VFX particle simulation pool
*/

#include "Render/Vfx/VfxSystemImpl.hpp"

#include <algorithm>
#include <array>
#include <cmath>

#include "raymath.h"

#include "Render/Vfx/VfxTextureFactory.hpp"

namespace racer {

VfxParticle *VfxSystem::Impl::alloc()
{
    VfxParticle *slot = nullptr;

    if (count_ < kVfxMaxParticles)
        slot = &pool_[static_cast<std::size_t>(count_++)];
    return slot;
}

void VfxSystem::Impl::resetScalars(VfxParticle *p)
{
    p->sizeGrowth = 0.0f;
    p->rot = 0.0f;
    p->rotVel = 0.0f;
    p->phase = 0.0f;
}

void VfxSystem::Impl::initRainDrop(VfxParticle *p, Vector3 focus)
{
    float ang = VfxTextureFactory::frand(0.0f, 2.0f * PI);
    float rad = kVfxRainRadius * std::sqrt(VfxTextureFactory::frand(0.0f, 1.0f));

    p->type = VfxPType::RAIN;
    p->pos = Vector3{
        focus.x + std::cos(ang) * rad,
        focus.y + VfxTextureFactory::frand(12.0f, 18.0f),
        focus.z + std::sin(ang) * rad};
    p->vel = Vector3{
        VfxTextureFactory::frand(-1.8f, 1.8f),
        kVfxRainFallSpeed + VfxTextureFactory::frand(-2.0f, 2.0f),
        VfxTextureFactory::frand(-1.8f, 1.8f)};
    p->maxLife = p->life = 1.4f;
    p->size = VfxTextureFactory::frand(0.1f, 0.14f);
    resetScalars(p);
    p->colorStart = VfxTextureFactory::rgba(190, 212, 240, 205);
    p->colorEnd = VfxTextureFactory::rgba(195, 216, 242, 170);
}

void VfxSystem::Impl::spawnRainDrop(Vector3 focus)
{
    VfxParticle *p = alloc();

    if (p == nullptr)
        return;
    initRainDrop(p, focus);
}

void VfxSystem::Impl::initSplashDrop(VfxParticle *p, Vector3 at)
{
    float ang = VfxTextureFactory::frand(0.0f, 2.0f * PI);
    float sp = VfxTextureFactory::frand(0.9f, 1.9f);

    p->type = VfxPType::SPLASH;
    p->pos = Vector3{at.x, kVfxGroundY + 0.03f, at.z};
    p->vel = Vector3{
        std::cos(ang) * sp,
        VfxTextureFactory::frand(0.15f, 0.4f),
        std::sin(ang) * sp};
    p->maxLife = p->life = 0.1f;
    p->size = 0.05f;
    resetScalars(p);
    p->colorStart = VfxTextureFactory::rgba(190, 210, 235, 150);
    p->colorEnd = VfxTextureFactory::rgba(195, 215, 240, 0);
}

void VfxSystem::Impl::spawnSplash(Vector3 at)
{
    for (int i = 0; i < 2; ++i) {
        VfxParticle *p = alloc();

        if (p == nullptr)
            return;
        initSplashDrop(p, at);
    }
}

void VfxSystem::Impl::updateRainIntensity(float dt)
{
    float step = 0.5f * dt;

    if (rainIntensity_ < rainTarget_)
        rainIntensity_ = std::min(rainTarget_, rainIntensity_ + step);
    else if (rainIntensity_ > rainTarget_)
        rainIntensity_ = std::max(rainTarget_, rainIntensity_ - step);
}

void VfxSystem::Impl::updateRainSpawn(float dt, Vector3 focus)
{
    if (rainIntensity_ > 0.001f) {
        rainAccum_ += 300.0f * rainIntensity_ * dt;
        int n = std::min(static_cast<int>(rainAccum_), 64);

        rainAccum_ -= static_cast<float>(n);
        for (int i = 0; i < n; ++i)
            spawnRainDrop(focus);
    } else {
        rainAccum_ = 0.0f;
    }
}

void VfxSystem::Impl::applyDriftSmoke(VfxParticle &p, float dt)
{
    float d = std::max(0.0f, 1.0f - 1.7f * dt);

    p.vel.x *= d;
    p.vel.z *= d;
    p.vel.y += 0.5f * dt;
}

void VfxSystem::Impl::applyOffroadDust(VfxParticle &p, float dt)
{
    float d = std::max(0.0f, 1.0f - 2.4f * dt);

    p.vel.x *= d;
    p.vel.z *= d;
    p.vel.y -= 2.5f * dt;
}

void VfxSystem::Impl::applyNitroFlame(VfxParticle &p, float dt)
{
    // Trainee faible : un freinage fort (5/s) immobilisait les particules
    // au meme endroit quand la voiture est lente, et leur accumulation
    // additive saturait l'ecran en blanc (flash au depart de course).
    float d = std::max(0.0f, 1.0f - 2.0f * dt);

    p.vel.x *= d;
    p.vel.y *= d;
    p.vel.z *= d;
}

void VfxSystem::Impl::applySpark(VfxParticle &p, float dt)
{
    p.vel.y -= 25.0f * dt;
}

void VfxSystem::Impl::applyConfetti(VfxParticle &p, float dt)
{
    float age = p.maxLife - p.life;
    float d = std::max(0.0f, 1.0f - 0.8f * dt);

    p.vel.y -= 3.5f * dt;
    if (p.vel.y < -1.5f)
        p.vel.y = -1.5f;
    p.vel.x = p.vel.x * d
        + std::cos(age * 2.6f + p.phase) * 2.2f * dt;
    p.vel.z = p.vel.z * d
        + std::sin(age * 2.1f + p.phase * 1.7f) * 2.2f * dt;
}

void VfxSystem::Impl::applyTypePhysics(VfxParticle &p, float dt)
{
    switch (p.type) {
    case VfxPType::DRIFT_SMOKE: applyDriftSmoke(p, dt); break;
    case VfxPType::OFFROAD_DUST: applyOffroadDust(p, dt); break;
    case VfxPType::NITRO_FLAME: applyNitroFlame(p, dt); break;
    case VfxPType::SPARK: applySpark(p, dt); break;
    case VfxPType::CONFETTI: applyConfetti(p, dt); break;
    case VfxPType::RAIN:
    case VfxPType::SPLASH:
        break;
    }
}

void VfxSystem::Impl::initDriftSmoke(
    VfxParticle *p, Vector3 pos, Vector3 carVel)
{
    p->type = VfxPType::DRIFT_SMOKE;
    p->pos = Vector3{
        pos.x + VfxTextureFactory::frand(-0.18f, 0.18f),
        std::max(0.08f, pos.y + VfxTextureFactory::frand(-0.05f, 0.12f)),
        pos.z + VfxTextureFactory::frand(-0.18f, 0.18f)};
    p->vel = Vector3{
        carVel.x * 0.25f + VfxTextureFactory::frand(-0.9f, 0.9f),
        carVel.y * 0.25f + VfxTextureFactory::frand(0.6f, 1.5f),
        carVel.z * 0.25f + VfxTextureFactory::frand(-0.9f, 0.9f)};
    p->maxLife = p->life = VfxTextureFactory::frand(0.8f, 1.4f);
    p->size = VfxTextureFactory::frand(0.4f, 0.6f);
    p->sizeGrowth = VfxTextureFactory::frand(1.3f, 2.1f);
    p->rot = VfxTextureFactory::frand(0.0f, 360.0f);
    p->rotVel = VfxTextureFactory::frand(-80.0f, 80.0f);
    p->phase = 0.0f;
    p->colorStart = VfxTextureFactory::rgba(192, 192, 198, 110);
    p->colorEnd = VfxTextureFactory::rgba(225, 225, 232, 0);
}

void VfxSystem::Impl::initOffroadDust(
    VfxParticle *p, Vector3 pos, Vector3 carVel)
{
    p->type = VfxPType::OFFROAD_DUST;
    p->pos = Vector3{
        pos.x + VfxTextureFactory::frand(-0.25f, 0.25f),
        std::max(0.06f, pos.y),
        pos.z + VfxTextureFactory::frand(-0.25f, 0.25f)};
    p->vel = Vector3{
        carVel.x * 0.2f + VfxTextureFactory::frand(-1.2f, 1.2f),
        VfxTextureFactory::frand(0.15f, 0.6f),
        carVel.z * 0.2f + VfxTextureFactory::frand(-1.2f, 1.2f)};
    p->maxLife = p->life = VfxTextureFactory::frand(0.35f, 0.7f);
    p->size = VfxTextureFactory::frand(0.3f, 0.5f);
    p->sizeGrowth = VfxTextureFactory::frand(1.0f, 1.6f);
    p->rot = VfxTextureFactory::frand(0.0f, 360.0f);
    p->rotVel = VfxTextureFactory::frand(-60.0f, 60.0f);
    p->phase = 0.0f;
    p->colorStart = VfxTextureFactory::rgba(206, 186, 140, 82);
    p->colorEnd = VfxTextureFactory::rgba(214, 198, 164, 0);
}

void VfxSystem::Impl::initNitroFlame(
    VfxParticle *p, Vector3 pos, Vector3 backDir, Vector3 carVel)
{
    float eject = VfxTextureFactory::frand(8.0f, 14.0f);

    p->type = VfxPType::NITRO_FLAME;
    p->pos = Vector3{
        pos.x + backDir.x * VfxTextureFactory::frand(0.0f, 0.15f),
        pos.y + backDir.y * VfxTextureFactory::frand(0.0f, 0.15f),
        pos.z + backDir.z * VfxTextureFactory::frand(0.0f, 0.15f)};
    // Seule une fraction de la vitesse voiture est heritee : a pleine
    // vitesse (carSpeed ~= eject), une addition complete annulerait la
    // vitesse relative et figerait les particules sur place (paquet
    // opaque qui ne se disperse pas -> flash blanc au 1er seconde).
    constexpr float kCarVelInherit = 0.3f;

    // Ejection legerement vers le sol : la camera de poursuite regarde la
    // voiture a travers le panache d'echappement ; des particules a hauteur
    // de caisse recouvraient la carrosserie a l'ecran (voiture blanchie).
    p->vel = Vector3{
        backDir.x * eject + carVel.x * kCarVelInherit
            + VfxTextureFactory::frand(-1.0f, 1.0f),
        VfxTextureFactory::frand(-1.6f, -0.4f),
        backDir.z * eject + carVel.z * kCarVelInherit
            + VfxTextureFactory::frand(-1.0f, 1.0f)};
    // Vie tres courte : le jet meurt colle a l'echappement au lieu de
    // s'accumuler dans le couloir camera->voiture.
    p->maxLife = p->life = VfxTextureFactory::frand(0.06f, 0.10f);
    // Quads petits et bien satures : vus de la camera de poursuite, les
    // flammes sont DEVANT la carrosserie a l'ecran ; des gros quads laiteux
    // recouvraient la voiture et la faisaient paraitre blanche.
    p->size = VfxTextureFactory::frand(0.24f, 0.36f);
    p->sizeGrowth = VfxTextureFactory::frand(-1.3f, -0.9f);
    p->rot = VfxTextureFactory::frand(0.0f, 360.0f);
    p->rotVel = VfxTextureFactory::frand(-200.0f, 200.0f);
    p->phase = 0.0f;
    p->colorStart = VfxTextureFactory::rgba(120, 175, 255, 150);
    p->colorEnd = VfxTextureFactory::rgba(255, 110, 15, 0);
}

void VfxSystem::Impl::initSpark(
    VfxParticle *p, Vector3 pos, Vector3 dir, float sp)
{
    p->type = VfxPType::SPARK;
    p->pos = pos;
    p->vel = Vector3{
        dir.x * sp + VfxTextureFactory::frand(-2.5f, 2.5f),
        dir.y * sp + VfxTextureFactory::frand(-1.0f, 3.0f),
        dir.z * sp + VfxTextureFactory::frand(-2.5f, 2.5f)};
    p->maxLife = p->life = VfxTextureFactory::frand(0.4f, 0.8f);
    p->size = VfxTextureFactory::frand(0.05f, 0.09f);
    resetScalars(p);
    p->colorStart = VfxTextureFactory::rgba(255, 244, 190, 255);
    p->colorEnd = VfxTextureFactory::rgba(255, 120, 30, 0);
}

void VfxSystem::Impl::setConfettiColors(VfxParticle *p)
{
    Color vive = ColorFromHSV(
        VfxTextureFactory::frand(0.0f, 360.0f),
        VfxTextureFactory::frand(0.75f, 0.95f),
        1.0f);

    p->colorStart = Color{vive.r, vive.g, vive.b, 255};
    p->colorEnd = Color{vive.r, vive.g, vive.b, 0};
}

void VfxSystem::Impl::initConfetti(VfxParticle *p, Vector3 pos)
{
    p->type = VfxPType::CONFETTI;
    p->pos = Vector3{
        pos.x + VfxTextureFactory::frand(-0.4f, 0.4f),
        pos.y + VfxTextureFactory::frand(-0.2f, 0.4f),
        pos.z + VfxTextureFactory::frand(-0.4f, 0.4f)};
    p->vel = Vector3{
        VfxTextureFactory::frand(-3.0f, 3.0f),
        VfxTextureFactory::frand(1.5f, 5.5f),
        VfxTextureFactory::frand(-3.0f, 3.0f)};
    p->maxLife = p->life = VfxTextureFactory::frand(3.0f, 5.0f);
    p->size = VfxTextureFactory::frand(0.12f, 0.2f);
    p->sizeGrowth = 0.0f;
    p->rot = VfxTextureFactory::frand(0.0f, 360.0f);
    p->rotVel = VfxTextureFactory::frand(-540.0f, 540.0f);
    p->phase = VfxTextureFactory::frand(0.0f, 6.2831f);
    setConfettiColors(p);
}

} // namespace racer

