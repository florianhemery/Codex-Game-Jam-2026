/// \file track_demo.cpp
/// \brief Demo autonome du rendu piste : cycle des presets avec captures
///        automatiques puis fermeture sans input.

#include "Render/TrackRenderer.hpp"
#include "Track/Track.hpp"

#include "raylib.h"

#include <cmath>
#include <memory>
#include <string>
#include <vector>

namespace {

constexpr int kWidth = 960;
constexpr int kHeight = 540;

void QueueSkidPair(racer::TrackRenderer& renderer, const Vector3& pos,
                   const Vector3& dir, float perpX, float perpZ)
{
    renderer.queueSkidMark(Vector3{pos.x + perpX * 0.5f, pos.y, pos.z + perpZ * 0.5f},
                           dir, 0.35f, 0.85f);
    renderer.queueSkidMark(Vector3{pos.x - perpX * 0.5f, pos.y, pos.z - perpZ * 0.5f},
                           dir, 0.35f, 0.85f);
}

void SimulateSkidArc(racer::TrackRenderer& renderer, const racer::Track& track)
{
    const auto& wp = track.waypoints();
    if (wp.size() < 4)
        return;

    float dist = 0.0f;
    constexpr float kTarget = 30.0f;
    size_t idx = 0;
    while (idx + 1 < wp.size() && dist < kTarget)
    {
        const Vector2 a = wp[idx];
        const Vector2 b = wp[idx + 1];
        const float dx = b.x - a.x;
        const float dy = b.y - a.y;
        const float segLen = std::sqrt(dx * dx + dy * dy);
        constexpr float kStep = 1.2f;
        for (float t = 0.0f; t < segLen && dist < kTarget; t += kStep)
        {
            const float u = t / segLen;
            const Vector3 pos{a.x + dx * u, 0.05f, a.y + dy * u};
            Vector3 dir{dx, 0.0f, dy};
            float perpX = -dir.z;
            float perpZ = dir.x;
            const float plen = std::sqrt(perpX * perpX + perpZ * perpZ);
            if (plen > 1e-4f)
            {
                perpX /= plen;
                perpZ /= plen;
            }
            QueueSkidPair(renderer, pos, dir, perpX, perpZ);
            dist += kStep;
        }
        ++idx;
    }
    renderer.flushSkidMarks();
}

void CenterCameraOnTrack(Camera3D& camera, const racer::Track& track)
{
    const auto& wp = track.waypoints();
    Vector2 center{0.0f, 0.0f};
    for (const auto& p : wp)
    {
        center.x += p.x;
        center.y += p.y;
    }
    center.x /= static_cast<float>(wp.size());
    center.y /= static_cast<float>(wp.size());
    camera.position = Vector3{center.x + 40.0f, 35.0f, center.y + 40.0f};
    camera.target = Vector3{center.x, 0.0f, center.y};
}

void LoadPreset(int index, const std::vector<racer::TrackDef>& presets, racer::Track& track,
                std::unique_ptr<racer::TrackRenderer>& renderer, Camera3D& camera,
                int& frame, int& presetIndex)
{
    presetIndex = index;
    track = racer::Track::make(presets[static_cast<size_t>(index)]);
    renderer = std::make_unique<racer::TrackRenderer>(
        track, presets[static_cast<size_t>(index)]);
    SimulateSkidArc(*renderer, track);
    frame = 0;
    CenterCameraOnTrack(camera, track);
}

} // namespace

int main()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(kWidth, kHeight, "track_demo");
    SetTargetFPS(60);

    const auto& presets = racer::Track::presets();

    Camera3D camera{};
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 55.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    int frame = 0;
    int presetIndex = 0;
    std::unique_ptr<racer::TrackRenderer> renderer;
    racer::Track track = racer::Track::make(presets[0]);

    LoadPreset(0, presets, track, renderer, camera, frame, presetIndex);

    const int presetCount = static_cast<int>(presets.size());
    while (!WindowShouldClose() && presetIndex < presetCount)
    {
        const float t = static_cast<float>(GetTime());
        BeginDrawing();
        racer::drawSkyGradient(kWidth, kHeight);
        BeginMode3D(camera);
        renderer->draw(t);
        EndMode3D();
        EndDrawing();

        if (frame == 20)
        {
            const std::string name =
                "track_demo_" + std::to_string(presetIndex) + ".png";
            TakeScreenshot(name.c_str());
            if (presetIndex + 1 < presetCount)
            {
                LoadPreset(presetIndex + 1, presets, track, renderer, camera, frame,
                            presetIndex);
            }
            else
            {
                ++presetIndex;
            }
        }
        ++frame;
    }

    CloseWindow();
    return 0;
}
