/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Game loop runtime context and render passes
*/

#ifndef GAME_LOOP_CONTEXT_HPP_
#define GAME_LOOP_CONTEXT_HPP_

#include <memory>
#include <vector>

#include "raylib.h"

#include "App/LapTimer.hpp"
#include "Engine/Render/RenderPipeline.hpp"
#include "Race/RaceState.hpp"
#include "Render/Track/TrackRenderer.hpp"
#include "Render/VfxSystem.hpp"
#include "Track/Track.hpp"

namespace racer {
namespace app {

struct GameLoop::Context {
    const int screenWidth = 1280;
    const int screenHeight = 720;
    const std::vector<TrackDef> &presets;
    int selectedTrack = 0;
    AppState appState = AppState::MENU;
    std::unique_ptr<RaceState> race;
    std::unique_ptr<TrackRenderer> trackRenderer;
    std::unique_ptr<engine::RenderPipeline> pipeline;
    std::unique_ptr<VfxSystem> vfx;
    float steerSmoothed = 0.0f;
    float wheelSpin = 0.0f;
    LapTimerState lapTimer;
    engine::Ambiance currentAmbiance = engine::Ambiance::MIDI;
    bool confettiEmitted = false;
    float smoothedSpeedRatio = 0.0f;
    Camera3D camera{};

    explicit Context(const std::vector<TrackDef> &trackPresets);
};

struct GameLoop::OpaquePass {
    explicit OpaquePass(Context &ctx) : ctx_(ctx) {}
    void operator()() const;

private:
    Context &ctx_;
};

struct GameLoop::LitPass {
    explicit LitPass(Context &ctx) : ctx_(ctx) {}
    void operator()() const;

private:
    Context &ctx_;
};

struct GameLoop::VfxPass {
    explicit VfxPass(Context &ctx) : ctx_(ctx) {}
    void operator()() const;

private:
    Context &ctx_;
};

} // namespace app
} // namespace racer

#endif /* !GAME_LOOP_CONTEXT_HPP_ */
