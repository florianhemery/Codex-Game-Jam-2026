/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Standalone HUD and menu demo with automatic captures
*/

#include "Ai/AiDriver.hpp"
#include "Race/RaceState.hpp"
#include "Render/Hud.hpp"
#include "Track/Track.hpp"
#include "Vehicle/Car.hpp"

#include "raylib.h"
#include "rlgl.h"

#include <vector>

namespace {

constexpr int kScreenWidth = 1280;
constexpr int kScreenHeight = 720;

class HudDemoApp {
public:
    static int run();

private:
    struct Gfx {
        static void drawRectangleGradientV(int x, int y, int w, int h,
            Color color1, Color color2);
        static void drawRectangle(int x, int y, int w, int h, Color color);
    };

    static void drawFakeScene();
    static void captureFrame(const char *file);
    template <typename DrawFn>
    static void renderAndCapture(const char *file, DrawFn &&draw);
    static void advanceRace(racer::RaceState &race, const racer::Car &car,
        racer::AIDriver &driver, int steps);
    static void captureHudScene(racer::RaceState &race,
        const racer::HudExtras &extras, const char *file);
    static void runFinishRace(racer::RaceState &finishRace,
        const racer::Car &playerCar, racer::AIDriver &playerDriver);
};

void HudDemoApp::Gfx::drawRectangleGradientV(int x, int y, int w, int h,
    Color color1, Color color2)
{
    DrawRectangleGradientV(x, y, w, h, color1, color2);
}

void HudDemoApp::Gfx::drawRectangle(int x, int y, int w, int h, Color color)
{
    DrawRectangle(x, y, w, h, color);
}

void HudDemoApp::drawFakeScene()
{
    const int horizon = static_cast<int>(kScreenHeight * 0.42f);
    Gfx::drawRectangleGradientV(0, 0, kScreenWidth, horizon,
        Color{96, 148, 208, 255}, Color{170, 206, 234, 255});
    Gfx::drawRectangleGradientV(0, horizon, kScreenWidth,
        kScreenHeight - horizon, Color{86, 138, 90, 255},
        Color{46, 82, 54, 255});
    const int roadY = static_cast<int>(kScreenHeight * 0.56f);
    const int roadH = static_cast<int>(kScreenHeight * 0.20f);
    Gfx::drawRectangle(0, roadY, kScreenWidth, roadH,
        Color{70, 72, 78, 255});
}

void HudDemoApp::captureFrame(const char *file)
{
    rlDrawRenderBatchActive();
    unsigned char *data = rlReadScreenPixels(kScreenWidth, kScreenHeight);
    Image img{data, kScreenWidth, kScreenHeight, 1,
        PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    ExportImage(img, file);
    MemFree(data);
}

template <typename DrawFn>
void HudDemoApp::renderAndCapture(const char *file, DrawFn &&draw)
{
    constexpr int kFrames = 10;
    for (int i = 0; i < kFrames; ++i)
    {
        BeginDrawing();
        draw();
        if (i == kFrames - 1)
            captureFrame(file);
        EndDrawing();
    }
}

void HudDemoApp::advanceRace(racer::RaceState &race, const racer::Car &car,
    racer::AIDriver &driver, int steps)
{
    for (int i = 0; i < steps; ++i)
        race.update(1.0f / 60.0f, driver.computeInput(car, race.getTrack()));
}

void HudDemoApp::captureHudScene(racer::RaceState &race,
    const racer::HudExtras &extras, const char *file)
{
    renderAndCapture(file, [&] {
        drawFakeScene();
        racer::Hud hud;
        hud.drawHudEx(race, kScreenWidth, kScreenHeight, extras);
    });
}

void HudDemoApp::runFinishRace(racer::RaceState &finishRace,
    const racer::Car &playerCar, racer::AIDriver &playerDriver)
{
    for (int i = 0;
         i < 60000 && finishRace.phase() != racer::RacePhase::FINISHED;
         ++i)
    {
        racer::CarInput input = playerDriver.computeInput(
            playerCar, finishRace.getTrack());
        finishRace.update(1.0f / 60.0f, input);
    }
}

int HudDemoApp::run()
{
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(kScreenWidth, kScreenHeight, "hud_demo");
    SetTargetFPS(60);

    const std::vector<racer::TrackDef> &presets = racer::Track::presets();

    racer::HudExtras extras;
    extras.racerColors = {RED, BLUE, DARKGREEN, ORANGE};

    renderAndCapture("hud_demo_menu.png", [&] {
        racer::Hud hud;
        hud.drawMenu(presets, 1, kScreenWidth, kScreenHeight);
    });

    racer::RaceState race(racer::Track::make(presets[1]), /*lapsToWin=*/3,
        /*aiCount=*/3);
    racer::CarInput input;
    input.throttle = 1.0f;
    for (int i = 0; i < 75; ++i)
        race.update(1.0f / 60.0f, input);

    captureHudScene(race, extras, "hud_demo_countdown.png");

    racer::AIDriver raceDriver(1.0f);
    const racer::Car &raceCar =
        race.racers()[static_cast<size_t>(race.playerIndex())].car;
    advanceRace(race, raceCar, raceDriver, 200 - 75);
    captureHudScene(race, extras, "hud_demo_go.png");

    advanceRace(race, raceCar, raceDriver, 600 - 200);
    extras.currentLapTime = 42.3f;
    extras.lastLapTime = 61.2f;
    extras.bestLapTime = 58.9f;
    captureHudScene(race, extras, "hud_demo_race.png");

    extras.currentLapTime = 1.2f;
    captureHudScene(race, extras, "hud_demo_race_lastlap.png");

    racer::RaceState finishRace(racer::Track::make(presets[1]), /*lapsToWin=*/3,
        /*aiCount=*/3);
    racer::AIDriver playerDriver(1.0f);
    const racer::Car &playerCar =
        finishRace.racers()[static_cast<size_t>(finishRace.playerIndex())].car;
    runFinishRace(finishRace, playerCar, playerDriver);

    extras.currentLapTime = 1.2f;
    extras.lastLapTime = 61.2f;
    extras.bestLapTime = 58.9f;
    captureHudScene(finishRace, extras, "hud_demo_finish.png");

    CloseWindow();
    return 0;
}

} // namespace

int main()
{
    return HudDemoApp::run();
}
