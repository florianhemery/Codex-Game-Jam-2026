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
#include "Vehicle/CarTuning.hpp"

namespace racer {

class Car {
public:
    Car() = default;

    Vector3 &position()
    {
        return position_;
    }

    const Vector3 &position() const
    {
        return position_;
    }

    float &heading()
    {
        return heading_;
    }

    float heading() const
    {
        return heading_;
    }

    float &speed()
    {
        return speed_;
    }

    float speed() const
    {
        return speed_;
    }

    float &velocityHeading()
    {
        return velocityHeading_;
    }

    float velocityHeading() const
    {
        return velocityHeading_;
    }

    float &nitroRemaining()
    {
        return nitroRemaining_;
    }

    float nitroRemaining() const
    {
        return nitroRemaining_;
    }

    bool &isDrifting()
    {
        return isDrifting_;
    }

    bool isDrifting() const
    {
        return isDrifting_;
    }

    float &surfaceGrip()
    {
        return surfaceGrip_;
    }

    float surfaceGrip() const
    {
        return surfaceGrip_;
    }

    float &surfaceDrag()
    {
        return surfaceDrag_;
    }

    float surfaceDrag() const
    {
        return surfaceDrag_;
    }

    CarTuning &tuning()
    {
        return tuning_;
    }

    const CarTuning &tuning() const
    {
        return tuning_;
    }

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

    Vector3 position_{0.0f, 0.0f, 0.0f};
    float heading_ = 0.0f;
    float speed_ = 0.0f;
    float velocityHeading_ = 0.0f;
    float nitroRemaining_ = 3.0f;
    bool isDrifting_ = false;
    float surfaceGrip_ = 1.0f;
    float surfaceDrag_ = 1.0f;
    CarTuning tuning_{};
};

} // namespace racer

#endif /* !CAR_HPP_ */
