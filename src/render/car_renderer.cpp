#include "render/car_renderer.h"

#include "rlgl.h"

namespace racer {

void DrawCar(const Car& car, Color bodyColor) {
    rlPushMatrix();
    rlTranslatef(car.position.x, car.position.y, car.position.z);
    rlRotatef(car.heading * RAD2DEG, 0.0f, 1.0f, 0.0f);

    DrawCube(Vector3{0.0f, 0.4f, 0.0f}, 1.8f, 0.6f, 3.6f, bodyColor);
    DrawCubeWires(Vector3{0.0f, 0.4f, 0.0f}, 1.8f, 0.6f, 3.6f, BLACK);
    DrawCube(Vector3{0.0f, 0.75f, -0.2f}, 1.4f, 0.35f, 1.6f, Fade(BLACK, 0.6f));

    constexpr float wx = 0.95f, wy = 0.25f, wz = 1.3f;
    Color wheelColor = DARKGRAY;
    DrawCube(Vector3{wx, wy, wz}, 0.35f, 0.5f, 0.7f, wheelColor);
    DrawCube(Vector3{-wx, wy, wz}, 0.35f, 0.5f, 0.7f, wheelColor);
    DrawCube(Vector3{wx, wy, -wz}, 0.35f, 0.5f, 0.7f, wheelColor);
    DrawCube(Vector3{-wx, wy, -wz}, 0.35f, 0.5f, 0.7f, wheelColor);

    rlPopMatrix();
}

} // namespace racer
