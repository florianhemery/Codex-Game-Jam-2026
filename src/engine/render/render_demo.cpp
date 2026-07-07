/// \file render_demo.cpp
/// \brief Demo autonome du pipeline : scene de test eclairee, cycle des 4
///        ambiances avec capture d'ecran automatique, fermeture sans input.

#include "engine/render/render_pipeline.h"

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <array>
#include <cmath>

namespace {

using racer::engine::Ambiance;
using racer::engine::RenderPipeline;

constexpr int kWidth = 960;
constexpr int kHeight = 540;
constexpr int kFramesPerAmbiance = 30;

struct DemoModels {
    Model ground{};
    Model sphere{};
    Model cylinder{};
};

/// Capture exacte du backbuffer 960x540 : TakeScreenshot applique l'echelle
/// DPI de l'ecran et cadrerait mal sur un affichage Windows a 150 %.
void SaveScreenshot(const char* fileName)
{
    unsigned char* pixels = rlReadScreenPixels(kWidth, kHeight);
    Image image{pixels, kWidth, kHeight, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    ExportImage(image, fileName);
    UnloadImage(image);
}

/// Neutralise le shader (possede par le pipeline) avant liberation du modele.
void UnloadDemoModel(Model& model)
{
    Shader def{};
    def.id = rlGetShaderIdDefault();
    def.locs = rlGetShaderLocsDefault();
    for (int i = 0; i < model.materialCount; ++i) model.materials[i].shader = def;
    UnloadModel(model);
}

/// Solides projetant des ombres (sans le sol, simple receveur).
void DrawCasters(const DemoModels& models, float t)
{
    // Couronne de cubes colores.
    DrawCube(Vector3{6.0f, 1.0f, -4.0f}, 2.0f, 2.0f, 2.0f, Color{230, 80, 60, 255});
    DrawCube(Vector3{-6.5f, 1.5f, -6.0f}, 2.2f, 3.0f, 2.2f, Color{250, 200, 70, 255});
    DrawCube(Vector3{-9.0f, 0.75f, 3.5f}, 1.5f, 1.5f, 1.5f, Color{90, 200, 120, 255});
    DrawCube(Vector3{9.5f, 2.0f, 4.0f}, 1.8f, 4.0f, 1.8f, Color{120, 130, 240, 255});
    DrawCube(Vector3{2.5f, 0.6f, -8.5f}, 1.2f, 1.2f, 1.2f, Color{240, 130, 200, 255});

    // Spheres et cylindres via modeles (normales propres).
    DrawModel(models.sphere, Vector3{-3.0f, 1.0f, -2.5f}, 1.0f, Color{235, 235, 235, 255});
    DrawModel(models.sphere, Vector3{4.5f, 0.8f, 7.5f}, 0.8f, Color{80, 190, 220, 255});
    DrawModel(models.cylinder, Vector3{-7.0f, 0.0f, 8.0f}, 1.0f, Color{240, 140, 60, 255});
    DrawModel(models.cylinder, Vector3{7.5f, 0.0f, -8.5f}, 1.0f, Color{140, 220, 90, 255});

    // Sphere mobile en orbite.
    const Vector3 mobile{std::cos(t*1.2f)*7.0f, 1.3f + 0.5f*std::sin(t*2.1f), std::sin(t*1.2f)*7.0f};
    DrawModel(models.sphere, mobile, 1.1f, Color{250, 250, 120, 255});

    // Proxy voiture : caisse + cabine + roues en cubes.
    DrawCube(Vector3{0.0f, 0.55f, 6.0f}, 1.8f, 0.6f, 3.4f, Color{205, 45, 55, 255});
    DrawCube(Vector3{0.0f, 1.05f, 6.3f}, 1.4f, 0.5f, 1.6f, Color{60, 60, 70, 255});
    DrawCube(Vector3{-0.85f, 0.3f, 4.9f}, 0.3f, 0.6f, 0.6f, Color{25, 25, 28, 255});
    DrawCube(Vector3{0.85f, 0.3f, 4.9f}, 0.3f, 0.6f, 0.6f, Color{25, 25, 28, 255});
    DrawCube(Vector3{-0.85f, 0.3f, 7.1f}, 0.3f, 0.6f, 0.6f, Color{25, 25, 28, 255});
    DrawCube(Vector3{0.85f, 0.3f, 7.1f}, 0.3f, 0.6f, 0.6f, Color{25, 25, 28, 255});
}

/// Scene eclairee complete : sol + bande d'asphalte + solides.
void DrawScene(const DemoModels& models, float t)
{
    DrawModel(models.ground, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, Color{86, 98, 74, 255});
    DrawCube(Vector3{0.0f, 0.02f, 0.0f}, 7.0f, 0.05f, 400.0f, Color{48, 48, 54, 255});
    DrawCasters(models, t);
}

/// Effets non eclaires : lueurs additives sur les point lights.
void DrawUnlit(const std::array<Vector3, 3>& lightPos, float pulse, bool headlights)
{
    BeginBlendMode(BLEND_ADDITIVE);
    DrawSphere(lightPos[0], 0.18f + 0.06f*pulse, Color{255, 60, 40, 200});
    DrawSphere(lightPos[1], 0.16f, Color{60, 220, 255, 190});
    DrawSphere(lightPos[2], 0.16f, Color{255, 190, 90, 190});
    if (headlights)
    {
        DrawSphere(Vector3{-0.65f, 0.55f, 7.75f}, 0.12f, Color{255, 235, 190, 220});
        DrawSphere(Vector3{0.65f, 0.55f, 7.75f}, 0.12f, Color{255, 235, 190, 220});
    }
    EndBlendMode();
}

} // namespace

int main()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(kWidth, kHeight, "render_demo");
    SetTargetFPS(60);

