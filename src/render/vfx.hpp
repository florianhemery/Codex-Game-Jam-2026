/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Particle VFX system (smoke, dust, nitro, rain)
*/

#ifndef VFX_HPP_
#define VFX_HPP_

#include <memory>

#include "raylib.h"

namespace racer {

class VfxSystem {
public:
    VfxSystem();
    ~VfxSystem();
    VfxSystem(const VfxSystem &) = delete;
    VfxSystem &operator=(const VfxSystem &) = delete;

    void Update(float dt, Vector3 focus);
    void Draw(const Camera3D &camera) const;

    void EmitDriftSmoke(Vector3 pos, Vector3 carVel);
    void EmitOffroadDust(Vector3 pos, Vector3 carVel);
    void EmitNitroFlame(Vector3 pos, Vector3 backDir, Vector3 carVel);
    void EmitSparks(Vector3 pos, Vector3 dir);
    void EmitConfetti(Vector3 pos);
    void SetRain(bool enabled);
    int ActiveCount() const;
    void Clear();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace racer

#endif /* !VFX_HPP_ */
