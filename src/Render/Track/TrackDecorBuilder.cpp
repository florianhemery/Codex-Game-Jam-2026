/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track rendering module
*/

#include "Render/Track/TrackDecorBuilder.hpp"
#include "Render/Track/TrackMeshBuilder.hpp"
#include "Render/Track/TrackRenderer.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <vector>

#include "rlgl.h"

namespace racer {

namespace {
constexpr int kSkidTextureSize = 2048;
} // namespace

namespace {

constexpr float kGroundDrawY = -0.10f;
constexpr float kTrackSurfaceY = 0.06f;

} // namespace

void TrackDecorBuilder::buildTrackMeshes(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth)
{
    SurfaceStyle style = renderer.surfaceStyle_;
    Mesh trackMesh = TrackMeshBuilder::buildStripMesh(
        track, perp, -halfWidth, halfWidth, kTrackSurfaceY,
        [style](size_t i) {
            return TrackMeshBuilder::asphaltColor(
                TrackMeshBuilder::hashIndex(i), style);
        });
    renderer.trackModel_ = LoadModelFromMesh(trackMesh);
    Mesh rubberMesh = TrackMeshBuilder::buildStripMesh(
        track, perp, -0.35f, 0.35f, kTrackSurfaceY + 0.008f,
        [](size_t) { return Color{52, 52, 58, 255}; });
    renderer.rubberLineModel_ = LoadModelFromMesh(rubberMesh);
    Mesh centerDash = TrackMeshBuilder::buildDashedStripMesh(
        track, perp, -0.12f, 0.12f, kTrackSurfaceY + 0.022f,
        WHITE, 4, 2);
    renderer.centerDashModel_ = LoadModelFromMesh(centerDash);
}

void TrackDecorBuilder::buildEdgeLineMeshes(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth)
{
    constexpr float kEdgeWidth = 0.18f;
    Mesh edgeOuter = TrackMeshBuilder::buildStripMesh(
        track, perp, halfWidth - kEdgeWidth, halfWidth - 0.02f,
        kTrackSurfaceY + 0.014f,
        [](size_t) { return WHITE; });
    renderer.edgeLineOuterModel_ = LoadModelFromMesh(edgeOuter);
    Mesh edgeInner = TrackMeshBuilder::buildStripMesh(
        track, perp, -halfWidth + 0.02f, -halfWidth + kEdgeWidth,
        kTrackSurfaceY + 0.014f,
        [](size_t) { return WHITE; });
    renderer.edgeLineInnerModel_ = LoadModelFromMesh(edgeInner);
}

void TrackDecorBuilder::buildCurbMeshes(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth)
{
    constexpr float kCurbWidth = 1.4f;
    Color curbA = (renderer.surfaceStyle_ == SurfaceStyle::ABIMEE)
        ? Color{180, 170, 150, 255} : RED;
    Color curbB = (renderer.surfaceStyle_ == SurfaceStyle::ABIMEE)
        ? Color{160, 150, 130, 255} : RAYWHITE;
    Mesh curbMeshOuter = TrackMeshBuilder::buildStripMesh(
        track, perp, halfWidth - kCurbWidth * 0.5f,
        halfWidth + kCurbWidth * 0.5f, kTrackSurfaceY + 0.004f,
        [curbA, curbB](size_t i) {
            return (static_cast<int>(i / 3) % 2 == 0) ? curbA : curbB;
        });
    renderer.curbModelOuter_ = LoadModelFromMesh(curbMeshOuter);
    Mesh curbMeshInner = TrackMeshBuilder::buildStripMesh(
        track, perp, -halfWidth - kCurbWidth * 0.5f,
        -halfWidth + kCurbWidth * 0.5f, kTrackSurfaceY + 0.004f,
        [curbA, curbB](size_t i) {
            return (static_cast<int>(i / 3) % 2 == 0) ? curbA : curbB;
        });
    renderer.curbModelInner_ = LoadModelFromMesh(curbMeshInner);
}

void TrackDecorBuilder::buildGroundAndFinishMeshes(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp)
{
    float minX = 0.0f;
    float maxX = 0.0f;
    float minZ = 0.0f;
    float maxZ = 0.0f;

    scanWaypointBounds(track, minX, maxX, minZ, maxZ);
    float span = std::max(maxX - minX, maxZ - minZ) + 80.0f;
    renderer.groundSpan_ = span;
    renderer.groundCenter_ = Vector3{
        (minX + maxX) * 0.5f, 0.0f, (minZ + maxZ) * 0.5f,
    };
    int tilesPerSide = (renderer.surfaceStyle_ == SurfaceStyle::ABIMEE) ? 16 : 8;
    Mesh groundMesh = TrackMeshBuilder::buildCheckerGroundMesh(
        span, tilesPerSide, renderer.surfaceStyle_);
    renderer.groundModel_ = LoadModelFromMesh(groundMesh);
    Mesh finishMesh = TrackMeshBuilder::buildFinishLineMesh(track, perp);
    renderer.finishLineModel_ = LoadModelFromMesh(finishMesh);
}

void TrackDecorBuilder::buildGroundAndFinish(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp)
{
    float halfWidth = renderer.trackHalfWidth_;

    buildEdgeLineMeshes(renderer, track, perp, halfWidth);
    buildCurbMeshes(renderer, track, perp, halfWidth);
    buildGroundAndFinishMeshes(renderer, track, perp);
}

void TrackDecorBuilder::initStartGantry(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp)
{
    const auto &wp = track.waypoints();

    renderer.startGantryBase_ = Vector3{
        wp[0].x, kTrackSurfaceY, wp[0].y,
    };
    renderer.startGantryPerp_ = Vector3{perp[0].x, 0.0f, perp[0].y};
    renderer.startGantryAlong_ = Vector3{-perp[0].y, 0.0f, perp[0].x};
}


void TrackDecorBuilder::addAbimeeCrackAt(
    TrackRenderer &renderer, const TrackPotholeInstance &hole,
    const std::vector<Vector2> &perp, size_t i, uint32_t h)
{
    TrackCrackInstance crack;

    crack.center = hole.position;
    crack.tangent = Vector3{perp[i].y, 0.0f, -perp[i].x};
    crack.length = 1.2f + static_cast<float>(h % 30) * 0.05f;
    renderer.cracks_.push_back(crack);
}

void TrackDecorBuilder::addAbimeePotholeAt(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, size_t i, uint32_t h)
{
    const auto &wp = track.waypoints();
    float t = static_cast<float>(h % 1000) / 1000.0f;
    size_t j = (i + 1) % wp.size();
    Vector2 p2{
        wp[i].x + (wp[j].x - wp[i].x) * t,
        wp[i].y + (wp[j].y - wp[i].y) * t,
    };
    float lateral = (static_cast<float>((h >> 10) % 100) / 100.0f - 0.5f)
        * track.width() * 0.7f;
    TrackPotholeInstance hole;

    hole.position = Vector3{
        p2.x + perp[i].x * lateral, 0.025f,
        p2.y + perp[i].y * lateral,
    };
    hole.radius = 0.35f + static_cast<float>(h % 40) * 0.015f;
    renderer.potholes_.push_back(hole);
    if (h % 5 == 0)
        addAbimeeCrackAt(renderer, hole, perp, i, h);
}

void TrackDecorBuilder::buildAbimeeDamage(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp)
{
    const auto &wp = track.waypoints();

    if (renderer.surfaceStyle_ != SurfaceStyle::ABIMEE)
        return;
    for (size_t i = 0; i < wp.size(); i += 2) {
        uint32_t h = TrackMeshBuilder::hashIndex(i + 50000);

        if (h % 3 != 0)
            continue;
        addAbimeePotholeAt(renderer, track, perp, i, h);
    }
}

void TrackDecorBuilder::scanWaypointBounds(
    const Track &track, float &minX, float &maxX,
    float &minZ, float &maxZ)
{
    const auto &wp = track.waypoints();

    minX = wp[0].x;
    maxX = wp[0].x;
    minZ = wp[0].y;
    maxZ = wp[0].y;
    for (const auto &p : wp) {
        minX = std::min(minX, p.x);
        maxX = std::max(maxX, p.x);
        minZ = std::min(minZ, p.y);
        maxZ = std::max(maxZ, p.y);
    }
}

void TrackDecorBuilder::computeSkidBounds(
    TrackRenderer &renderer, const Track &track, float halfWidth)
{
    float minX = 0.0f;
    float maxX = 0.0f;
    float minZ = 0.0f;
    float maxZ = 0.0f;

    scanWaypointBounds(track, minX, maxX, minZ, maxZ);
    constexpr float kSkidMargin = 8.0f;

    minX -= halfWidth + kSkidMargin;
    maxX += halfWidth + kSkidMargin;
    minZ -= halfWidth + kSkidMargin;
    maxZ += halfWidth + kSkidMargin;
    renderer.skidWorldSize_ = std::max(maxX - minX, maxZ - minZ);
    renderer.skidWorldOrigin_ = Vector2{
        (minX + maxX - renderer.skidWorldSize_) * 0.5f,
        (minZ + maxZ - renderer.skidWorldSize_) * 0.5f,
    };
}

void TrackDecorBuilder::setupSkidTextureModel(TrackRenderer &renderer)
{
    renderer.skidTexture_ = LoadRenderTexture(
        kSkidTextureSize, kSkidTextureSize);
    BeginTextureMode(renderer.skidTexture_);
    ClearBackground(BLANK);
    EndTextureMode();
    Mesh skidQuad = TrackMeshBuilder::buildSkidQuadMesh(
        renderer.skidWorldOrigin_, renderer.skidWorldSize_,
        kTrackSurfaceY + 0.03f);
    renderer.skidOverlayModel_ = LoadModelFromMesh(skidQuad);
    if (renderer.skidOverlayModel_.materialCount > 0) {
        renderer.skidOverlayModel_.materials[0]
            .maps[MATERIAL_MAP_DIFFUSE].texture = renderer.skidTexture_.texture;
    }
}

void TrackDecorBuilder::initSkidOverlay(
    TrackRenderer &renderer, const Track &track, float halfWidth)
{
    computeSkidBounds(renderer, track, halfWidth);
    setupSkidTextureModel(renderer);
}

bool TrackDecorBuilder::isSharpCorner(
    const Track &track, size_t i, size_t n, Vector2 &d0, Vector2 &d1)
{
    const auto &wp = track.waypoints();
    size_t prev = (i + n - 1) % n;
    size_t next = (i + 1) % n;
    constexpr float kCurveDot = 0.88f;

    d0 = Vector2{wp[i].x - wp[prev].x, wp[i].y - wp[prev].y};
    d1 = Vector2{wp[next].x - wp[i].x, wp[next].y - wp[i].y};
    float l0 = std::sqrt(d0.x * d0.x + d0.y * d0.y);
    float l1 = std::sqrt(d1.x * d1.x + d1.y * d1.y);

    if (l0 < 1e-4f || l1 < 1e-4f)
        return false;
    d0.x /= l0;
    d0.y /= l0;
    d1.x /= l1;
    d1.y /= l1;
    return d0.x * d1.x + d0.y * d1.y < kCurveDot;
}

} // namespace racer

