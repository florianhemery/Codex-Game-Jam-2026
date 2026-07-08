/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track barrier sponsor and lamp decor
*/

#include <cmath>

#include "Render/Track/TrackDecorBuilder.hpp"
#include "Render/Track/TrackMeshBuilder.hpp"
#include "Render/Track/TrackRenderer.hpp"
#include "rlgl.h"

namespace racer {

void TrackDecorBuilder::appendBarrierRails(
    TrackMeshBuilder::MeshBuffers &barrierBuf, Vector3 pos,
    Vector3 along, Vector3 out, SurfaceStyle style)
{
    Color postColor = (style == SurfaceStyle::ABIMEE)
        ? Color{120, 80, 55, 255} : Color{150, 150, 158, 255};
    Color railColor = (style == SurfaceStyle::ABIMEE)
        ? Color{140, 100, 70, 255} : Color{210, 210, 218, 255};

    TrackMeshBuilder::appendBox(
        barrierBuf, pos, along, Vector3{0.0f, 1.0f, 0.0f}, out,
        0.08f, 0.35f, 0.08f, postColor);
    TrackMeshBuilder::appendBox(
        barrierBuf,
        Vector3{pos.x + along.x * 0.4f, 0.55f, pos.z + along.z * 0.4f},
        along, Vector3{0.0f, 1.0f, 0.0f}, out, 0.55f, 0.06f, 0.05f, railColor);
    TrackMeshBuilder::appendBox(
        barrierBuf,
        Vector3{pos.x - along.x * 0.4f, 0.75f, pos.z - along.z * 0.4f},
        along, Vector3{0.0f, 1.0f, 0.0f}, out, 0.55f, 0.06f, 0.05f, railColor);
}

Vector3 TrackDecorBuilder::barrierCornerPos(
    const Track &track, const std::vector<Vector2> &perp,
    float halfWidth, size_t i, float side)
{
    const auto &wp = track.waypoints();

    return Vector3{
        wp[i].x + perp[i].x * (halfWidth + 2.5f) * side, 0.0f,
        wp[i].y + perp[i].y * (halfWidth + 2.5f) * side,
    };
}

void TrackDecorBuilder::addBarrierAtCorner(
    TrackMeshBuilder::MeshBuffers &barrierBuf, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth,
    SurfaceStyle style, size_t i, size_t n)
{
    Vector2 d0{};
    Vector2 d1{};

    if (!isSharpCorner(track, i, n, d0, d1))
        return;
    const auto &wp = track.waypoints();
    float side = (perp[i].x * wp[i].x + perp[i].y * wp[i].y) > 0.0f
        ? 1.0f : -1.0f;
    Vector3 pos = barrierCornerPos(track, perp, halfWidth, i, side);
    Vector3 along{d0.x + d1.x, 0.0f, d0.y + d1.y};
    float alen = std::sqrt(along.x * along.x + along.z * along.z);

    if (alen > 1e-4f) {
        along.x /= alen;
        along.z /= alen;
    }
    Vector3 out{perp[i].x * side, 0.0f, perp[i].y * side};

    appendBarrierRails(barrierBuf, pos, along, out, style);
}

void TrackDecorBuilder::buildBarrierMeshes(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth, size_t n)
{
    TrackMeshBuilder::MeshBuffers barrierBuf;

    for (size_t i = 0; i < n; ++i) {
        addBarrierAtCorner(
            barrierBuf, track, perp, halfWidth, renderer.surfaceStyle_, i, n);
    }
    if (!barrierBuf.vertices.empty()) {
        renderer.barrierModel_ = LoadModelFromMesh(
            TrackMeshBuilder::meshFromBuffers(barrierBuf));
        renderer.hasBarriers_ = true;
    }
}

void TrackDecorBuilder::appendSponsorPanel(
    TrackMeshBuilder::MeshBuffers &sponsorBuf, Vector3 base,
    Vector3 face, Vector3 right, Color panelColor)
{
    TrackMeshBuilder::appendBox(
        sponsorBuf,
        Vector3{base.x + face.x * 0.12f, 1.6f, base.z + face.z * 0.12f},
        right, Vector3{0.0f, 1.0f, 0.0f}, face, 1.1f, 0.75f, 0.05f,
        panelColor);
    TrackMeshBuilder::appendBox(
        sponsorBuf, Vector3{base.x, 0.8f, base.z}, right,
        Vector3{0.0f, 1.0f, 0.0f}, face, 0.06f, 1.6f, 0.06f, DARKGRAY);
}

void TrackDecorBuilder::addSponsorAtWaypoint(
    TrackMeshBuilder::MeshBuffers &sponsorBuf,
    const Track &track, const std::vector<Vector2> &perp,
    float halfWidth, size_t i)
{
    const auto &wp = track.waypoints();
    uint32_t h = TrackMeshBuilder::hashIndex(i + 30000);
    float sideSign = (h % 2 == 0) ? 1.0f : -1.0f;
    float dist = halfWidth + 6.0f;
    Vector3 base{
        wp[i].x + perp[i].x * dist * sideSign, 0.0f,
        wp[i].y + perp[i].y * dist * sideSign,
    };
    Color panel{
        static_cast<unsigned char>(80 + h % 120),
        static_cast<unsigned char>(60 + (h >> 4) % 120),
        static_cast<unsigned char>(50 + (h >> 8) % 120),
        255,
    };
    Vector3 face{-perp[i].x * sideSign, 0.0f, -perp[i].y * sideSign};
    Vector3 right{face.z, 0.0f, -face.x};
    Color panelColor = (h % 2 == 0) ? panel : Fade(WHITE, 0.35f);

    appendSponsorPanel(sponsorBuf, base, face, right, panelColor);
}

void TrackDecorBuilder::buildSponsorMeshes(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth)
{
    const auto &wp = track.waypoints();
    TrackMeshBuilder::MeshBuffers sponsorBuf;

    for (size_t i = 0; i < wp.size(); i += 12)
        addSponsorAtWaypoint(sponsorBuf, track, perp, halfWidth, i);
    if (!sponsorBuf.vertices.empty()) {
        renderer.sponsorModel_ = LoadModelFromMesh(
            TrackMeshBuilder::meshFromBuffers(sponsorBuf));
        renderer.hasSponsors_ = true;
    }
}

void TrackDecorBuilder::setupLampTop(
    TrackLampInstance &lamp, const Vector2 &perp,
    float sideSign, bool broken)
{
    float lean = broken ? 0.2f : 0.06f;

    lamp.top = Vector3{
        lamp.base.x + perp.x * sideSign * lean, 6.5f,
        lamp.base.z + perp.y * sideSign * lean,
    };
    lamp.lit = !broken;
    lamp.headColor = lamp.lit
        ? Color{255, 240, 200, 255} : Color{70, 70, 80, 255};
}

void TrackDecorBuilder::addLampAtWaypoint(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth, size_t i)
{
    const auto &wp = track.waypoints();
    uint32_t h = TrackMeshBuilder::hashIndex(i + 40000);
    float sideSign = (static_cast<int>(i / 10) % 2 == 0) ? 1.0f : -1.0f;
    float dist = halfWidth + 5.0f;
    TrackLampInstance lamp;

    lamp.base = Vector3{
        wp[i].x + perp[i].x * dist * sideSign, 0.0f,
        wp[i].y + perp[i].y * dist * sideSign,
    };
    bool broken = renderer.surfaceStyle_ == SurfaceStyle::ABIMEE
        && (h % 3 == 0);

    setupLampTop(lamp, perp[i], sideSign, broken);
    renderer.lamps_.push_back(lamp);
}

void TrackDecorBuilder::buildLampRing(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth)
{
    const auto &wp = track.waypoints();

    for (size_t i = 0; i < wp.size(); i += 8)
        addLampAtWaypoint(renderer, track, perp, halfWidth, i);
}

void TrackDecorBuilder::initInflatableArch(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth, size_t n)
{
    const auto &wp = track.waypoints();
    size_t mid = n / 2;
    float hw = halfWidth + 0.5f;

    renderer.arch_.leftBase = Vector3{
        wp[mid].x - perp[mid].x * hw, 0.0f,
        wp[mid].y - perp[mid].y * hw,
    };
    renderer.arch_.rightBase = Vector3{
        wp[mid].x + perp[mid].x * hw, 0.0f,
        wp[mid].y + perp[mid].y * hw,
    };
    renderer.arch_.colorA = Color{220, 40, 60, 255};
    renderer.arch_.colorB = Color{40, 80, 220, 255};
    renderer.hasArch_ = true;
}

void TrackDecorBuilder::buildSceneMeshes(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth, size_t n)
{
    buildTrackMeshes(renderer, track, perp, halfWidth);
    buildGroundAndFinish(renderer, track, perp);
    initStartGantry(renderer, track, perp);
    initSkidOverlay(renderer, track, halfWidth);
    buildBarrierMeshes(renderer, track, perp, halfWidth, n);
    buildSponsorMeshes(renderer, track, perp, halfWidth);
}

void TrackDecorBuilder::buildSceneDecor(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth, size_t n)
{
    buildCloudRing(renderer);
    buildGrandstands(renderer, track, perp, halfWidth);
    constexpr int kStride = 2;
    const auto &wp = track.waypoints();

    for (size_t i = 0; i < wp.size(); i += static_cast<size_t>(kStride))
        populateWaypointDecor(renderer, track, perp, halfWidth, i);
    buildAbimeeDamage(renderer, track, perp);
    buildLampRing(renderer, track, perp, halfWidth);
    initInflatableArch(renderer, track, perp, halfWidth, n);
}

} // namespace racer

