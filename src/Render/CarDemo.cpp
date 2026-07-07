/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Standalone car renderer demo with automatic captures
*/

#include "Render/CarRenderer.hpp"
#include "Vehicle/Car.hpp"

#include "raylib.h"

#include <cmath>

namespace {

constexpr int kCapW = 960;
constexpr int kCapH = 540;

const int kCaptureFrames[3] = {30, 75, 120};

class CarDemoApp {
public:
    static int run();

private:
    struct Gfx {
        static void clearBackground(Color color);
        static void drawText(const char *text, int x, int y, int fontSize,
            Color color);
        static void drawTexturePro(Texture2D texture, Rectangle source,
            Rectangle dest, Vector2 origin, float rotation, Color tint);
        static void drawPlane(Vector3 center, Vector2 size, Color color);
        static void drawGrid(int slices, float spacing);
    };

    static RenderTexture2D loadCaptureTarget();
    static void initCars(racer::Car &red, racer::Car &blue, racer::Car &green);
    static void initCamera(Camera3D &camera);
    static void updateTurntableCamera(Camera3D &camera, float t);
    static racer::CarVisual buildRedVisual(const racer::Car &car, float t);
    static racer::CarVisual buildBlueVisual(const racer::Car &car, float t);
    static racer::CarVisual buildGreenVisual(const racer::Car &car, float t);
    static void drawCars(const racer::Car &red, const racer::CarVisual &redVis,
        const racer::Car &blue, const racer::CarVisual &blueVis,
        const racer::Car &green, const racer::CarVisual &greenVis);
    static void renderScene(RenderTexture2D &target, const Camera3D &camera,
        const racer::Car &red, const racer::CarVisual &redVis,
        const racer::Car &blue, const racer::CarVisual &blueVis,
        const racer::Car &green, const racer::CarVisual &greenVis, int frame);
    static void presentFrame(const RenderTexture2D &target);
    static void exportCapture(const RenderTexture2D &target, int index);
};

void CarDemoApp::Gfx::clearBackground(Color color)
{
    ClearBackground(color);
}

void CarDemoApp::Gfx::drawText(const char *text, int x, int y, int fontSize,
    Color color)
{
    DrawText(text, x, y, fontSize, color);
}

void CarDemoApp::Gfx::drawTexturePro(Texture2D texture, Rectangle source,
    Rectangle dest, Vector2 origin, float rotation, Color tint)
{
    DrawTexturePro(texture, source, dest, origin, rotation, tint);
}

void CarDemoApp::Gfx::drawPlane(Vector3 center, Vector2 size, Color color)
{
    DrawPlane(center, size, color);
}

void CarDemoApp::Gfx::drawGrid(int slices, float spacing)
{
    DrawGrid(slices, spacing);
}

RenderTexture2D CarDemoApp::loadCaptureTarget()
{
    RenderTexture2D target = LoadRenderTexture(kCapW * 2, kCapH * 2);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);
    return target;
}

void CarDemoApp::initCars(racer::Car &red, racer::Car &blue, racer::Car &green)
{
    red = {};
    red.position() = Vector3{-3.2f, 0.0f, 0.0f};
    red.speed() = 14.0f;

    blue = {};
    blue.position() = Vector3{0.0f, 0.0f, 0.0f};
    blue.speed() = 34.0f;

    green = {};
    green.position() = Vector3{3.2f, 0.0f, 0.0f};
    green.heading() = 0.35f;
    green.velocityHeading() = -0.15f;
    green.speed() = 22.0f;
}

void CarDemoApp::initCamera(Camera3D &camera)
{
    camera = {};
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 40.0f;
    camera.projection = CAMERA_PERSPECTIVE;
}

void CarDemoApp::updateTurntableCamera(Camera3D &camera, float t)
{
    const float angle = 0.55f + t * 0.85f;
    camera.position = Vector3{
        std::sin(angle) * 10.5f, 4.2f, std::cos(angle) * 10.5f};
    camera.target = Vector3{0.0f, 0.5f, 0.0f};
}

racer::CarVisual CarDemoApp::buildRedVisual(const racer::Car &car, float t)
{
    racer::CarVisual vis{};
    vis.steer = 0.8f;
    vis.braking = true;
    vis.wheelSpin = t * (car.speed() / racer::kWheelRadius) * 0.25f;
    return vis;
}

