/*
** EPITECH PROJECT, 2026
** racer
** File description:
** World-space anchor points for car lights and exhaust
*/

#ifndef CAR_LIGHT_POINTS_HPP_
#define CAR_LIGHT_POINTS_HPP_

#include "raylib.h"

namespace racer {

struct CarLightPoints {
    Vector3 headL;
    Vector3 headR;
    Vector3 brakeL;
    Vector3 brakeR;
    Vector3 exhaust;
};

} // namespace racer

#endif /* !CAR_LIGHT_POINTS_HPP_ */
