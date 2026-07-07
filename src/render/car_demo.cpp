// Demo autonome du rendu voiture : 3 voitures aux etats visuels contrastes,
// camera turntable lente, captures automatiques puis fermeture (aucun input).
#include "raylib.h"

#include <cmath>

#include "render/car_renderer.h"
#include "vehicle/car.h"

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(960, 540, "car_demo");
    SetTargetFPS(60);

    // Rendu hors-ecran en 2x (1920x1080) puis reduction a 960x540 a l'export :
    // captures nettes et cadrage exact quel que soit le scaling DPI de l'OS
    // (TakeScreenshot lirait un framebuffer redimensionne par Windows).
    const int kCapW = 960, kCapH = 540;
    RenderTexture2D target = LoadRenderTexture(kCapW * 2, kCapH * 2);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

    racer::Car red{};
    red.position = Vector3{-3.2f, 0.0f, 0.0f};
    red.speed = 14.0f;

    racer::Car blue{};
    blue.position = Vector3{0.0f, 0.0f, 0.0f};
    blue.speed = 34.0f;

    racer::Car green{};
    green.position = Vector3{3.2f, 0.0f, 0.0f};
    green.heading = 0.35f;          // chassis tourne...
    green.velocityHeading = -0.15f; // ...mais trajectoire en retard : drift lisible
    green.speed = 22.0f;

    Camera3D camera{};
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 40.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    const int captureFrames[3] = {30, 75, 120};
    int captured = 0;
    int frame = 0;

    while (!WindowShouldClose() && frame <= 130) {
        float t = static_cast<float>(GetTime());

        // Turntable lente : la derniere capture montre le 3/4 arriere
        // (feux stop, feu de pluie, flammes nitro).
        float angle = 0.55f + t * 0.85f;
        camera.position = Vector3{std::sin(angle) * 10.5f, 4.2f, std::cos(angle) * 10.5f};
        camera.target = Vector3{0.0f, 0.5f, 0.0f};

        racer::CarVisual redVis{};
        redVis.steer = 0.8f;
        redVis.braking = true;
        redVis.wheelSpin = t * (red.speed / racer::kWheelRadius) * 0.25f;

        racer::CarVisual blueVis{};
        blueVis.nitro = true;
        blueVis.headlights = true;
        blueVis.wheelSpin = t * (blue.speed / racer::kWheelRadius);

        racer::CarVisual greenVis{};
        greenVis.drifting = true;
        greenVis.steer = -0.6f;
        greenVis.wheelSpin = t * (green.speed / racer::kWheelRadius) * 0.6f;

        BeginTextureMode(target);
        ClearBackground(Color{58, 62, 70, 255});

        BeginMode3D(camera);
        DrawPlane(Vector3{0.0f, 0.0f, 0.0f}, Vector2{40.0f, 40.0f}, Color{120, 122, 126, 255});
        DrawGrid(20, 1.0f);

        racer::DrawCarEx(red, redVis, Color{214, 48, 44, 255});
        racer::DrawCarEx(blue, blueVis, Color{38, 96, 220, 255});
        racer::DrawCarEx(green, greenVis, Color{40, 168, 76, 255});
        EndMode3D();

        DrawText("rouge: frein+braquage | bleue: nitro+phares | verte: drift", 20, 20, 40, RAYWHITE);
        DrawText(TextFormat("frame %d", frame), 20, 72, 40, LIGHTGRAY);
        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);
        // La texture OpenGL est verticalement inversee : hauteur source negative.
        DrawTexturePro(target.texture,
                       Rectangle{0.0f, 0.0f, static_cast<float>(target.texture.width),
                                 -static_cast<float>(target.texture.height)},
                       Rectangle{0.0f, 0.0f, static_cast<float>(GetRenderWidth()),
                                 static_cast<float>(GetRenderHeight())},
                       Vector2{0.0f, 0.0f}, 0.0f, WHITE);
        EndDrawing();

        if (captured < 3 && frame == captureFrames[captured]) {
            Image img = LoadImageFromTexture(target.texture);
            ImageFlipVertical(&img);
            ImageResize(&img, kCapW, kCapH); // reduction 2x -> lissage type supersampling
            ExportImage(img, TextFormat("car_demo_%d.png", captured));
            UnloadImage(img);
            ++captured;
        }
        ++frame;
    }

    UnloadRenderTexture(target);
    CloseWindow();
    return 0;
}