racer::CarVisual CarDemoApp::buildBlueVisual(const racer::Car &car, float t)
{
    racer::CarVisual vis{};
    vis.nitro = true;
    vis.headlights = true;
    vis.wheelSpin = t * (car.speed() / racer::kWheelRadius);
    return vis;
}

racer::CarVisual CarDemoApp::buildGreenVisual(const racer::Car &car, float t)
{
    racer::CarVisual vis{};
    vis.drifting = true;
    vis.steer = -0.6f;
    vis.wheelSpin = t * (car.speed() / racer::kWheelRadius) * 0.6f;
    return vis;
}

void CarDemoApp::drawCars(const racer::Car &red, const racer::CarVisual &redVis,
    const racer::Car &blue, const racer::CarVisual &blueVis,
    const racer::Car &green, const racer::CarVisual &greenVis)
{
    racer::CarRenderer::drawCarEx(red, redVis, Color{214, 48, 44, 255});
    racer::CarRenderer::drawCarEx(blue, blueVis, Color{38, 96, 220, 255});
    racer::CarRenderer::drawCarEx(green, greenVis, Color{40, 168, 76, 255});
}

void CarDemoApp::renderScene(RenderTexture2D &target, const Camera3D &camera,
    const racer::Car &red, const racer::CarVisual &redVis,
    const racer::Car &blue, const racer::CarVisual &blueVis,
    const racer::Car &green, const racer::CarVisual &greenVis, int frame)
{
    BeginTextureMode(target);
    Gfx::clearBackground(Color{58, 62, 70, 255});

    BeginMode3D(camera);
    Gfx::drawPlane(Vector3{0.0f, 0.0f, 0.0f}, Vector2{40.0f, 40.0f},
        Color{120, 122, 126, 255});
    Gfx::drawGrid(20, 1.0f);
    drawCars(red, redVis, blue, blueVis, green, greenVis);
    EndMode3D();

    Gfx::drawText("rouge: frein+braquage | bleue: nitro+phares | verte: drift",
        20, 20, 40, RAYWHITE);
    Gfx::drawText(TextFormat("frame %d", frame), 20, 72, 40, LIGHTGRAY);
    EndTextureMode();
}

void CarDemoApp::presentFrame(const RenderTexture2D &target)
{
    BeginDrawing();
    Gfx::clearBackground(BLACK);
    float texW = static_cast<float>(target.texture.width);
    float texH = static_cast<float>(target.texture.height);
    Gfx::drawTexturePro(target.texture,
        Rectangle{0.0f, 0.0f, texW, -texH},
        Rectangle{0.0f, 0.0f,
            static_cast<float>(GetRenderWidth()),
            static_cast<float>(GetRenderHeight())},
        Vector2{0.0f, 0.0f}, 0.0f, WHITE);
    EndDrawing();
}

void CarDemoApp::exportCapture(const RenderTexture2D &target, int index)
{
    Image img = LoadImageFromTexture(target.texture);
    ImageFlipVertical(&img);
    ImageResize(&img, kCapW, kCapH);
    ExportImage(img, TextFormat("car_demo_%d.png", index));
    UnloadImage(img);
}

int CarDemoApp::run()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(960, 540, "car_demo");
    SetTargetFPS(60);

    RenderTexture2D target = loadCaptureTarget();

    racer::Car red{};
    racer::Car blue{};
    racer::Car green{};
    initCars(red, blue, green);

    Camera3D camera{};
    initCamera(camera);

    int captured = 0;
    int frame = 0;

    while (!WindowShouldClose() && frame <= 130)
    {
        const float t = static_cast<float>(GetTime());
        updateTurntableCamera(camera, t);

        const racer::CarVisual redVis = buildRedVisual(red, t);
        const racer::CarVisual blueVis = buildBlueVisual(blue, t);
        const racer::CarVisual greenVis = buildGreenVisual(green, t);

        renderScene(target, camera, red, redVis, blue, blueVis, green, greenVis,
            frame);
        presentFrame(target);

        if (captured < 3 && frame == kCaptureFrames[captured])
        {
            exportCapture(target, captured);
            ++captured;
        }
        ++frame;
    }

    UnloadRenderTexture(target);
    CloseWindow();
    return 0;
}

} // namespace

int main()
{
    return CarDemoApp::run();
}
