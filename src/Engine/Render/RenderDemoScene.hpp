/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Render pipeline demo scene helpers
*/

#ifndef RENDER_DEMO_SCENE_HPP_
#define RENDER_DEMO_SCENE_HPP_

#include <array>

#include "Engine/Render/RenderPipeline.hpp"
#include "raylib.h"

namespace render_demo {

constexpr int kWidth = 960;
constexpr int kHeight = 540;
constexpr int kFramesPerAmbiance = 30;

struct DemoModels {
    Model ground_{};
    Model sphere_{};
    Model cylinder_{};
};

class RenderDemoScene {
public:
    static void saveScreenshot(const char *fileName);
    static void unloadDemoModel(Model &model);
    static void drawColorCubes();
    static void drawPropModels(const DemoModels &models);
    static void drawMobileSphere(const DemoModels &models, float t);
    static void drawCarProxy();
    static void drawCasters(const DemoModels &models, float t);
    static void drawScene(const DemoModels &models, float t);
    static void drawUnlit(
        const std::array<Vector3, 3> &lightPos,
        float pulse,
        bool headlights);
    static DemoModels loadModels();
    static Camera3D makeCamera();
    static void assignLitShaders(DemoModels &models, const Shader &lit);
    static std::array<Vector3, 3> makeLightPositions();
    static void setupPipelineLights(
        racer::engine::RenderPipeline &pipeline,
        const std::array<Vector3, 3> &lightPos,
        float pulse,
        bool headlights);
    static void renderAmbianceFrame(
        racer::engine::RenderPipeline &pipeline,
        DemoModels &models,
        const Camera3D &camera,
        const char *label,
        float t,
        float pulse,
        bool headlights);
    static void advanceAmbiance(
        racer::engine::RenderPipeline &pipeline,
        int &ambianceIndex,
        int &frame,
        const std::array<racer::engine::Ambiance, 4> &ambiances);
    static void initDemoWindow();
    static void runAmbianceDemo();
    static void shutdownDemoWindow();
};

} // namespace render_demo

#endif /* !RENDER_DEMO_SCENE_HPP_ */
