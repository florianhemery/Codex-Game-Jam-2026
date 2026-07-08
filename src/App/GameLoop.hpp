/*

** EPITECH PROJECT, 2026

** racer

** File description:

** Menu and race frame loop

*/



#ifndef GAME_LOOP_HPP_

#define GAME_LOOP_HPP_



#include <memory>

#include <vector>



#include "raylib.h"



#include "App/LapTimer.hpp"

#include "Race/RaceState.hpp"

#include "Render/CarVisual.hpp"

#include "Engine/Render/RenderPipeline.hpp"

#include "Render/Hud/HudExtras.hpp"

#include "Track/Track.hpp"

#include "Vehicle/Car.hpp"



namespace racer {

namespace app {

enum class AppState { MENU, OPEN_WORLD, RACING };



enum class MenuScreen { MAIN, QUICK_RACE };



class GameLoop {

public:

    struct Context;



    static void run(Context &ctx);



private:

    struct OpaquePass;

    struct LitPass;

    struct CarsPass;

    struct UnlitPass;

    struct VfxPass;



    static void startRace(Context &ctx, int trackIndex, bool fromOpenWorld = false);

    static void enterOpenWorld(Context &ctx);

    static void leaveRace(Context &ctx);

    static void pollShaders(Context &ctx);

    static void updateDisplay(Context &ctx);

    static bool handleMainMenuFrame(Context &ctx);

    static bool handleMenuFrame(Context &ctx);

    static bool handleOpenWorldFrame(Context &ctx, float dt);

    static bool handleQuickRaceOverlay(Context &ctx);

    static bool shouldReturnToMenu(Context &ctx);

    static void handleRaceRestart(Context &ctx);

    static void readPlayerInput(Context &ctx, float dt);

    static void updateWheelSpin(Context &ctx, float dt);

    static void updateDriftVfx(Context &ctx, const RacerEntry &entry,

        Vector3 vel, float speed);

    static void updateNitroVfx(Context &ctx, const RacerEntry &entry,

        Vector3 backDir, Vector3 vel);

    static void updateOffroadVfx(Context &ctx, const RacerEntry &entry,

        Vector3 vel);

    static void updateRacerVfx(Context &ctx, const RacerEntry &entry);

    static CarVisual buildCarVisual(Context &ctx, const RacerEntry &entry,

        bool headlights);

    static void updateConfetti(Context &ctx, const Car &playerCar);

    static void setupHeadlights(Context &ctx);

    static engine::RenderPipeline::PostParams buildPostParams(Context &ctx);

    static void renderWorld(Context &ctx);

    static void buildHudExtras(Context &ctx, HudExtras &extras);

    static void handlePauseInput(Context &ctx);

    static void simulateRace(Context &ctx, float dt);

    static void drawRaceFrame(Context &ctx);

    static void processRaceFrame(Context &ctx, float dt);

};



} // namespace app

} // namespace racer



#include "App/GameLoopContext.hpp"



#endif /* !GAME_LOOP_HPP_ */

