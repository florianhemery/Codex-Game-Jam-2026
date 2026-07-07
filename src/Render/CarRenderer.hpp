/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Car visual rendering
*/

#ifndef CAR_RENDERER_HPP_
#define CAR_RENDERER_HPP_

#include "raylib.h"

#include "Render/CarLightPoints.hpp"
#include "Render/CarVisual.hpp"
#include "Vehicle/Car.hpp"

namespace racer {

inline constexpr float kWheelRadius = 0.35f;

class CarRenderer {
public:
    static void drawCar(const Car &car, Color bodyColor);
    static void drawCarEx(const Car &car, const CarVisual &vis, Color bodyColor);
    static CarLightPoints getCarLightPoints(const Car &car);
};

} // namespace racer

#endif /* !CAR_RENDERER_HPP_ */
