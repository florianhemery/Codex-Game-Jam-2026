/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Car visual state for rendering
*/

#ifndef CAR_VISUAL_HPP_
#define CAR_VISUAL_HPP_

namespace racer {

struct CarVisual {
    float steer = 0.0f;
    float wheelSpin = 0.0f;
    bool braking = false;
    bool nitro = false;
    bool headlights = false;
    bool drifting = false;
};

} // namespace racer

#endif /* !CAR_VISUAL_HPP_ */
