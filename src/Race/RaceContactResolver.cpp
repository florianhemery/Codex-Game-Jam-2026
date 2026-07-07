/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Car-to-car contact resolution (separation, damping, deflection)
*/

#include "Race/RaceContactResolver.hpp"

#include <algorithm>
#include <cmath>

namespace racer {

namespace {

constexpr float kContactDist = 3.0f;
constexpr float kMaxPush = 0.25f;
constexpr float kMaxDeflect = 0.06f;
constexpr float kSpeedDamping = 0.96f;

} // namespace

float RaceContactResolver::normalizeAngle(float angle)
{
    while (angle > PI) {
        angle -= 2.0f * PI;
    }
    while (angle < -PI) {
        angle += 2.0f * PI;
    }
    return angle;
}

float RaceContactResolver::sign(float value)
{
    if (value > 0.0f) {
        return 1.0f;
    }
    if (value < 0.0f) {
        return -1.0f;
    }
    return 0.0f;
}

void RaceContactResolver::resolveAll(std::vector<RacerEntry>& racers)
{
    for (size_t i = 0; i < racers.size(); ++i) {
        if (racers[i].finished) {
            continue;
        }
        for (size_t j = i + 1; j < racers.size(); ++j) {
            if (racers[j].finished) {
                continue;
            }
            resolvePair(racers, i, j);
        }
    }
}

bool RaceContactResolver::isWithinContactRange(float distSq)
{
    return distSq < kContactDist * kContactDist;
}

void RaceContactResolver::applyFallbackNormal(
    const Car& a, float& nx, float& nz, float& dist)
{
    Vector3 fwd = a.forward();
    nx = fwd.x;
    nz = fwd.z;
    dist = 0.0f;
}

void RaceContactResolver::computeContactNormal(
    const Car& a, float dx, float dz, float distSq,
    float& nx, float& nz, float& dist)
{
    dist = std::sqrt(distSq);
    if (dist > 1e-4f) {
        nx = dx / dist;
        nz = dz / dist;
        return;
    }
    applyFallbackNormal(a, nx, nz, dist);
}

bool RaceContactResolver::tryPrepareContact(
    const std::vector<RacerEntry>& racers, size_t i, size_t j,
    float& nx, float& nz, float& overlap)
{
    const Car& a = racers[i].car;
    const Car& b = racers[j].car;
    float dx = b.position().x - a.position().x;
    float dz = b.position().z - a.position().z;
    float distSq = dx * dx + dz * dz;

    if (!isWithinContactRange(distSq)) {
        return false;
    }
    float dist = 0.0f;
    computeContactNormal(a, dx, dz, distSq, nx, nz, dist);
    overlap = kContactDist - dist;
    return true;
}

void RaceContactResolver::resolvePair(
    std::vector<RacerEntry>& racers, size_t i, size_t j)
{
    float nx = 0.0f;
    float nz = 0.0f;
    float overlap = 0.0f;

    if (!tryPrepareContact(racers, i, j, nx, nz, overlap)) {
        return;
    }
    Car& a = racers[i].car;
    Car& b = racers[j].car;

    applyContactSeparation(a, b, nx, nz, overlap);
    applyContactDamping(a, b, nx, nz);
    applyContactDeflection(a, b, nx, nz, overlap);
}

void RaceContactResolver::applyContactSeparation(
    Car& a, Car& b, float nx, float nz, float overlap)
{
    float push = std::min(overlap * 0.25f, kMaxPush);

    a.position().x -= nx * push;
    a.position().z -= nz * push;
    b.position().x += nx * push;
    b.position().z += nz * push;
}

void RaceContactResolver::applyContactDamping(Car& a, Car& b, float nx, float nz)
{
    Vector3 va = a.velocity();
    Vector3 vb = b.velocity();
    float closing = (va.x - vb.x) * nx + (va.z - vb.z) * nz;
    float t = std::clamp(closing / 6.0f, 0.0f, 1.0f);
    float damping = 1.0f - (1.0f - kSpeedDamping) * t;
    bool aRams = va.x * nx + va.z * nz > 0.0f;
    bool bRams = vb.x * nx + vb.z * nz < 0.0f;

    if (aRams) {
        a.speed() *= damping;
    }
    if (bRams) {
        b.speed() *= damping;
    }
}

void RaceContactResolver::computeDeflectScalars(
    float overlap, float& push, float& deflect)
{
    push = std::min(overlap * 0.25f, kMaxPush);
    deflect = std::min(kMaxDeflect, overlap * 0.04f);
}

void RaceContactResolver::computeHeadingSides(
    const Car& a,
    const Car& b,
    float nx,
    float nz,
    float& sideA,
    float& sideB)
{
    float ax = std::sin(a.velocityHeading());
    float az = std::cos(a.velocityHeading());
    float bx = std::sin(b.velocityHeading());
    float bz = std::cos(b.velocityHeading());

    sideA = sign(ax * nz - az * nx);
    sideB = sign(bx * (-nz) - bz * (-nx));
}

void RaceContactResolver::applyHeadingDeflection(
    Car& a, Car& b, float sideA, float sideB, float deflect)
{
    a.velocityHeading() = normalizeAngle(
        a.velocityHeading() + sideA * deflect);
    b.velocityHeading() = normalizeAngle(
        b.velocityHeading() + sideB * deflect);
}

void RaceContactResolver::nudgeLateral(
    Car& car, float fwdX, float fwdZ, float push, float sideSign)
{
    car.position().x += sideSign * fwdZ * push * 0.6f;
    car.position().z += sideSign * (-fwdX) * push * 0.6f;
}

void RaceContactResolver::nudgeIfApproaching(
    Car& car,
    float nx,
    float nz,
    float push,
    float sideSign,
    bool approaching)
{
    if (!approaching) {
        return;
    }
    float fwdX = std::sin(car.velocityHeading());
    float fwdZ = std::cos(car.velocityHeading());

    nudgeLateral(car, fwdX, fwdZ, push, sideSign);
}

void RaceContactResolver::applyLateralNudges(
    Car& a,
    Car& b,
    float nx,
    float nz,
    float push,
    float sideA,
    float sideB)
{
    Vector3 va = a.velocity();
    Vector3 vb = b.velocity();

    nudgeIfApproaching(
        a, nx, nz, push, sideA, va.x * nx + va.z * nz > 0.0f);
    nudgeIfApproaching(
        b, nx, nz, push, sideB, vb.x * nx + vb.z * nz < 0.0f);
}

void RaceContactResolver::applyContactDeflection(
    Car& a, Car& b, float nx, float nz, float overlap)
{
    float push = 0.0f;
    float deflect = 0.0f;
    float sideA = 0.0f;
    float sideB = 0.0f;

    computeDeflectScalars(overlap, push, deflect);
    computeHeadingSides(a, b, nx, nz, sideA, sideB);
    applyHeadingDeflection(a, b, sideA, sideB, deflect);
    applyLateralNudges(a, b, nx, nz, push, sideA, sideB);
}

} // namespace racer
