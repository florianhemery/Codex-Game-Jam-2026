/*

** EPITECH PROJECT, 2026

** racer

** File description:

** Game loop runtime context and render passes

*/



#ifndef GAME_LOOP_CONTEXT_HPP_

#define GAME_LOOP_CONTEXT_HPP_



#include <memory>

#include <optional>

#include <vector>



#include "raylib.h"



#include "App/AudioFeedback.hpp"

#include "App/LapTimer.hpp"

#include "World/Aurelia/AureliaWorld.hpp"
#include "World/Sim/ProgressionState.hpp"

#include "Engine/Render/RenderPipeline.hpp"

#include "Race/RaceState.hpp"

#include "Render/Track/TrackRenderer.hpp"

#include "Render/VfxSystem.hpp"

#include "Track/Track.hpp"



namespace racer {

namespace app {



struct GameLoop::Context {

    int screenWidth = 1280;

    int screenHeight = 720;

    int windowedWidth = 1280;

    int windowedHeight = 720;

    const std::vector<TrackDef> &presets;

    int selectedTrack = 0;

    AppState appState = AppState::MENU;

    MenuScreen menuScreen = MenuScreen::MAIN;

    std::unique_ptr<RaceState> race;

    std::unique_ptr<racer::world::AureliaWorld> aurelia;

    std::unique_ptr<TrackRenderer> trackRenderer;

    std::unique_ptr<engine::RenderPipeline> pipeline;

    std::unique_ptr<VfxSystem> vfx;

    float steerSmoothed = 0.0f;

    float wheelSpin = 0.0f;

    LapTimerState lapTimer;

    AudioFeedback audio;

    engine::Ambiance currentAmbiance = engine::Ambiance::MIDI;

    bool confettiEmitted = false;

    bool racePaused = false;

    bool raceFromOpenWorld = false;

    racer::world::RegionId openWorldRespawnRegion =
        racer::world::RegionId::MARINA;

    std::optional<racer::world::ProgressionState> openWorldProgressionBackup;

    bool openWorldQuickRace = false;

    bool showHowToPlay = false;

    float controlsHintTimer = 30.0f;

    int driftCount = 0;

    bool wasDrifting = false;

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



struct GameLoop::CarsPass {

    explicit CarsPass(Context &ctx) : ctx_(ctx) {}

    void operator()() const;



private:

    Context &ctx_;

};



struct GameLoop::UnlitPass {

    explicit UnlitPass(Context &ctx) : ctx_(ctx) {}

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

