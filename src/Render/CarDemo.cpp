/// \file car_demo.cpp
/// \brief Demo autonome du rendu voiture : 3 voitures aux etats visuels
///        contrastes, camera turntable lente, captures automatiques puis
///        fermeture (aucun input). Rendu hors-ecran en 2x puis reduction.

#include "Render/CarRenderer.hpp"
#include "Vehicle/Car.hpp"

#include "raylib.h"

#include <cmath>

namespace {

constexpr int kCapW = 960;
constexpr int kCapH = 540;

const int kCaptureFrames[3] = {30, 75, 120};

RenderTexture2D LoadCaptureTarget()
{
    RenderTexture2D target = LoadRenderTexture(kCapW * 2, kCapH * 2);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);
    return target;
}

void InitCars(racer::Car& red, racer::Car& blue, racer::Car& green)
{
    red = {};
    red.position = Vector3{-3.2f, 0.0f, 0.0f};
    red.speed = 14.0f;

    blue = {};
    blue.position = Vector3{0.0f, 0.0f, 0.0f};
    blue.speed = 34.0f;

    green = {};
    green.position = Vector3{3.2f, 0.0f, 0.0f};
    green.heading = 0.35f;
    green.velocityHeading = -0.15f;
    green.speed = 22.0f;
}

void InitCamera(Camera3D& camera)
{
    camera = {};
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 40.0f;
    camera.projection = CAMERA_PERSPECTIVE;
}

void UpdateTurntableCamera(Camera3D& camera, float t)
{
    const float angle = 0.55f + t * 0.85f;
    camera.position = Vector3{
        std::sin(angle) * 10.5f, 4.2f, std::cos(angle) * 10.5f};
    camera.target = Vector3{0.0f, 0.5f, 0.0f};
}

racer::CarVisual BuildRedVisual(const racer::Car& car, float t)
{
    racer::CarVisual vis{};
    vis.steer = 0.8f;
    vis.braking = true;
    vis.wheelSpin = t * (car.speed / racer::kWheelRadius) * 0.25f;
    return vis;
}

racer::CarVisual BuildBlueVisual(const racer::Car& car, float t)
{
    racer::CarVisual vis{};
    vis.nitro = true;
    vis.headlights = true;
    vis.wheelSpin = t * (car.speed / racer::kWheelRadius);
    return vis;
}

racer::CarVisual BuildGreenVisual(const racer::Car& car, float t)
{
    racer::CarVisual vis{};
    vis.drifting = true;
    vis.steer = -0.6f;
    vis.wheelSpin = t * (car.speed / racer::kWheelRadius) * 0.6f;
    return vis;
}

void DrawCars(const racer::Car& red, const racer::CarVisual& redVis,
              const racer::Car& blue, const racer::CarVisual& blueVis,
              const racer::Car& green, const racer::CarVisual& greenVis)
{
    racer::DrawCarEx(red, redVis, Color{214, 48, 44, 255});
    racer::DrawCarEx(blue, blueVis, Color{38, 96, 220, 255});
    racer::DrawCarEx(green, greenVis, Color{40, 168, 76, 255});
}

void RenderScene(RenderTexture2D& target, const Camera3D& camera,
                 const racer::Car& red, const racer::CarVisual& redVis,
                 const racer::Car& blue, const racer::CarVisual& blueVis,
                 const racer::Car& green, const racer::CarVisual& greenVis,
                 int frame)
{
    BeginTextureMode(target);
    ClearBackground(Color{58, 62, 70, 255});

    BeginMode3D(camera);
    DrawPlane(Vector3{0.0f, 0.0f, 0.0f}, Vector2{40.0f, 40.0f},
              Color{120, 122, 126, 255});
    DrawGrid(20, 1.0f);
    DrawCars(red, redVis, blue, blueVis, green, greenVis);
    EndMode3D();

    DrawText("rouge: frein+braquage | bleue: nitro+phares | verte: drift",
             20, 20, 40, RAYWHITE);
    DrawText(TextFormat("frame %d", frame), 20, 72, 40, LIGHTGRAY);
    EndTextureMode();
}

void PresentFrame(const RenderTexture2D& target)
{
    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexturePro(target.texture,
                   Rectangle{0.0f, 0.0f, static_cast<float>(target.texture.width),
                             -static_cast<float>(target.texture.height)},
                   Rectangle{0.0f, 0.0f, static_cast<float>(GetRenderWidth()),
                             static_cast<float>(GetRenderHeight())},
                   Vector2{0.0f, 0.0f}, 0.0f, WHITE);
    EndDrawing();
}

void ExportCapture(const RenderTexture2D& target, int index)
{
    Image img = LoadImageFromTexture(target.texture);
    ImageFlipVertical(&img);
    ImageResize(&img, kCapW, kCapH);
    ExportImage(img, TextFormat("car_demo_%d.png", index));
    UnloadImage(img);
}

} // namespace

int main()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(960, 540, "car_demo");
    SetTargetFPS(60);

    RenderTexture2D target = LoadCaptureTarget();

    racer::Car red{};
    racer::Car blue{};
    racer::Car green{};
    InitCars(red, blue, green);

    Camera3D camera{};
    InitCamera(camera);

    int captured = 0;
    int frame = 0;

    while (!WindowShouldClose() && frame <= 130)
    {
        const float t = static_cast<float>(GetTime());
        UpdateTurntableCamera(camera, t);

        const racer::CarVisual redVis = BuildRedVisual(red, t);
        const racer::CarVisual blueVis = BuildBlueVisual(blue, t);
        const racer::CarVisual greenVis = BuildGreenVisual(green, t);

        RenderScene(target, camera, red, redVis, blue, blueVis, green, greenVis,
                    frame);
        PresentFrame(target);

        if (captured < 3 && frame == kCaptureFrames[captured])
        {
            ExportCapture(target, captured);
            ++captured;
        }
        ++frame;
    }

    UnloadRenderTexture(target);
    CloseWindow();
    return 0;
}
