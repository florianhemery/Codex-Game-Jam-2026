/// \file vfx_demo.cpp
/// \brief Demo autonome du systeme de particules : scenario temporel (fumee de
///        drift, poussiere, nitro, etincelles, confettis, pluie) avec captures
///        automatiques et fermeture auto, sans input.

#include "Render/VfxSystem.hpp"

#include "raylib.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace {

constexpr int kWidth = 960;
constexpr int kHeight = 540;
constexpr float kDt = 1.0f / 60.0f;

void InitCamera(Camera3D& camera)
{
    camera = {};
    camera.position = Vector3{9.5f, 7.0f, 9.5f};
    camera.target = Vector3{0.0f, 1.0f, 0.0f};
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 55.0f;
    camera.projection = CAMERA_PERSPECTIVE;
}

void EmitDriftSalvo(racer::VfxSystem& vfx, float tf)
{
    const float ang = tf * kDt * 1.2f;
    const Vector3 pos{5.5f * std::cos(ang), 0.15f, 5.5f * std::sin(ang)};
    const Vector3 vel{-std::sin(ang) * 6.6f, 0.0f, std::cos(ang) * 6.6f};
    vfx.emitDriftSmoke(pos, vel);
    vfx.emitOffroadDust(Vector3{pos.x * 1.15f, 0.1f, pos.z * 1.15f}, vel);
}

void EmitNitroBurst(racer::VfxSystem& vfx, float tf)
{
    const float a = tf * 0.06f;
    Vector3 backDir{std::cos(a), 0.18f, std::sin(a)};
    const float len = std::sqrt(backDir.x * backDir.x + backDir.y * backDir.y +
                                backDir.z * backDir.z);
    vfx.emitNitroFlame(Vector3{-3.5f, 0.7f, 2.5f},
                       Vector3{backDir.x / len, backDir.y / len, backDir.z / len},
                       Vector3{0.0f, 0.0f, 0.0f});
}

void EmitSparkBurst(racer::VfxSystem& vfx)
{
    vfx.emitSparks(Vector3{3.6f, 0.9f, -2.2f}, Vector3{-0.7f, 0.5f, 0.5f});
}

void DrawSceneMarkers()
{
    DrawPlane(Vector3{0.0f, -0.02f, 0.0f}, Vector2{90.0f, 90.0f},
              Color{47, 53, 62, 255});
    DrawGrid(24, 1.0f);
    DrawCube(Vector3{-3.5f, 0.35f, 2.5f}, 0.7f, 0.7f, 0.7f,
              Color{70, 76, 88, 255});
    DrawCube(Vector3{4.0f, 1.0f, -2.6f}, 0.25f, 2.0f, 2.6f,
              Color{62, 68, 80, 255});
}

void drawHudOverlay(int frame, int activeCount)
{
    DrawText(TextFormat("frame %d", frame), 12, 10, 20, RAYWHITE);
    DrawText(TextFormat("particules actives : %d / 4096", activeCount),
             12, 34, 20, Color{255, 210, 90, 255});
}

void UpdateScenario(racer::VfxSystem& vfx, int frame, float tf)
{
    if (frame % 2 == 0)
        EmitDriftSalvo(vfx, tf);
    if (frame >= 60)
        EmitNitroBurst(vfx, tf);
    if (frame >= 90 && frame <= 112 && frame % 4 == 0)
        EmitSparkBurst(vfx);
    if (frame == 90 || frame == 100)
        vfx.emitConfetti(Vector3{0.0f, 2.6f, 0.0f});
    if (frame == 120)
        vfx.setRain(true);
}

void MaybeScreenshot(int frame)
{
    if (frame == 45)
        TakeScreenshot("vfx_demo_0.png");
    if (frame == 85)
        TakeScreenshot("vfx_demo_1.png");
    if (frame == 115)
        TakeScreenshot("vfx_demo_2.png");
    if (frame == 200)
        TakeScreenshot("vfx_demo_3.png");
}

} // namespace

int main()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(kWidth, kHeight, "vfx_demo particules");
    SetTargetFPS(60);

    Camera3D camera{};
    InitCamera(camera);

    racer::VfxSystem vfx;
    int maxActive = 0;

    for (int frame = 0; frame <= 205 && !WindowShouldClose(); ++frame)
    {
        const float tf = static_cast<float>(frame);
        UpdateScenario(vfx, frame, tf);
        vfx.update(kDt, Vector3{0.0f, 0.0f, 0.0f});
        maxActive = std::max(maxActive, vfx.activeCount());

        BeginDrawing();
        ClearBackground(Color{36, 42, 54, 255});

        BeginMode3D(camera);
        DrawSceneMarkers();
        vfx.draw(camera);
        EndMode3D();

        drawHudOverlay(frame, vfx.activeCount());
        EndDrawing();

        MaybeScreenshot(frame);
    }

    std::printf("[vfx_demo] max particules actives : %d (pool 4096)\n",
                maxActive);
    CloseWindow();
    return (maxActive < 4096) ? 0 : 1;
}
