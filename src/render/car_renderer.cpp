#include "render/car_renderer.h"

#include "rlgl.h"

namespace racer {

void DrawCar(const Car& car, Color bodyColor) {
    // Ombre plate (fausse -- pas d'eclairage dynamique), ancre la voiture au sol visuellement.
    DrawCylinder(Vector3{car.position.x, 0.02f, car.position.z}, 1.9f, 1.9f, 0.02f, 14, Fade(BLACK, 0.35f));

    rlPushMatrix();
    rlTranslatef(car.position.x, car.position.y, car.position.z);
    rlRotatef(car.heading * RAD2DEG, 0.0f, 1.0f, 0.0f);

    DrawCube(Vector3{0.0f, 0.4f, 0.0f}, 1.8f, 0.6f, 3.6f, bodyColor);
    DrawCubeWires(Vector3{0.0f, 0.4f, 0.0f}, 1.8f, 0.6f, 3.6f, BLACK);
    DrawCube(Vector3{0.0f, 0.75f, -0.2f}, 1.4f, 0.35f, 1.6f, Fade(BLACK, 0.6f));

    // Aileron arriere.
    DrawCube(Vector3{0.0f, 0.85f, -1.9f}, 1.5f, 0.08f, 0.35f, Fade(BLACK, 0.85f));
    DrawCube(Vector3{-0.65f, 0.6f, -1.9f}, 0.1f, 0.5f, 0.3f, Fade(BLACK, 0.85f));
    DrawCube(Vector3{0.65f, 0.6f, -1.9f}, 0.1f, 0.5f, 0.3f, Fade(BLACK, 0.85f));

    // Phares avant / feux arriere.
    DrawCube(Vector3{-0.6f, 0.45f, 1.78f}, 0.35f, 0.2f, 0.1f, Color{255, 244, 200, 255});
    DrawCube(Vector3{0.6f, 0.45f, 1.78f}, 0.35f, 0.2f, 0.1f, Color{255, 244, 200, 255});
    DrawCube(Vector3{-0.6f, 0.45f, -1.78f}, 0.35f, 0.2f, 0.1f, Color{200, 20, 20, 255});
    DrawCube(Vector3{0.6f, 0.45f, -1.78f}, 0.35f, 0.2f, 0.1f, Color{200, 20, 20, 255});

    constexpr float wx = 0.95f, wy = 0.25f, wz = 1.3f;
    Color wheelColor = DARKGRAY;
    DrawCube(Vector3{wx, wy, wz}, 0.35f, 0.5f, 0.7f, wheelColor);
    DrawCube(Vector3{-wx, wy, wz}, 0.35f, 0.5f, 0.7f, wheelColor);
    DrawCube(Vector3{wx, wy, -wz}, 0.35f, 0.5f, 0.7f, wheelColor);
    DrawCube(Vector3{-wx, wy, -wz}, 0.35f, 0.5f, 0.7f, wheelColor);

    rlPopMatrix();
}

} // namespace racer
