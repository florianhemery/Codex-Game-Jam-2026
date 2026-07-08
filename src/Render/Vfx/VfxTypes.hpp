/*
** EPITECH PROJECT, 2026
** racer
** File description:
** VFX particle type definitions
*/

#ifndef VFX_TYPES_HPP_
#define VFX_TYPES_HPP_

#include <cstdint>

#include "raylib.h"

namespace racer {

constexpr int kVfxMaxParticles = 4096;
constexpr float kVfxGroundY = 0.0f;
constexpr float kVfxRainRadius = 40.0f;
constexpr float kVfxRainFallSpeed = -22.0f;
constexpr int kVfxMaxImpactsParFrame = 64;

enum class VfxPType : std::uint8_t {
    DRIFT_SMOKE,
    OFFROAD_DUST,
    NITRO_FLAME,
    SPARK,
    CONFETTI,
    RAIN,
    SPLASH
};

struct VfxParticle {
    VfxPType type = VfxPType::DRIFT_SMOKE;
    Vector3 pos{0.0f, 0.0f, 0.0f};
    Vector3 vel{0.0f, 0.0f, 0.0f};
    float life = 0.0f;
    float maxLife = 1.0f;
    float size = 1.0f;
    float sizeGrowth = 0.0f;
    float rot = 0.0f;
    float rotVel = 0.0f;
    float phase = 0.0f;
    Color colorStart{255, 255, 255, 255};
    Color colorEnd{255, 255, 255, 0};
};

struct VfxCamBasis {
    Vector3 right;
    Vector3 up;
    Vector3 fwd;
    Vector3 pos;
};

} // namespace racer

#endif /* !VFX_TYPES_HPP_ */
