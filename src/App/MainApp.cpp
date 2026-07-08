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
#include "Save/SaveSystem.hpp"
#include "Track/Track.hpp"

int MainApp::run()
{
    const std::vector<racer::TrackDef> &presets = racer::Track::presets();
    racer::app::GameLoop::Context ctx(presets);

    {
        racer::world::ProgressionState loaded;
        racer::save::SaveSystem::load(
            racer::save::SaveSystem::defaultProfileName(), loaded);
        ctx.openWorldProgressionBackup = loaded;
    }

    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(ctx.screenWidth, ctx.screenHeight, "racer");
    // Escape ouvre le menu pause en course, ne doit pas quitter le jeu.
    SetExitKey(0);
    SetWindowMinSize(640, 360);
    ctx.windowedWidth = ctx.screenWidth;
    ctx.windowedHeight = ctx.screenHeight;
    SetTargetFPS(60);
    ctx.vfx = std::make_unique<racer::VfxSystem>();
    racer::app::initCamera(ctx.camera);
    ctx.pipeline = std::make_unique<racer::engine::RenderPipeline>(
        ctx.screenWidth, ctx.screenHeight);
    ctx.audioSystem.init();
    racer::app::GameLoop::run(ctx);
    ctx.audioSystem.shutdown();

    if (ctx.aurelia) {
        racer::save::SaveSystem::save(
            racer::save::SaveSystem::defaultProfileName(),
            ctx.aurelia->progression());
    } else if (ctx.openWorldProgressionBackup) {
        racer::save::SaveSystem::save(
            racer::save::SaveSystem::defaultProfileName(),
            *ctx.openWorldProgressionBackup);
    }

    ctx.pipeline.reset();
    ctx.vfx.reset();
    CloseWindow();
    return 0;
}
