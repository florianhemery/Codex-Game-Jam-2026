/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Car-to-car contact resolution (separation, damping, deflection)
*/

#ifndef RACE_CONTACT_RESOLVER_HPP_
#define RACE_CONTACT_RESOLVER_HPP_

#include <cstddef>
#include <vector>

#include "Race/RaceState.hpp"
#include "Vehicle/Car.hpp"

namespace racer {

class RaceContactResolver {
public:
    static void resolveAll(std::vector<RacerEntry>& racers);

private:
    static float normalizeAngle(float angle);
    static float sign(float value);

    static void resolvePair(
        std::vector<RacerEntry>& racers, size_t i, size_t j);
    static bool tryPrepareContact(
        const std::vector<RacerEntry>& racers,
        size_t i,
        size_t j,
        float& nx,
        float& nz,
        float& overlap);
    static bool isWithinContactRange(float distSq);
    static void computeContactNormal(
        const Car& a, float dx, float dz, float distSq,
        float& nx, float& nz, float& dist);
    static void applyFallbackNormal(
        const Car& a, float& nx, float& nz, float& dist);
    static void applyContactSeparation(
        Car& a, Car& b, float nx, float nz, float overlap);
    static void applyContactDamping(Car& a, Car& b, float nx, float nz);
    static void applyContactDeflection(
        Car& a, Car& b, float nx, float nz, float overlap);
    static void computeDeflectScalars(
        float overlap, float& push, float& deflect);
    static void computeHeadingSides(
        const Car& a,
        const Car& b,
        float nx,
        float nz,
        float& sideA,
        float& sideB);
    static void applyHeadingDeflection(
        Car& a, Car& b, float sideA, float sideB, float deflect);
    static void applyLateralNudges(
        Car& a,
        Car& b,
        float nx,
        float nz,
        float push,
        float sideA,
        float sideB);
    static void nudgeIfApproaching(
        Car& car,
        float nx,
        float nz,
        float push,
        float sideSign,
        bool approaching);
    static void nudgeLateral(
        Car& car, float fwdX, float fwdZ, float push, float sideSign);
};

} // namespace racer

#endif /* !RACE_CONTACT_RESOLVER_HPP_ */
