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
#include "Render/Car/CarWheelDraw.hpp"
#include "Vehicle/Car.hpp"

namespace racer {

class CarRenderer {
public:
    static void drawCar(const Car &car, Color bodyColor);
    static void drawCarEx(const Car &car, const CarVisual &vis,
        Color bodyColor, Shader litShader = {});
    static CarLightPoints getCarLightPoints(const Car &car);
};

} // namespace racer

#endif /* !CAR_RENDERER_HPP_ */
