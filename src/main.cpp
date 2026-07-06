#include <cmath>

#include "raylib.h"

#include "race/race_state.h"
#include "render/car_renderer.h"
#include "render/hud.h"
#include "render/track_renderer.h"
#include "track/track.h"
#include "vehicle/car.h"

namespace {

racer::RaceState MakeNewRace() {
    return racer::RaceState(racer::Track::MakeStadiumTrack(), /*lapsToWin=*/3, /*aiCount=*/3);
}

Color ColorForRacerIndex(size_t index, bool isPlayer) {
    if (isPlayer) return RED;
    constexpr Color palette[] = {BLUE, DARKGREEN, ORANGE, PURPLE};
    return palette[index % (sizeof(palette) / sizeof(palette[0]))];
}

} // namespace

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "racer -- prototype arcade (jour 1)");
    SetTargetFPS(60);

    racer::RaceState race = MakeNewRace();
    racer::TrackRenderer trackRenderer(race.GetTrack());

    Camera3D camera{};
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 65.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    camera.position = {0.0f, 8.0f, -12.0f};
    camera.target = {0.0f, 0.0f, 0.0f};

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (IsKeyPressed(KEY_R)) {
            race = MakeNewRace();
        }

        racer::CarInput input;
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) input.throttle = 1.0f;
        else if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) input.throttle = -1.0f;

        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) input.steer = -1.0f;
        else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) input.steer = 1.0f;

        input.handbrake = IsKeyDown(KEY_SPACE);
        input.nitro = IsKeyDown(KEY_LEFT_SHIFT);

        race.Update(dt, input);

        const racer::RacerEntry& player = race.Racers()[static_cast<size_t>(race.PlayerIndex())];
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
        ClearBackground(SKYBLUE);

        BeginMode3D(camera);
        trackRenderer.Draw();
        const auto& racers = race.Racers();
        for (size_t i = 0; i < racers.size(); ++i) {
            DrawCar(racers[i].car, ColorForRacerIndex(i, racers[i].isPlayer));
        }
        EndMode3D();

        DrawHud(race, screenWidth, screenHeight);
        DrawFPS(screenWidth - 90, screenHeight - 30);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
