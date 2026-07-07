// Demo autonome du HUD et du menu : sequence automatique sans input qui
// capture trois ecrans (menu, countdown, course) puis se ferme seule.
#include <vector>

#include "raylib.h"
#include "rlgl.h"

#include "ai/ai_driver.h"
#include "race/race_state.h"
#include "render/hud.h"
#include "track/track.h"
#include "vehicle/car.h"

namespace {

constexpr int kScreenWidth = 1280;
constexpr int kScreenHeight = 720;

// Decor factice (ciel + sol + bande de piste) : donne un fond contraste pour
// juger la lisibilite du HUD sans dependre du rendu 3D.
void DrawFakeScene() {
    int horizon = static_cast<int>(kScreenHeight * 0.42f);
    DrawRectangleGradientV(0, 0, kScreenWidth, horizon, Color{96, 148, 208, 255}, Color{170, 206, 234, 255});
    DrawRectangleGradientV(0, horizon, kScreenWidth, kScreenHeight - horizon, Color{86, 138, 90, 255},
                           Color{46, 82, 54, 255});
    DrawRectangle(0, static_cast<int>(kScreenHeight * 0.56f), kScreenWidth, static_cast<int>(kScreenHeight * 0.20f),
                  Color{70, 72, 78, 255});
}

// Capture manuelle du back buffer. TakeScreenshot ne convient pas ici : il
// applique l'echelle DPI (framebuffer letterboxe) et surtout ne vide pas le
// batch rlgl en attente, donc tout ce qui est dessine en fin de frame
// manquerait a l'image.
void CaptureFrame(const char* file) {
    rlDrawRenderBatchActive();
    unsigned char* data = rlReadScreenPixels(kScreenWidth, kScreenHeight);
    Image img{data, kScreenWidth, kScreenHeight, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    ExportImage(img, file);
    MemFree(data);
}

// Dessine le meme contenu quelques frames puis capture la derniere.
template <typename DrawFn>
void RenderAndCapture(const char* file, DrawFn&& draw) {
    const int frames = 10;
    for (int i = 0; i < frames; ++i) {
        BeginDrawing();
        draw();
        if (i == frames - 1) CaptureFrame(file);
        EndDrawing();
    }
}

} // namespace

int main() {
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(kScreenWidth, kScreenHeight, "hud_demo");
    SetTargetFPS(60);

    const std::vector<racer::TrackDef>& presets = racer::Track::Presets();

    racer::HudExtras extras;
    extras.racerColors = {RED, BLUE, DARKGREEN, ORANGE};

    // (a) Menu, selection sur l'index 1.
    RenderAndCapture("hud_demo_menu.png", [&] { racer::DrawMenu(presets, 1, kScreenWidth, kScreenHeight); });

    // (b) Course fraiche avancee de ~1.25 s : toujours en countdown (chiffre 2).
    racer::RaceState race(racer::Track::Make(presets[1]), /*lapsToWin=*/3, /*aiCount=*/3);
    racer::CarInput input;
    input.throttle = 1.0f;
    for (int i = 0; i < 75; ++i) race.Update(1.0f / 60.0f, input);

    RenderAndCapture("hud_demo_countdown.png", [&] {
        DrawFakeScene();
        racer::DrawHudEx(race, kScreenWidth, kScreenHeight, extras);
    });

    // (b bis) Juste apres le depart (~0.35 s de course) : flash "GO !" + feu
    // vert. Le joueur est pilote par une IA : plein gaz sans direction il
    // quitterait la piste et son point serait plaque au bord de la minimap.
    racer::AIDriver raceDriver(1.0f);
    const racer::Car& raceCar = race.Racers()[static_cast<size_t>(race.PlayerIndex())].car;
    for (int i = 0; i < 200 - 75; ++i) {
        race.Update(1.0f / 60.0f, raceDriver.ComputeInput(raceCar, race.GetTrack()));
    }
    RenderAndCapture("hud_demo_go.png", [&] {
        DrawFakeScene();
        racer::DrawHudEx(race, kScreenWidth, kScreenHeight, extras);
    });

    // (c) ~10 s simulees au total (3 s de countdown + 7 s de course).
    for (int i = 0; i < 600 - 200; ++i) {
        race.Update(1.0f / 60.0f, raceDriver.ComputeInput(raceCar, race.GetTrack()));
    }
    extras.currentLapTime = 42.3f;
    extras.lastLapTime = 61.2f;
    extras.bestLapTime = 58.9f;

    RenderAndCapture("hud_demo_race.png", [&] {
        DrawFakeScene();
        racer::DrawHudEx(race, kScreenWidth, kScreenHeight, extras);
    });

    // (c bis) Ligne franchie il y a ~1 s : la ligne "Dernier" clignote dans le
    // panneau des chronos (currentLapTime < 3 s et lastLapTime > 0).
    extras.currentLapTime = 1.2f;
    RenderAndCapture("hud_demo_race_lastlap.png", [&] {
        DrawFakeScene();
        racer::DrawHudEx(race, kScreenWidth, kScreenHeight, extras);
    });

    // (d) Validation bonus : le joueur est pilote par une IA jusqu'a la fin de
    // la course pour verifier l'ecran d'arrivee (tableau + ecarts estimes).
    racer::RaceState finishRace(racer::Track::Make(presets[1]), /*lapsToWin=*/3, /*aiCount=*/3);
    racer::AIDriver playerDriver(1.0f);
    const racer::Car& playerCar = finishRace.Racers()[static_cast<size_t>(finishRace.PlayerIndex())].car;
    for (int i = 0; i < 60000 && finishRace.Phase() != racer::RacePhase::Finished; ++i) {
        finishRace.Update(1.0f / 60.0f, playerDriver.ComputeInput(playerCar, finishRace.GetTrack()));
    }
    // Ligne franchie a l'instant : currentLapTime < 3 s => la ligne "Dernier"
    // doit apparaitre dans le panneau des chronos (validation visuelle).
    extras.currentLapTime = 1.2f;
    extras.lastLapTime = 61.2f;
    extras.bestLapTime = 58.9f;

    RenderAndCapture("hud_demo_finish.png", [&] {
        DrawFakeScene();
        racer::DrawHudEx(finishRace, kScreenWidth, kScreenHeight, extras);
    });

    CloseWindow();
    return 0;
}
