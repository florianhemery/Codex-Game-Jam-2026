/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Render demo scene drawing helpers
*/

#include "Engine/Render/RenderDemoScene.hpp"

#include <cmath>

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

namespace render_demo {

void RenderDemoScene::saveScreenshot(const char *fileName)
{
    unsigned char *pixels = rlReadScreenPixels(kWidth, kHeight);
    Image image{
        pixels, kWidth, kHeight, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    ExportImage(image, fileName);
    UnloadImage(image);
}

void RenderDemoScene::unloadDemoModel(Model &model)
{
    Shader def{};
    def.id = rlGetShaderIdDefault();
    def.locs = rlGetShaderLocsDefault();
    for (int i = 0; i < model.materialCount; ++i)
        model.materials[i].shader = def;
    UnloadModel(model);
}

void RenderDemoScene::drawColorCubes()
{
    DrawCube(
        Vector3{6.0f, 1.0f, -4.0f}, 2.0f, 2.0f, 2.0f,
        Color{230, 80, 60, 255});
    DrawCube(
        Vector3{-6.5f, 1.5f, -6.0f}, 2.2f, 3.0f, 2.2f,
        Color{250, 200, 70, 255});
    DrawCube(
        Vector3{-9.0f, 0.75f, 3.5f}, 1.5f, 1.5f, 1.5f,
        Color{90, 200, 120, 255});
    DrawCube(
        Vector3{9.5f, 2.0f, 4.0f}, 1.8f, 4.0f, 1.8f,
        Color{120, 130, 240, 255});
    DrawCube(
        Vector3{2.5f, 0.6f, -8.5f}, 1.2f, 1.2f, 1.2f,
        Color{240, 130, 200, 255});
}

void RenderDemoScene::drawPropModels(const DemoModels &models)
{
    DrawModel(
        models.sphere_, Vector3{-3.0f, 1.0f, -2.5f}, 1.0f,
        Color{235, 235, 235, 255});
    DrawModel(
        models.sphere_, Vector3{4.5f, 0.8f, 7.5f}, 0.8f,
        Color{80, 190, 220, 255});
    DrawModel(
        models.cylinder_, Vector3{-7.0f, 0.0f, 8.0f}, 1.0f,
        Color{240, 140, 60, 255});
    DrawModel(
        models.cylinder_, Vector3{7.5f, 0.0f, -8.5f}, 1.0f,
        Color{140, 220, 90, 255});
}

void RenderDemoScene::drawMobileSphere(const DemoModels &models, float t)
{
    const Vector3 mobile{
        std::cos(t * 1.2f) * 7.0f,
        1.3f + 0.5f * std::sin(t * 2.1f),
        std::sin(t * 1.2f) * 7.0f};

    DrawModel(models.sphere_, mobile, 1.1f, Color{250, 250, 120, 255});
}

void RenderDemoScene::drawCarProxy()
{
    DrawCube(
        Vector3{0.0f, 0.55f, 6.0f}, 1.8f, 0.6f, 3.4f,
        Color{205, 45, 55, 255});
    DrawCube(
        Vector3{0.0f, 1.05f, 6.3f}, 1.4f, 0.5f, 1.6f,
        Color{60, 60, 70, 255});
    DrawCube(
        Vector3{-0.85f, 0.3f, 4.9f}, 0.3f, 0.6f, 0.6f,
        Color{25, 25, 28, 255});
    DrawCube(
        Vector3{0.85f, 0.3f, 4.9f}, 0.3f, 0.6f, 0.6f,
        Color{25, 25, 28, 255});
    DrawCube(
        Vector3{-0.85f, 0.3f, 7.1f}, 0.3f, 0.6f, 0.6f,
        Color{25, 25, 28, 255});
    DrawCube(
        Vector3{0.85f, 0.3f, 7.1f}, 0.3f, 0.6f, 0.6f,
        Color{25, 25, 28, 255});
}

void RenderDemoScene::drawCasters(const DemoModels &models, float t)
{
    drawColorCubes();
    drawPropModels(models);
    drawMobileSphere(models, t);
    drawCarProxy();
}

void RenderDemoScene::drawScene(const DemoModels &models, float t)
{
    DrawModel(
        models.ground_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f,
        Color{86, 98, 74, 255});
    DrawCube(
        Vector3{0.0f, 0.02f, 0.0f}, 7.0f, 0.05f, 400.0f,
        Color{48, 48, 54, 255});
    drawCasters(models, t);
}

void RenderDemoScene::drawUnlit(
    const std::array<Vector3, 3> &lightPos,
    float pulse,
    bool headlights)
{
    BeginBlendMode(BLEND_ADDITIVE);
    DrawSphere(lightPos[0], 0.18f + 0.06f * pulse, Color{255, 60, 40, 200});
    DrawSphere(lightPos[1], 0.16f, Color{60, 220, 255, 190});
    DrawSphere(lightPos[2], 0.16f, Color{255, 190, 90, 190});
    if (headlights) {
        DrawSphere(
            Vector3{-0.65f, 0.55f, 7.75f}, 0.12f,
            Color{255, 235, 190, 220});
        DrawSphere(
            Vector3{0.65f, 0.55f, 7.75f}, 0.12f,
            Color{255, 235, 190, 220});
    }
    EndBlendMode();
}

DemoModels RenderDemoScene::loadModels()
{
    DemoModels models;

    models.ground_ = LoadModelFromMesh(GenMeshPlane(400.0f, 400.0f, 8, 8));
    models.sphere_ = LoadModelFromMesh(GenMeshSphere(1.0f, 24, 32));
    models.cylinder_ = LoadModelFromMesh(GenMeshCylinder(0.8f, 3.0f, 24));
    return models;
}

Camera3D RenderDemoScene::makeCamera()
{
    Camera3D camera{};

    camera.position = Vector3{15.5f, 4.6f, 15.5f};
    camera.target = Vector3{0.0f, 1.6f, 0.0f};
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 55.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    return camera;
}

void RenderDemoScene::assignLitShaders(DemoModels &models, const Shader &lit)
{
    models.ground_.materials[0].shader = lit;
    models.sphere_.materials[0].shader = lit;
    models.cylinder_.materials[0].shader = lit;
}

std::array<Vector3, 3> RenderDemoScene::makeLightPositions()
{
    return {
        Vector3{4.0f, 1.5f, 3.0f},
        Vector3{-5.0f, 1.3f, -2.0f},
        Vector3{-2.0f, 1.6f, 9.0f},
    };
}

void RenderDemoScene::setupPipelineLights(
    racer::engine::RenderPipeline &pipeline,
    const std::array<Vector3, 3> &lightPos,
    float pulse,
    bool headlights)
{
    pipeline.clearLights();
    pipeline.addLight(
        lightPos[0], Vector3{6.0f * pulse + 1.0f, 0.7f, 0.5f});
    pipeline.addLight(lightPos[1], Vector3{0.7f, 3.6f, 4.4f});
    pipeline.addLight(lightPos[2], Vector3{4.2f, 2.6f, 1.1f});
    if (headlights) {
        pipeline.addLight(
            Vector3{-0.65f, 0.7f, 8.6f}, Vector3{7.0f, 6.3f, 4.6f});
        pipeline.addLight(
            Vector3{0.65f, 0.7f, 8.6f}, Vector3{7.0f, 6.3f, 4.6f});
    }
}

void RenderDemoScene::renderAmbianceFrame(
    racer::engine::RenderPipeline &pipeline,
    DemoModels &models,
    const Camera3D &camera,
    const char *label,
    float t,
    float pulse,
    bool headlights)
{
    const std::array<Vector3, 3> lightPos = makeLightPositions();

    assignLitShaders(models, pipeline.litShader());
    setupPipelineLights(pipeline, lightPos, pulse, headlights);
    BeginDrawing();
    pipeline.frame(
        camera,
        [&]() { drawCasters(models, t); },
        [&]() { drawScene(models, t); },
        [&]() { drawUnlit(lightPos, pulse, headlights); },
        racer::engine::RenderPipeline::PostParams{0.0f, false});
    DrawText(label, 14, 12, 22, RAYWHITE);
    EndDrawing();
}

void RenderDemoScene::advanceAmbiance(
    racer::engine::RenderPipeline &pipeline,
    int &ambianceIndex,
    int &frame,
    const std::array<racer::engine::Ambiance, 4> &ambiances)
{
    ++frame;
    if (frame < kFramesPerAmbiance)
        return;
    saveScreenshot(TextFormat("render_demo_%d.png", ambianceIndex));
    frame = 0;
    ++ambianceIndex;
    if (ambianceIndex < 4) {
        pipeline.setAmbiance(
            ambiances[static_cast<std::size_t>(ambianceIndex)]);
    }
}

void RenderDemoScene::initDemoWindow()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(kWidth, kHeight, "render_demo");
    SetTargetFPS(60);
}

void RenderDemoScene::runAmbianceDemo()
{
    racer::engine::RenderPipeline pipeline(kWidth, kHeight);
    DemoModels models = loadModels();
    Camera3D camera = makeCamera();
    const std::array<racer::engine::Ambiance, 4> ambiances = {
        racer::engine::Ambiance::MIDI,
        racer::engine::Ambiance::AUBE_DOREE,
        racer::engine::Ambiance::CREPUSCULE,
        racer::engine::Ambiance::ORAGE};
    const std::array<const char *, 4> labels = {
        "Midi", "Aube doree", "Crepuscule", "Orage"};
    int ambianceIndex = 0;
    int frame = 0;

    pipeline.setAmbiance(ambiances[0]);
    while (!WindowShouldClose() && ambianceIndex < 4) {
        pipeline.pollShaderReload();
        const float t = static_cast<float>(GetTime());
        const float pulse = 0.5f + 0.5f * std::sin(t * 6.0f);
        const bool headlights = pipeline.params().headlights;

        renderAmbianceFrame(
            pipeline,
            models,
            camera,
            labels[static_cast<std::size_t>(ambianceIndex)],
            t,
            pulse,
            headlights);
        advanceAmbiance(pipeline, ambianceIndex, frame, ambiances);
    }
    unloadDemoModel(models.ground_);
    unloadDemoModel(models.sphere_);
    unloadDemoModel(models.cylinder_);
}

void RenderDemoScene::shutdownDemoWindow()
{
    CloseWindow();
}

} // namespace render_demo

