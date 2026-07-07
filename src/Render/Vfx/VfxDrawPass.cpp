/*
** EPITECH PROJECT, 2026
** racer
** File description:
** VFX particle billboard draw pass
*/

#include "Render/Vfx/VfxDrawPass.hpp"

#include <algorithm>
#include <cmath>

#include "raymath.h"
#include "rlgl.h"

#include "Render/Vfx/VfxTextureFactory.hpp"

namespace racer {

VfxCamBasis VfxDrawPass::makeCamBasis(const Camera3D &cam)
{
    Matrix v = MatrixLookAt(cam.position, cam.target, cam.up);
    VfxCamBasis b;

    b.right = Vector3{v.m0, v.m4, v.m8};
    b.up = Vector3{v.m1, v.m5, v.m9};
    b.fwd = Vector3Normalize(Vector3Subtract(cam.target, cam.position));
    return b;
}

void VfxDrawPass::emitQuad(
    const Texture2D &tex, Vector3 c, Vector3 hr, Vector3 hu, Color tint)
{
    rlCheckRenderBatchLimit(4);
    rlSetTexture(tex.id);
    rlBegin(RL_QUADS);
    rlColor4ub(tint.r, tint.g, tint.b, tint.a);
    rlTexCoord2f(0.0f, 1.0f);
    rlVertex3f(c.x - hr.x - hu.x, c.y - hr.y - hu.y, c.z - hr.z - hu.z);
    rlTexCoord2f(1.0f, 1.0f);
    rlVertex3f(c.x + hr.x - hu.x, c.y + hr.y - hu.y, c.z + hr.z - hu.z);
    rlTexCoord2f(1.0f, 0.0f);
    rlVertex3f(c.x + hr.x + hu.x, c.y + hr.y + hu.y, c.z + hr.z + hu.z);
    rlTexCoord2f(0.0f, 0.0f);
    rlVertex3f(c.x - hr.x + hu.x, c.y - hr.y + hu.y, c.z - hr.z + hu.z);
    rlEnd();
}

void VfxDrawPass::computeBillboardAxes(
    const VfxParticle &p, const VfxCamBasis &cb, Vector3 &hr, Vector3 &hu)
{
    float half = p.size * 0.5f;
    float cr = std::cos(p.rot * DEG2RAD);
    float sr = std::sin(p.rot * DEG2RAD);

    hr = Vector3Add(
        Vector3Scale(cb.right, cr * half),
        Vector3Scale(cb.up, sr * half));
    hu = Vector3Add(
        Vector3Scale(cb.right, -sr * half),
        Vector3Scale(cb.up, cr * half));
}

void VfxDrawPass::computeConfettiAxes(
    const VfxParticle &p, const VfxCamBasis &cb, Vector3 &hr, Vector3 &hu)
{
    float age = p.maxLife - p.life;
    float flip = std::sin(p.phase + age * 7.0f);
    float halfW = p.size * 0.5f * flip;
    float halfH = p.size * 0.5f;
    float cr = std::cos(p.rot * DEG2RAD);
    float sr = std::sin(p.rot * DEG2RAD);

    hr = Vector3Add(
        Vector3Scale(cb.right, cr * halfW),
        Vector3Scale(cb.up, sr * halfW));
    hu = Vector3Add(
        Vector3Scale(cb.right, -sr * halfH),
        Vector3Scale(cb.up, cr * halfH));
}

void VfxDrawPass::computeStreakAxes(
    const VfxParticle &p, const VfxCamBasis &cb, Vector3 &hr, Vector3 &hu)
{
    Vector3 dir = p.vel;
    float speed = Vector3Length(dir);
    float halfLen = 0.7f;

    dir = (speed > 0.001f)
        ? Vector3Scale(dir, 1.0f / speed)
        : Vector3{0.0f, 1.0f, 0.0f};
    if (p.type == VfxPType::SPARK)
        halfLen = std::clamp(speed * 0.035f, 0.08f, 0.35f);
    else if (p.type == VfxPType::SPLASH)
        halfLen = 0.09f;
    hu = Vector3Scale(dir, halfLen);
    Vector3 side = Vector3CrossProduct(dir, cb.fwd);
    float sideLen = Vector3Length(side);

    side = (sideLen > 0.001f)
        ? Vector3Scale(side, 1.0f / sideLen)
        : cb.right;
    hr = Vector3Scale(side, p.size * 0.5f);
}

void VfxDrawPass::computeAxes(
    const VfxParticle &p, const VfxCamBasis &cb, Vector3 &hr, Vector3 &hu)
{
    switch (p.type) {
        case VfxPType::DRIFT_SMOKE:
        case VfxPType::OFFROAD_DUST:
        case VfxPType::NITRO_FLAME:
            computeBillboardAxes(p, cb, hr, hu);
            break;
        case VfxPType::CONFETTI:
            computeConfettiAxes(p, cb, hr, hu);
            break;
        case VfxPType::SPARK:
        case VfxPType::RAIN:
        case VfxPType::SPLASH:
            computeStreakAxes(p, cb, hr, hu);
            break;
    }
}

void VfxDrawPass::drawVfxParticlesOfType(
    const VfxParticle *pool, int count, PType type,
    const Texture2D &tex, const VfxCamBasis &cb)
{
    for (int i = 0; i < count; ++i) {
        const VfxParticle &p = pool[i];

        if (p.type != type)
            continue;
        float t = 1.0f - p.life / p.maxLife;
        Color col = VfxTextureFactory::lerpColor(p.colorStart, p.colorEnd, t);
        Vector3 hr{0.0f, 0.0f, 0.0f};
        Vector3 hu{0.0f, 0.0f, 0.0f};

        computeAxes(p, cb, hr, hu);
        emitQuad(tex, p.pos, hr, hu, col);
    }
}

} // namespace racer
