#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

#include "raylib.h"

#include "engine/render/render_pipeline.h"
#include "race/race_state.h"
#include "render/car_renderer.h"
#include "render/hud.h"
#include "render/track_renderer.h"
#include "render/vfx.h"
#include "track/track.h"
#include "vehicle/car.h"

namespace {

enum class AppState { Menu, Racing };

using racer::engine::Ambiance;
using racer::engine::RenderPipeline;

Color ColorForRacerIndex(size_t index, bool isPlayer) {
    if (isPlayer) return RED;
    constexpr Color palette[] = {BLUE, DARKGREEN, ORANGE, PURPLE};
    return palette[index % (sizeof(palette) / sizeof(palette[0]))];
}

Ambiance AmbianceForTrack(int trackIndex, const racer::TrackDef& def) {
    if (def.surfaceStyle == racer::SurfaceStyle::Abimee) return Ambiance::Orage;
    switch (trackIndex % 3) {
    case 0: return Ambiance::Midi;
    case 1: return Ambiance::AubeDoree;
    default: return Ambiance::Crepuscule;
    }
}

struct LapTimerState {
    float currentLapTime = 0.0f;
    float lastLapTime = 0.0f;
    float bestLapTime = 0.0f;
    int lastLapCount = 0;
    float lastLapFlash = 0.0f;
};

void UpdateLapTimer(LapTimerState& timer, const racer::RacerEntry& player, float dt, racer::RacePhase phase) {
    if (phase != racer::RacePhase::Racing) return;
    timer.currentLapTime += dt;
    if (player.lap > timer.lastLapCount) {
        timer.lastLapTime = timer.currentLapTime;
        timer.currentLapTime = 0.0f;
        if (timer.bestLapTime <= 0.0f || timer.lastLapTime < timer.bestLapTime) {
            timer.bestLapTime = timer.lastLapTime;
        }
        timer.lastLapFlash = 3.0f;
        timer.lastLapCount = player.lap;
    }
    timer.lastLapFlash = std::max(0.0f, timer.lastLapFlash - dt);
}

} // namespace

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 720;
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "racer");
    SetTargetFPS(60);

    const std::vector<racer::TrackDef>& presets = racer::Track::Presets();
    int selectedTrack = 0;
    AppState appState = AppState::Menu;

    std::unique_ptr<racer::RaceState> race;
    std::unique_ptr<racer::TrackRenderer> trackRenderer;
    std::unique_ptr<RenderPipeline> pipeline;
    racer::VfxSystem vfx;
    float steerSmoothed = 0.0f;
    float wheelSpin = 0.0f;
    LapTimerState lapTimer;
    Ambiance currentAmbiance = Ambiance::Midi;
    bool confettiEmitted = false;

    Camera3D camera{};
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 65.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    camera.position = {0.0f, 8.0f, -12.0f};
    camera.target = {0.0f, 0.0f, 0.0f};

    auto startRace = [&](int trackIndex) {
        race = std::make_unique<racer::RaceState>(racer::Track::Make(presets[static_cast<size_t>(trackIndex)]),
                                                    /*lapsToWin=*/3, /*aiCount=*/3);
        currentAmbiance = AmbianceForTrack(trackIndex, presets[static_cast<size_t>(trackIndex)]);
        if (pipeline) {
            pipeline->SetAmbiance(currentAmbiance);
        }
        trackRenderer = std::make_unique<racer::TrackRenderer>(race->GetTrack(), presets[static_cast<size_t>(trackIndex)]);
        if (pipeline) {
            trackRenderer->ApplyShader(pipeline->LitShader());
        }
        steerSmoothed = 0.0f;
        wheelSpin = 0.0f;
        lapTimer = {};
        confettiEmitted = false;
        vfx.Clear();
        if (currentAmbiance == Ambiance::Orage) vfx.SetRain(true);
        else vfx.SetRain(false);
    };

    pipeline = std::make_unique<RenderPipeline>(screenWidth, screenHeight);

    while (!WindowShouldClose()) {
        float dt = std::min(GetFrameTime(), 0.1f);
        pipeline->PollShaderReload();
        if (trackRenderer) trackRenderer->ApplyShader(pipeline->LitShader());

        if (appState == AppState::Menu) {
            int count = static_cast<int>(presets.size());
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) selectedTrack = (selectedTrack + count - 1) % count;
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) selectedTrack = (selectedTrack + 1) % count;

            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                startRace(selectedTrack);
                appState = AppState::Racing;
            }

            BeginDrawing();
            ClearBackground(Color{20, 24, 36, 255});
            racer::DrawMenu(presets, selectedTrack, screenWidth, screenHeight);
            EndDrawing();
            continue;
        }

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
        UpdateLapTimer(lapTimer, player, dt, race->Phase());

        wheelSpin += playerCar.speed * dt / racer::kWheelRadius;

        const auto& racers = race->Racers();
        const auto& pipeParams = pipeline->Params();

        for (size_t i = 0; i < racers.size(); ++i) {
            const auto& r = racers[i];
            Vector3 vel = r.car.Velocity();
            float speed = std::sqrt(vel.x * vel.x + vel.z * vel.z);
            Vector3 backDir = speed > 0.1f ? Vector3{-vel.x / speed, 0.0f, -vel.z / speed} : r.car.Forward();

            if (r.car.isDrifting && std::fabs(r.car.speed) > 6.0f) {
                vfx.EmitDriftSmoke(Vector3{r.car.position.x, 0.12f, r.car.position.z}, vel);
                if (speed > 0.1f && trackRenderer) {
                    Vector3 markPos{r.car.position.x - vel.x / speed * 1.0f, 0.05f,
                                    r.car.position.z - vel.z / speed * 1.0f};
                    trackRenderer->QueueSkidMark(markPos, vel, 0.32f, 0.7f);
                }
            }

            if (r.lastInput.nitro && r.car.nitroRemaining > 0.0f) {
                auto lights = racer::GetCarLightPoints(r.car);
                vfx.EmitNitroFlame(lights.exhaust, backDir, vel);
            }

            if (std::fabs(r.car.surfaceDrag) > 1.5f && std::fabs(r.car.speed) > 5.0f) {
                vfx.EmitOffroadDust(Vector3{r.car.position.x, 0.1f, r.car.position.z}, vel);
            }
        }

        if (race->Phase() == racer::RacePhase::Finished && !confettiEmitted) {
            vfx.EmitConfetti(Vector3{playerCar.position.x, 3.0f, playerCar.position.z});
            confettiEmitted = true;
        }

        vfx.Update(dt, playerCar.position);

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
        if (trackRenderer) trackRenderer->FlushSkidMarks();

        pipeline->ClearLights();
        if (pipeParams.headlights) {
            for (size_t i = 0; i < racers.size(); ++i) {
                auto lp = racer::GetCarLightPoints(racers[i].car);
                pipeline->AddLight(lp.headL, Vector3{2.5f, 2.4f, 2.0f});
                pipeline->AddLight(lp.headR, Vector3{2.5f, 2.4f, 2.0f});
            }
        }

        float speedRatio = std::clamp(std::fabs(playerCar.speed) / playerCar.tuning.maxSpeed, 0.0f, 1.0f);
        RenderPipeline::PostParams post{speedRatio, player.lastInput.nitro && playerCar.nitroRemaining > 0.0f};

        pipeline->Frame(
            camera,
            [&]() {
                if (!trackRenderer) return;
                trackRenderer->DrawOpaqueGeometry();
                for (size_t i = 0; i < racers.size(); ++i) {
                    racer::DrawCarEx(racers[i].car, {}, ColorForRacerIndex(i, racers[i].isPlayer));
                }
            },
            [&]() {
                if (!trackRenderer) return;
                trackRenderer->Draw(static_cast<float>(GetTime()));
                for (size_t i = 0; i < racers.size(); ++i) {
                    const auto& r = racers[i];
                    racer::CarVisual vis;
                    vis.steer = r.isPlayer ? steerSmoothed : 0.0f;
                    vis.wheelSpin = r.isPlayer ? wheelSpin : 0.0f;
                    vis.braking = r.lastInput.throttle < -0.01f && r.car.speed > 1.0f;
                    vis.nitro = r.lastInput.nitro && r.car.nitroRemaining > 0.0f;
                    vis.headlights = pipeParams.headlights;
                    vis.drifting = r.car.isDrifting;
                    racer::DrawCarEx(r.car, vis, ColorForRacerIndex(i, racers[i].isPlayer));
                }
            },
            [&]() { vfx.Draw(camera); },
            post);

        racer::HudExtras extras;
        extras.currentLapTime = lapTimer.currentLapTime;
        extras.lastLapTime = lapTimer.lastLapFlash > 0.0f ? lapTimer.lastLapTime : 0.0f;
        extras.bestLapTime = lapTimer.bestLapTime;
        for (size_t i = 0; i < racers.size(); ++i) {
            extras.racerColors.push_back(ColorForRacerIndex(i, racers[i].isPlayer));
        }
        racer::DrawHudEx(*race, screenWidth, screenHeight, extras);

        if (race->Phase() == racer::RacePhase::Finished) {
            const char* menuHint = "M : retour au menu";
            int hw = MeasureText(menuHint, 20);
            DrawText(menuHint, screenWidth / 2 - hw / 2, screenHeight / 2 + 50, 20, LIGHTGRAY);
        }
        DrawFPS(screenWidth - 90, screenHeight - 30);
        EndDrawing();
    }

    pipeline.reset();
    CloseWindow();
    return 0;
}
