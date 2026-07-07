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

void TrackDecorBuilder::buildTrackMeshes(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth)
{
    SurfaceStyle style = renderer.surfaceStyle_;
    Mesh trackMesh = TrackMeshBuilder::buildStripMesh(
        track, perp, -halfWidth, halfWidth, 0.02f,
        [style](size_t i) {
            return TrackMeshBuilder::asphaltColor(
                TrackMeshBuilder::hashIndex(i), style);
        });
    renderer.trackModel_ = LoadModelFromMesh(trackMesh);
    Mesh rubberMesh = TrackMeshBuilder::buildStripMesh(
        track, perp, -0.35f, 0.35f, 0.021f,
        [](size_t) { return Color{28, 28, 32, 255}; });
    renderer.rubberLineModel_ = LoadModelFromMesh(rubberMesh);
    Mesh centerDash = TrackMeshBuilder::buildDashedStripMesh(
        track, perp, -0.12f, 0.12f, 0.035f, WHITE, 4, 2);
    renderer.centerDashModel_ = LoadModelFromMesh(centerDash);
}

void TrackDecorBuilder::buildEdgeLineMeshes(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth)
{
    constexpr float kEdgeWidth = 0.18f;
    Mesh edgeOuter = TrackMeshBuilder::buildStripMesh(
        track, perp, halfWidth - kEdgeWidth, halfWidth - 0.02f, 0.034f,
        [](size_t) { return WHITE; });
    renderer.edgeLineOuterModel_ = LoadModelFromMesh(edgeOuter);
    Mesh edgeInner = TrackMeshBuilder::buildStripMesh(
        track, perp, -halfWidth + 0.02f, -halfWidth + kEdgeWidth, 0.034f,
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
        halfWidth + kCurbWidth * 0.5f, 0.025f,
        [curbA, curbB](size_t i) {
            return (static_cast<int>(i / 3) % 2 == 0) ? curbA : curbB;
        });
    renderer.curbModelOuter_ = LoadModelFromMesh(curbMeshOuter);
    Mesh curbMeshInner = TrackMeshBuilder::buildStripMesh(
        track, perp, -halfWidth - kCurbWidth * 0.5f,
        -halfWidth + kCurbWidth * 0.5f, 0.025f,
        [curbA, curbB](size_t i) {
            return (static_cast<int>(i / 3) % 2 == 0) ? curbA : curbB;
        });
    renderer.curbModelInner_ = LoadModelFromMesh(curbMeshInner);
}

void TrackDecorBuilder::buildGroundAndFinishMeshes(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp)
{
    Mesh groundMesh = TrackMeshBuilder::buildCheckerGroundMesh(
        500.0f, 40, renderer.surfaceStyle_);
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

    renderer.startGantryBase_ = Vector3{wp[0].x, 0.0f, wp[0].y};
    renderer.startGantryPerp_ = Vector3{perp[0].x, 0.0f, perp[0].y};
    renderer.startGantryAlong_ = Vector3{-perp[0].y, 0.0f, perp[0].x};
}

void TrackDecorBuilder::buildCloudPuffs(
    TrackCloudInstance &cloud, int cloudIndex)
{
    uint32_t h = TrackMeshBuilder::hashIndex(
        static_cast<size_t>(cloudIndex) + 9000);
    int puffCount = 3 + static_cast<int>(h % 3);

    for (int p = 0; p < puffCount; ++p) {
        uint32_t ph = TrackMeshBuilder::hashIndex(
            static_cast<size_t>(cloudIndex * 10 + p));
        cloud.puffOffsets.push_back(Vector3{
            static_cast<float>(ph % 100) * 0.08f - 4.0f,
            static_cast<float>((ph >> 4) % 50) * 0.02f,
            static_cast<float>((ph >> 8) % 100) * 0.08f - 4.0f,
        });
        cloud.puffScales.push_back(
            1.8f + static_cast<float>(ph % 40) * 0.04f);
    }
}

void TrackDecorBuilder::buildCloudRing(TrackRenderer &renderer)
{
    for (int c = 0; c < 14; ++c) {
        uint32_t h = TrackMeshBuilder::hashIndex(
            static_cast<size_t>(c) + 9000);
        float angle = static_cast<float>(c) / 14.0f * 2.0f * PI;
        float dist = 120.0f + static_cast<float>(h % 80);
        TrackCloudInstance cloud;

        cloud.basePosition = Vector3{
            std::cos(angle) * dist, 42.0f + static_cast<float>(h % 20),
            std::sin(angle) * dist,
        };
        cloud.driftSpeed = 0.4f + static_cast<float>(h % 50) * 0.01f;
        cloud.scale = 2.5f + static_cast<float>(h % 30) * 0.05f;
        buildCloudPuffs(cloud, c);
        renderer.clouds_.push_back(cloud);
    }
}

Color TrackDecorBuilder::makeSpectatorShirtColor(uint32_t sh)
{
    return Color{
        static_cast<unsigned char>(80 + sh % 175),
        static_cast<unsigned char>(60 + (sh >> 4) % 175),
        static_cast<unsigned char>(50 + (sh >> 8) % 175),
        255,
    };
}

Vector3 TrackDecorBuilder::makeSpectatorPosition(
    const TrackGrandstandInstance &gs,
    float rowH, float alongT, float outwardD)
{
    return Vector3{
        gs.origin.x + gs.along.x * alongT + gs.outward.x * outwardD,
        rowH,
        gs.origin.z + gs.along.z * alongT + gs.outward.z * outwardD,
    };
}

void TrackDecorBuilder::addGrandstandSpectator(
    TrackGrandstandInstance &gs, size_t mid,
    int row, int col)
{
    constexpr int kCols = 14;
    uint32_t sh = TrackMeshBuilder::hashIndex(
        static_cast<size_t>(row * kCols + col) + mid * 17);
    float alongT = (static_cast<float>(col) / (kCols - 1) - 0.5f) * gs.length;
    float outwardD = static_cast<float>(row) * 1.8f + 1.5f;
    float rowH = static_cast<float>(row) * 1.1f + 0.5f;
    TrackSpectatorInstance spec;

    spec.position = makeSpectatorPosition(gs, rowH, alongT, outwardD);
    spec.shirtColor = makeSpectatorShirtColor(sh);
    spec.jumpPhase = static_cast<float>(sh % 628) * 0.01f;
    spec.jumpSpeed = 2.5f + static_cast<float>(sh % 30) * 0.05f;
    gs.spectators.push_back(spec);
}

void TrackDecorBuilder::fillGrandstandSpectators(
    TrackGrandstandInstance &gs, size_t mid)
{
    constexpr int kRows = 4;
    constexpr int kCols = 14;

    for (int row = 0; row < kRows; ++row) {
        for (int col = 0; col < kCols; ++col)
            addGrandstandSpectator(gs, mid, row, col);
    }
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
        renderer.skidWorldOrigin_, renderer.skidWorldSize_, 0.045f);
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

