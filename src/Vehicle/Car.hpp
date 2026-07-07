/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Arcade vehicle model and physics
*/

#ifndef CAR_HPP_
#define CAR_HPP_

#include "raylib.h"

#include "Vehicle/CarInput.hpp"
#include "Vehicle/CarState.hpp"

namespace racer {

class Car : public CarState {
public:
    Car() = default;

    void update(const CarInput &input, float dt);
    Vector3 forward() const;
    Vector3 velocity() const;

private:
    static float normalizeAngle(float angle);
    static float sign(float value);
    static bool updateNitro(Car &car, const CarInput &input, float dt);
    static void applyEngineAndDrag(
        Car &car, const CarInput &input, float dt, bool nitroActive);
    static void updateHeading(Car &car, const CarInput &input, float dt);
    static void updateVelocityHeading(Car &car, float dt);
    static void integratePosition(Car &car, float dt);
};

} // namespace racer

#endif /* !CAR_HPP_ */
