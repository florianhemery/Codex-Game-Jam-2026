/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Chase camera setup and smoothing
*/

#include <algorithm>
#include <cmath>

#include "App/CameraController.hpp"

namespace racer {
namespace app {

namespace {

struct CameraTargets {
    Vector3 position;
    Vector3 target;
};

CameraTargets computeDesiredCamera(const Car &playerCar)
{
    Vector3 forward = playerCar.forward();

    return CameraTargets{
        {
            playerCar.position().x - forward.x * 9.0f,
            playerCar.position().y + 4.5f,
            playerCar.position().z - forward.z * 9.0f,
        },
        {
            playerCar.position().x + forward.x * 4.0f,
            playerCar.position().y + 1.0f,
            playerCar.position().z + forward.z * 4.0f,
        },
    };
}

void lerpCameraAxis(float &current, float desired, float t)
{
    current += (desired - current) * t;
}

void applyCameraLerp(
    Camera3D &camera, const CameraTargets &desired, float t)
{
    lerpCameraAxis(camera.position.x, desired.position.x, t);
    lerpCameraAxis(camera.position.y, desired.position.y, t);
    lerpCameraAxis(camera.position.z, desired.position.z, t);
    lerpCameraAxis(camera.target.x, desired.target.x, t);
    lerpCameraAxis(camera.target.y, desired.target.y, t);
    lerpCameraAxis(camera.target.z, desired.target.z, t);
}

} // namespace

void initCamera(Camera3D &camera)
{
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 65.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    camera.position = {0.0f, 8.0f, -12.0f};
    camera.target = {0.0f, 0.0f, 0.0f};
}

void updateCamera(Camera3D &camera, const Car &playerCar, float dt)
{
    float camLerp = std::min(1.0f, 6.0f * dt);
    CameraTargets desired = computeDesiredCamera(playerCar);

    applyCameraLerp(camera, desired, camLerp);
}

} // namespace app
} // namespace racer
