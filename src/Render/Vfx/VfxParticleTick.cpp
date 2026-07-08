/*
** EPITECH PROJECT, 2026
** racer
** File description:
** VFX particle integration and ticking
*/

#include "Render/Vfx/VfxSystemImpl.hpp"

#include <algorithm>
#include <cmath>

namespace racer {

void VfxSystem::Impl::integrateMotion(VfxParticle &p, float dt)
{
    p.pos.x += p.vel.x * dt;
    p.pos.y += p.vel.y * dt;
    p.pos.z += p.vel.z * dt;
}

bool VfxSystem::Impl::resolveCollision(
    VfxParticle &p,
    std::array<Vector3, kVfxMaxImpactsParFrame> &impacts,
    int &impactCount)
{
    bool kill = false;

    if (p.type == VfxPType::SPARK && p.pos.y < kVfxGroundY) {
        p.pos.y = kVfxGroundY;
        p.vel.y *= -0.4f;
        p.vel.x *= 0.75f;
        p.vel.z *= 0.75f;
        p.life *= 0.6f;
    } else if (p.type == VfxPType::OFFROAD_DUST && p.pos.y < 0.05f) {
        p.pos.y = 0.05f;
        if (p.vel.y < 0.0f)
            p.vel.y = 0.0f;
    } else if (p.type == VfxPType::CONFETTI && p.pos.y < 0.03f) {
        p.pos.y = 0.03f;
        p.vel = Vector3{0.0f, 0.0f, 0.0f};
    } else if (p.type == VfxPType::RAIN && p.pos.y <= kVfxGroundY) {
        if (impactCount < kVfxMaxImpactsParFrame)
            impacts[static_cast<std::size_t>(impactCount++)] = p.pos;
        kill = true;
    }
    return kill;
}

bool VfxSystem::Impl::tickVfxParticle(
    int index, float dt,
    std::array<Vector3, kVfxMaxImpactsParFrame> &impacts,
    int &impactCount)
{
    VfxParticle &p = pool_[static_cast<std::size_t>(index)];
    bool kill = false;

    p.life -= dt;
    kill = p.life <= 0.0f;
    if (!kill) {
        applyTypePhysics(p, dt);
        integrateMotion(p, dt);
        kill = resolveCollision(p, impacts, impactCount);
        p.rot += p.rotVel * dt;
        p.size = std::max(0.02f, p.size + p.sizeGrowth * dt);
    }
    if (kill) {
        pool_[static_cast<std::size_t>(index)]
            = pool_[static_cast<std::size_t>(--count_)];
        return true;
    }
    return false;
}

void VfxSystem::Impl::integrateAll(float dt)
{
    std::array<Vector3, kVfxMaxImpactsParFrame> impacts{};
    int impactCount = 0;

    for (int i = 0; i < count_;) {
        if (tickVfxParticle(i, dt, impacts, impactCount))
            continue;
        ++i;
    }
    for (int i = 0; i < impactCount; ++i) {
        const Vector3 &hit = impacts[static_cast<std::size_t>(i)];

        spawnSplash(Vector3{hit.x, kVfxGroundY, hit.z});
    }
}

} // namespace racer

