/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Particle VFX system (smoke, dust, nitro, rain)
*/

#ifndef VFX_SYSTEM_HPP_
#define VFX_SYSTEM_HPP_

#include <memory>

#include "raylib.h"

namespace racer {

class VfxSystem {
public:
    VfxSystem();
    ~VfxSystem();
    VfxSystem(const VfxSystem &) = delete;
    VfxSystem &operator=(const VfxSystem &) = delete;

    void update(float dt, Vector3 focus);
    void draw(const Camera3D &camera) const;

    void emitDriftSmoke(Vector3 pos, Vector3 carVel);
    void emitOffroadDust(Vector3 pos, Vector3 carVel);
    void emitNitroFlame(Vector3 pos, Vector3 backDir, Vector3 carVel);
    void emitSparks(Vector3 pos, Vector3 dir);
    void emitConfetti(Vector3 pos);
    void setRain(bool enabled);
    int activeCount() const;
    void clear();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace racer

#endif /* !VFX_SYSTEM_HPP_ */
