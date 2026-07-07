#include <cmath>
#include <memory>
#include <string>

#include "raylib.h"

#include "render/track_renderer.h"
#include "track/track.h"

namespace {

constexpr int kWidth = 960;
constexpr int kHeight = 540;

void SimulateSkidArc(racer::TrackRenderer& renderer, const racer::Track& track) {
    const auto& wp = track.Waypoints();
    if (wp.size() < 4) return;

    float dist = 0.0f;
    constexpr float kTarget = 30.0f;
    size_t idx = 0;
    while (idx + 1 < wp.size() && dist < kTarget) {
        Vector2 a = wp[idx];
        Vector2 b = wp[idx + 1];
        float segLen = std::sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
        float step = 1.2f;
        for (float t = 0.0f; t < segLen && dist < kTarget; t += step) {
            float u = t / segLen;
            Vector3 pos{a.x + (b.x - a.x) * u, 0.05f, a.y + (b.y - a.y) * u};
            Vector3 dir{b.x - a.x, 0.0f, b.y - a.y};
            float perpX = -dir.z;
            float perpZ = dir.x;
            float plen = std::sqrt(perpX * perpX + perpZ * perpZ);
            if (plen > 1e-4f) {
                perpX /= plen;
                perpZ /= plen;
            }
            renderer.QueueSkidMark(Vector3{pos.x + perpX * 0.5f, pos.y, pos.z + perpZ * 0.5f}, dir, 0.35f, 0.85f);
            renderer.QueueSkidMark(Vector3{pos.x - perpX * 0.5f, pos.y, pos.z - perpZ * 0.5f}, dir, 0.35f, 0.85f);
            dist += step;
        }
        ++idx;
    }
    renderer.FlushSkidMarks();
}

} // namespace

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(kWidth, kHeight, "track_demo");
    SetTargetFPS(60);

    const auto& presets = racer::Track::Presets();
  Camera3D camera{};
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 55.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    int frame = 0;
    int presetIndex = 0;
    std::unique_ptr<racer::TrackRenderer> renderer;
    racer::Track track = racer::Track::Make(presets[0]);

    auto loadPreset = [&](int i) {
        presetIndex = i;
        track = racer::Track::Make(presets[static_cast<size_t>(i)]);
        renderer = std::make_unique<racer::TrackRenderer>(track, presets[static_cast<size_t>(i)]);
        SimulateSkidArc(*renderer, track);
        frame = 0;

        const auto& wp = track.Waypoints();
        Vector2 center{0.0f, 0.0f};
        for (const auto& p : wp) {
            center.x += p.x;
            center.y += p.y;
        }
        center.x /= static_cast<float>(wp.size());
        center.y /= static_cast<float>(wp.size());
        camera.position = Vector3{center.x + 40.0f, 35.0f, center.y + 40.0f};
        camera.target = Vector3{center.x, 0.0f, center.y};
    };

    loadPreset(0);

    while (!WindowShouldClose() && presetIndex < static_cast<int>(presets.size())) {
        float t = static_cast<float>(GetTime());
        BeginDrawing();
        racer::DrawSkyGradient(kWidth, kHeight);
        BeginMode3D(camera);
        renderer->Draw(t);
        EndMode3D();
        EndDrawing();

        if (frame == 20) {
            std::string name = "track_demo_" + std::to_string(presetIndex) + ".png";
            TakeScreenshot(name.c_str());
            if (presetIndex + 1 < static_cast<int>(presets.size())) {
                loadPreset(presetIndex + 1);
            } else {
                ++presetIndex;
            }
        }
        ++frame;
    }

    CloseWindow();
    return 0;
}
