/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Vehicle runtime state and accessors
*/

#ifndef CAR_STATE_HPP_
#define CAR_STATE_HPP_

#include "raylib.h"

#include "Vehicle/CarTuning.hpp"

namespace racer {

struct CarState {
    Vector3 position_{0.0f, 0.0f, 0.0f};
    float heading_ = 0.0f;
    float speed_ = 0.0f;
    float velocityHeading_ = 0.0f;
    float nitroRemaining_ = 3.0f;
    bool isDrifting_ = false;
    float surfaceGrip_ = 1.0f;
    float surfaceDrag_ = 1.0f;
    float verticalVelocity_ = 0.0f;
    bool airborne_ = false;
    float startBoostTimer_ = 0.0f;
    float startBoostAccelMul_ = 1.0f;
    float startBoostSpeedBonus_ = 0.0f;
    // Smoothed grip value actually used to blend velocityHeading toward
    // heading (see Car::updateVelocityHeading). Kept as persistent state
    // rather than recomputed fresh each frame so drift-enter/exit and
    // surface-grip changes can be low-pass filtered instead of snapping
    // instantly -- see CarTelemetry-driven tuning notes in Car.cpp.
    float headingGrip_ = 6.0f;
    CarTuning tuning_{};

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

    float &verticalVelocity()
    {
        return verticalVelocity_;
    }

    float verticalVelocity() const
    {
        return verticalVelocity_;
    }

    bool &airborne()
    {
        return airborne_;
    }

    bool airborne() const
    {
        return airborne_;
    }

    CarTuning &tuning()
    {
        return tuning_;
    }

    const CarTuning &tuning() const
    {
        return tuning_;
    }

    float &startBoostTimer()
    {
        return startBoostTimer_;
    }

    float startBoostTimer() const
    {
        return startBoostTimer_;
    }

    float &startBoostAccelMul()
    {
        return startBoostAccelMul_;
    }

    float startBoostAccelMul() const
    {
        return startBoostAccelMul_;
    }

    float &startBoostSpeedBonus()
    {
        return startBoostSpeedBonus_;
    }

    float startBoostSpeedBonus() const
    {
        return startBoostSpeedBonus_;
    }

    float &headingGrip()
    {
        return headingGrip_;
    }

    float headingGrip() const
    {
        return headingGrip_;
    }
};

} // namespace racer

#endif /* !CAR_STATE_HPP_ */