    {
        RenderPipeline pipeline(kWidth, kHeight);

        DemoModels models;
        models.ground = LoadModelFromMesh(GenMeshPlane(400.0f, 400.0f, 8, 8));
        models.sphere = LoadModelFromMesh(GenMeshSphere(1.0f, 24, 32));
        models.cylinder = LoadModelFromMesh(GenMeshCylinder(0.8f, 3.0f, 24));

        Camera3D camera{};
        camera.position = Vector3{15.5f, 4.6f, 15.5f};
        camera.target = Vector3{0.0f, 1.6f, 0.0f};
        camera.up = Vector3{0.0f, 1.0f, 0.0f};
        camera.fovy = 55.0f;
        camera.projection = CAMERA_PERSPECTIVE;

        const std::array<Ambiance, 4> ambiances = {
            Ambiance::Midi, Ambiance::AubeDoree, Ambiance::Crepuscule, Ambiance::Orage};
        const std::array<const char*, 4> labels = {"Midi", "Aube doree", "Crepuscule", "Orage"};

        int ambianceIndex = 0;
        int frame = 0;
        pipeline.SetAmbiance(ambiances[0]);

        while (!WindowShouldClose() && ambianceIndex < 4)
        {
            pipeline.PollShaderReload();

            // Les materiaux portent toujours le lit shader courant (hot-reload).
            const Shader lit = pipeline.LitShader();
            models.ground.materials[0].shader = lit;
            models.sphere.materials[0].shader = lit;
            models.cylinder.materials[0].shader = lit;

            const float t = static_cast<float>(GetTime());
            const float pulse = 0.5f + 0.5f*std::sin(t*6.0f);

            // Point lights de test : rouge pulsante, cyan, orange chaude.
            const std::array<Vector3, 3> lightPos = {
                Vector3{4.0f, 1.5f, 3.0f},
                Vector3{-5.0f, 1.3f, -2.0f},
                Vector3{-2.0f, 1.6f, 9.0f},
            };
            pipeline.ClearLights();
            pipeline.AddLight(lightPos[0], Vector3{6.0f*pulse + 1.0f, 0.7f, 0.5f});
            pipeline.AddLight(lightPos[1], Vector3{0.7f, 3.6f, 4.4f});
            pipeline.AddLight(lightPos[2], Vector3{4.2f, 2.6f, 1.1f});

            const bool headlights = pipeline.Params().headlights;
            if (headlights)
            {
                // Phares du proxy voiture (avant vers +z).
                pipeline.AddLight(Vector3{-0.65f, 0.7f, 8.6f}, Vector3{7.0f, 6.3f, 4.6f});
                pipeline.AddLight(Vector3{0.65f, 0.7f, 8.6f}, Vector3{7.0f, 6.3f, 4.6f});
            }

            BeginDrawing();
            pipeline.Frame(
                camera,
                [&]() { DrawCasters(models, t); },
                [&]() { DrawScene(models, t); },
                [&]() { DrawUnlit(lightPos, pulse, headlights); },
                RenderPipeline::PostParams{0.0f, false});
            DrawText(labels[static_cast<std::size_t>(ambianceIndex)], 14, 12, 22, RAYWHITE);
            EndDrawing();

            ++frame;
            if (frame >= kFramesPerAmbiance)
            {
                SaveScreenshot(TextFormat("render_demo_%d.png", ambianceIndex));
                frame = 0;
                ++ambianceIndex;
                if (ambianceIndex < 4) pipeline.SetAmbiance(ambiances[static_cast<std::size_t>(ambianceIndex)]);
            }
        }

        UnloadDemoModel(models.ground);
        UnloadDemoModel(models.sphere);
        UnloadDemoModel(models.cylinder);
    }

    CloseWindow();
    return 0;
}
