/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Application entry facade
*/

#include <memory>
#include <vector>

#include "raylib.h"

#include "App/CameraController.hpp"
#include "App/GameLoop.hpp"
#include "App/MainApp.hpp"
#include "Engine/Render/RenderPipeline.hpp"
#include "Track/Track.hpp"

int MainApp::run()
{
    const std::vector<racer::TrackDef> &presets = racer::Track::presets();
    racer::app::GameLoop::Context ctx(presets);

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(ctx.screenWidth, ctx.screenHeight, "racer");
    SetTargetFPS(60);
    racer::app::initCamera(ctx.camera);
    ctx.pipeline = std::make_unique<racer::engine::RenderPipeline>(
        ctx.screenWidth, ctx.screenHeight);
    racer::app::GameLoop::run(ctx);
    ctx.pipeline.reset();
    CloseWindow();
    return 0;
}
