/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track skid mark overlay
*/

#include "Render/Track/TrackSkidMarks.hpp"

#include <algorithm>
#include <cmath>

#include "Render/Track/TrackInstanceTypes.hpp"
#include "Render/Track/TrackMeshBuilder.hpp"
#include "Render/Track/TrackRenderer.hpp"

namespace racer {

namespace {
constexpr int kSkidTextureSize = 2048;
} // namespace

void TrackSkidMarks::drawOne(
    const TrackSkidMarkCmd &cmd, Vector2 origin, float worldSize)
{
    Vector2 center = TrackMeshBuilder::worldToSkidTex(
        cmd.position.x, cmd.position.z, origin, worldSize);
    float lenPx = 1.2f / worldSize * static_cast<float>(kSkidTextureSize);
    float wPx = cmd.width / worldSize * static_cast<float>(kSkidTextureSize);
    float angle = std::atan2(cmd.direction.x, cmd.direction.z) * RAD2DEG;
    Rectangle rect{
        center.x - lenPx * 0.5f, center.y - wPx * 0.5f, lenPx, wPx,
    };

    DrawRectanglePro(
        rect, Vector2{lenPx * 0.5f, wPx * 0.5f}, angle,
        Color{
            0, 0, 0,
            static_cast<unsigned char>(cmd.strength * 76.0f),
        });
}

void TrackSkidMarks::queue(
    TrackRenderer &renderer, Vector3 pos, Vector3 dir,
    float width, float strength)
{
    float len = std::sqrt(dir.x * dir.x + dir.z * dir.z);

    if (len < 1e-4f)
        return;
    TrackSkidMarkCmd cmd;

    cmd.position = pos;
    cmd.direction = Vector3{dir.x / len, 0.0f, dir.z / len};
    cmd.width = width;
    cmd.strength = std::clamp(strength, 0.0f, 1.0f);
    renderer.skidQueue_.push_back(cmd);
}

void TrackSkidMarks::flush(TrackRenderer &renderer)
{
    if (renderer.skidQueue_.empty() || renderer.skidTexture_.id == 0)
        return;
    BeginTextureMode(renderer.skidTexture_);

    for (const auto &cmd : renderer.skidQueue_)
        TrackSkidMarks::drawOne(
            cmd, renderer.skidWorldOrigin_, renderer.skidWorldSize_);
    EndTextureMode();
    renderer.skidQueue_.clear();
}

} // namespace racer
