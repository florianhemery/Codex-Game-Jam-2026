/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Chase camera setup and smoothing
*/

#ifndef CAMERA_CONTROLLER_HPP_
#define CAMERA_CONTROLLER_HPP_

#include "raylib.h"

#include "Vehicle/Car.hpp"

namespace racer {
namespace app {

void initCamera(Camera3D &camera);
void updateCamera(Camera3D &camera, const Car &playerCar, float dt);

} // namespace app
} // namespace racer

#endif
