#include <algorithm>
#include <cmath>
#include <memory>

#include "raylib.h"

#include "race/race_state.h"
#include "render/car_renderer.h"
#include "render/hud.h"
#include "render/track_renderer.h"
#include "track/track.h"
#include "vehicle/car.h"

namespace {

enum class AppState { Menu, Racing };

Color ColorForRacerIndex(size_t index, bool isPlayer) {
    if (isPlayer) return RED;
    constexpr Color palette[] = {BLUE, DARKGREEN, ORANGE, PURPLE};
    return palette[index % (sizeof(palette) / sizeof(palette[0]))];
}

} // namespace

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "racer -- prototype arcade");
    SetTargetFPS(60);

    const std::vector<racer::TrackDef>& presets = racer::Track::Presets();
    int selectedTrack = 0;
    AppState appState = AppState::Menu;

    std::unique_ptr<racer::RaceState> race;
    std::unique_ptr<racer::TrackRenderer> trackRenderer;
    float steerSmoothed = 0.0f;

    Camera3D camera{};
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 65.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    camera.position = {0.0f, 8.0f, -12.0f};
    camera.target = {0.0f, 0.0f, 0.0f};

    auto startRace = [&](int trackIndex) {
        race = std::make_unique<racer::RaceState>(racer::Track::Make(presets[static_cast<size_t>(trackIndex)]),
                                                    /*lapsToWin=*/3, /*aiCount=*/3);
        trackRenderer = std::make_unique<racer::TrackRenderer>(race->GetTrack());
        steerSmoothed = 0.0f;
    };

    while (!WindowShouldClose()) {
        // Plafonne dt : une fenetre mise en arriere-plan (perte de focus,
        // alt-tab) peut faire bondir GetFrameTime() a plusieurs secondes d'un
        // coup, ce qui fait exploser l'integration physique (position/heading
        // qui part en vrille en un seul appel a Update). Sans ce plafond, la
        // simulation ne resiste pas a un simple changement de fenetre active.
        float dt = std::min(GetFrameTime(), 0.1f);

        if (appState == AppState::Menu) {
            int count = static_cast<int>(presets.size());
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) selectedTrack = (selectedTrack + count - 1) % count;
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) selectedTrack = (selectedTrack + 1) % count;
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                startRace(selectedTrack);
                appState = AppState::Racing;
            }

            BeginDrawing();
            racer::DrawMenu(presets, selectedTrack, screenWidth, screenHeight);
            EndDrawing();
            continue;
        }

        // appState == Racing
        if (race->Phase() == racer::RacePhase::Finished && IsKeyPressed(KEY_M)) {
            appState = AppState::Menu;
            continue;
        }
        if (IsKeyPressed(KEY_R)) {
            startRace(selectedTrack);
        }

        racer::CarInput input;
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) input.throttle = 1.0f;
        else if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) input.throttle = -1.0f;

        // Note : avec la camera de poursuite actuelle (qui regarde le long de
        // +Z), le "droite ecran" correspond au monde -X -- input.steer=+1 doit
        // donc correspondre a la touche gauche pour tourner a droite a l'ecran.
        float steerTarget = 0.0f;
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) steerTarget = 1.0f;
        else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) steerTarget = -1.0f;
        steerSmoothed += (steerTarget - steerSmoothed) * std::min(1.0f, 8.0f * dt);
        input.steer = steerSmoothed;

        input.handbrake = IsKeyDown(KEY_SPACE);
        input.nitro = IsKeyDown(KEY_LEFT_SHIFT);

        race->Update(dt, input);

        const racer::RacerEntry& player = race->Racers()[static_cast<size_t>(race->PlayerIndex())];
        const racer::Car& playerCar = player.car;

        // Camera de poursuite, lissee, positionnee derriere et au-dessus de la voiture.
        Vector3 forward = playerCar.Forward();
        Vector3 desiredCamPos{
            playerCar.position.x - forward.x * 9.0f,
            playerCar.position.y + 4.5f,
            playerCar.position.z - forward.z * 9.0f,
        };
        Vector3 desiredTarget{
            playerCar.position.x + forward.x * 4.0f,
            playerCar.position.y + 1.0f,
            playerCar.position.z + forward.z * 4.0f,
        };
        float camLerp = std::min(1.0f, 6.0f * dt);
        camera.position.x += (desiredCamPos.x - camera.position.x) * camLerp;
        camera.position.y += (desiredCamPos.y - camera.position.y) * camLerp;
        camera.position.z += (desiredCamPos.z - camera.position.z) * camLerp;
        camera.target.x += (desiredTarget.x - camera.target.x) * camLerp;
        camera.target.y += (desiredTarget.y - camera.target.y) * camLerp;
        camera.target.z += (desiredTarget.z - camera.target.z) * camLerp;

        BeginDrawing();
        // ClearBackground reinitialise aussi le depth buffer (contrairement a
        // un simple DrawRectangle...) -- indispensable avant tout rendu 3D,
        // sinon les profondeurs de la frame precedente polluent la suivante.
        ClearBackground(SKYBLUE);

        BeginMode3D(camera);
        trackRenderer->Draw();
        const auto& racers = race->Racers();
        for (size_t i = 0; i < racers.size(); ++i) {
            DrawCar(racers[i].car, ColorForRacerIndex(i, racers[i].isPlayer));
        }
        EndMode3D();

        racer::DrawHud(*race, screenWidth, screenHeight);
        if (race->Phase() == racer::RacePhase::Finished) {
            const char* menuHint = "M : retour au menu";
            int hw = MeasureText(menuHint, 20);
            DrawText(menuHint, screenWidth / 2 - hw / 2, screenHeight / 2 + 50, 20, LIGHTGRAY);
        }
        DrawFPS(screenWidth - 90, screenHeight - 30);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
