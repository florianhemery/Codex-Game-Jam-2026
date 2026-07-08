/*
** EPITECH PROJECT, 2026
** racer
** File description:
** VFX particle billboard draw pass
*/

#ifndef VFX_DRAW_PASS_HPP_
#define VFX_DRAW_PASS_HPP_

#include "raylib.h"

#include "Render/Vfx/VfxTypes.hpp"

namespace racer {

struct VfxDrawPass {
    static VfxCamBasis makeCamBasis(const Camera3D &cam);
    static unsigned char nearCameraFade(
        const VfxParticle &p, const VfxCamBasis &cb, unsigned char alpha);
    static void emitQuad(
        const Texture2D &tex, Vector3 c, Vector3 hr, Vector3 hu, Color tint);
    static void computeBillboardAxes(
        const VfxParticle &p, const VfxCamBasis &cb, Vector3 &hr, Vector3 &hu);
    static void computeConfettiAxes(
        const VfxParticle &p, const VfxCamBasis &cb, Vector3 &hr, Vector3 &hu);
    static void computeStreakAxes(
        const VfxParticle &p, const VfxCamBasis &cb, Vector3 &hr, Vector3 &hu);
    static void computeAxes(
        const VfxParticle &p, const VfxCamBasis &cb, Vector3 &hr, Vector3 &hu);
    static void drawParticlesOfType(
        const VfxParticle *pool, int count, VfxPType type,
        const Texture2D &tex, const VfxCamBasis &cb);
};

} // namespace racer

#endif /* !VFX_DRAW_PASS_HPP_ */
