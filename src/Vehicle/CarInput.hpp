/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Vehicle driver input state
*/

#ifndef CAR_INPUT_HPP_
#define CAR_INPUT_HPP_

namespace racer {

struct CarInput {
    float throttle = 0.0f;
    float steer = 0.0f;
    bool handbrake = false;
    bool nitro = false;
};

} // namespace racer

#endif /* !CAR_INPUT_HPP_ */
