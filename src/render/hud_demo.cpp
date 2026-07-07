/// \file hud_demo.cpp
/// \brief Demo autonome du HUD et du menu : sequence automatique sans input qui
///        capture plusieurs ecrans (menu, countdown, course, arrivee) puis se
///        ferme seule.

#include "ai/ai_driver.hpp"
#include "race/race_state.hpp"
#include "render/hud.hpp"
#include "track/track.hpp"
#include "vehicle/car.hpp"

#include "raylib.h"
#include "rlgl.h"

#include <vector>

namespace {

constexpr int kScreenWidth = 1280;
constexpr int kScreenHeight = 720;

/// Decor factice (ciel + sol + bande de piste) pour juger la lisibilite du HUD.
void DrawFakeScene()
{
    const int horizon = static_cast<int>(kScreenHeight * 0.42f);
    DrawRectangleGradientV(0, 0, kScreenWidth, horizon, Color{96, 148, 208, 255},
                           Color{170, 206, 234, 255});
    DrawRectangleGradientV(0, horizon, kScreenWidth, kScreenHeight - horizon,
                           Color{86, 138, 90, 255}, Color{46, 82, 54, 255});
    DrawRectangle(0, static_cast<int>(kScreenHeight * 0.56f), kScreenWidth,
                  static_cast<int>(kScreenHeight * 0.20f), Color{70, 72, 78, 255});
}

/// Capture manuelle du back buffer (vide le batch rlgl avant lecture).
void CaptureFrame(const char* file)
{
    rlDrawRenderBatchActive();
    unsigned char* data = rlReadScreenPixels(kScreenWidth, kScreenHeight);
    Image img{data, kScreenWidth, kScreenHeight, 1,
              PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    ExportImage(img, file);
    MemFree(data);
}

/// Dessine le meme contenu quelques frames puis capture la derniere.
template <typename DrawFn>
void RenderAndCapture(const char* file, DrawFn&& draw)
{
    constexpr int kFrames = 10;
    for (int i = 0; i < kFrames; ++i)
    {
        BeginDrawing();
        draw();
        if (i == kFrames - 1)
            CaptureFrame(file);
        EndDrawing();
    }
}

void AdvanceRace(racer::RaceState& race, const racer::Car& car,
                 racer::AIDriver& driver, int steps)
{
    for (int i = 0; i < steps; ++i)
        race.Update(1.0f / 60.0f, driver.ComputeInput(car, race.GetTrack()));
}

void CaptureHudScene(racer::RaceState& race, const racer::HudExtras& extras,
                     const char* file)
{
    RenderAndCapture(file, [&] {
        DrawFakeScene();
        racer::DrawHudEx(race, kScreenWidth, kScreenHeight, extras);
    });
}

} // namespace

int main()
{
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(kScreenWidth, kScreenHeight, "hud_demo");
    SetTargetFPS(60);

    const std::vector<racer::TrackDef>& presets = racer::Track::Presets();

    racer::HudExtras extras;
    extras.racerColors = {RED, BLUE, DARKGREEN, ORANGE};

    RenderAndCapture("hud_demo_menu.png", [&] {
        racer::DrawMenu(presets, 1, kScreenWidth, kScreenHeight);
    });

    racer::RaceState race(racer::Track::Make(presets[1]), /*lapsToWin=*/3,
                          /*aiCount=*/3);
    racer::CarInput input;
    input.throttle = 1.0f;
    for (int i = 0; i < 75; ++i)
        race.Update(1.0f / 60.0f, input);

    CaptureHudScene(race, extras, "hud_demo_countdown.png");

    racer::AIDriver raceDriver(1.0f);
    const racer::Car& raceCar =
        race.Racers()[static_cast<size_t>(race.PlayerIndex())].car;
    AdvanceRace(race, raceCar, raceDriver, 200 - 75);
    CaptureHudScene(race, extras, "hud_demo_go.png");

    AdvanceRace(race, raceCar, raceDriver, 600 - 200);
    extras.currentLapTime = 42.3f;
    extras.lastLapTime = 61.2f;
    extras.bestLapTime = 58.9f;
    CaptureHudScene(race, extras, "hud_demo_race.png");

    extras.currentLapTime = 1.2f;
    CaptureHudScene(race, extras, "hud_demo_race_lastlap.png");

    racer::RaceState finishRace(racer::Track::Make(presets[1]), /*lapsToWin=*/3,
                                /*aiCount=*/3);
    racer::AIDriver playerDriver(1.0f);
    const racer::Car& playerCar =
        finishRace.Racers()[static_cast<size_t>(finishRace.PlayerIndex())].car;
    for (int i = 0; i < 60000 && finishRace.Phase() != racer::RacePhase::Finished;
         ++i)
    {
        finishRace.Update(1.0f / 60.0f,
                          playerDriver.ComputeInput(playerCar, finishRace.GetTrack()));
    }

    extras.currentLapTime = 1.2f;
    extras.lastLapTime = 61.2f;
    extras.bestLapTime = 58.9f;
    CaptureHudScene(finishRace, extras, "hud_demo_finish.png");

    CloseWindow();
    return 0;
}
