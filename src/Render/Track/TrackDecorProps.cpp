/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track props grandstands and waypoint decor
*/

#include "Render/Track/TrackDecorBuilder.hpp"
#include "Render/Track/TrackMeshBuilder.hpp"
#include "Render/Track/TrackRenderer.hpp"
#include "rlgl.h"

namespace racer {

void TrackDecorBuilder::fillBuildingProp(
    TrackPropInstance &prop, uint32_t h)
{
    prop.type = 1;
    prop.heightScale = 1.0f + static_cast<float>(h % 100) * 0.03f;
    prop.color = Color{
        static_cast<unsigned char>(90 + h % 60),
        static_cast<unsigned char>(90 + h % 60),
        static_cast<unsigned char>(100 + h % 60),
        255,
    };
}

void TrackDecorBuilder::fillTreeProp(
    TrackPropInstance &prop, uint32_t h, bool isDeadTree)
{
    prop.type = isDeadTree ? 2 : 0;
    prop.heightScale = isDeadTree
        ? (0.7f + static_cast<float>(h % 50) * 0.004f)
        : (0.8f + static_cast<float>(h % 100) * 0.006f);
    prop.color = isDeadTree
        ? Color{92, 72, 48, 255}
        : Color{34, static_cast<unsigned char>(110 + h % 50), 34, 255};
}

void TrackDecorBuilder::addWaypointProp(
    TrackRenderer &renderer, Vector3 pos, uint32_t h, SurfaceStyle style)
{
    bool isBuilding = (h % 5 == 0) && style != SurfaceStyle::ABIMEE;
    bool isDeadTree = style == SurfaceStyle::ABIMEE && (h % 4 != 0);
    TrackPropInstance prop;

    prop.position = pos;
    if (isBuilding)
        fillBuildingProp(prop, h);
    else
        fillTreeProp(prop, h, isDeadTree);
    renderer.props_.push_back(prop);
}

void TrackDecorBuilder::addWaypointNpc(
    TrackRenderer &renderer, Vector3 pos,
    const Vector2 &perp, float sideSign, uint32_t h)
{
    TrackNpcInstance npc;

    npc.position = Vector3{
        pos.x + perp.x * 1.5f, 0.0f, pos.z + perp.y * 1.5f,
    };
    npc.heading = std::atan2(perp.x * sideSign, perp.y * sideSign);
    npc.shirtColor = Color{
        static_cast<unsigned char>(100 + h % 120),
        static_cast<unsigned char>(80 + (h >> 3) % 120),
        static_cast<unsigned char>(70 + (h >> 6) % 120),
        255,
    };
    npc.flagColor = Color{
        static_cast<unsigned char>(150 + h % 100),
        static_cast<unsigned char>(40 + (h >> 4) % 80),
        static_cast<unsigned char>(40 + (h >> 7) % 80),
        255,
    };
    npc.animPhase = static_cast<float>(h % 628) * 0.01f;
    renderer.npcs_.push_back(npc);
}

void TrackDecorBuilder::addWaypointTireStack(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth,
    float sideSign, size_t i, uint32_t h)
{
    const auto &wp = track.waypoints();
    TrackTireStackInstance stack;

    stack.position = Vector3{
        wp[i].x + perp[i].x * (halfWidth + 2.2f) * sideSign, 0.0f,
        wp[i].y + perp[i].y * (halfWidth + 2.2f) * sideSign,
    };
    stack.tiers = 2 + static_cast<int>(h % 3);
    renderer.tireStacks_.push_back(stack);
}

void TrackDecorBuilder::addWaypointPennant(
    TrackRenderer &renderer, Vector3 pos, uint32_t h)
{
    TrackPennantInstance pen;

    pen.base = Vector3{pos.x, 0.0f, pos.z};
    pen.top = Vector3{pos.x, 5.5f, pos.z};
    pen.color = Color{
        static_cast<unsigned char>(180 + h % 60),
        static_cast<unsigned char>(50 + (h >> 5) % 150),
        static_cast<unsigned char>(50 + (h >> 9) % 150),
        255,
    };
    pen.phase = static_cast<float>(h % 628) * 0.01f;
    renderer.pennants_.push_back(pen);
}

void TrackDecorBuilder::populateWaypointDecor(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth, size_t i)
{
    const auto &wp = track.waypoints();
    uint32_t h = TrackMeshBuilder::hashIndex(i);
    float sideSign = (h % 2 == 0) ? 1.0f : -1.0f;
    float extraOffset = 4.0f + static_cast<float>(h % 100) * 0.05f;
    float dist = halfWidth + extraOffset;
    Vector3 pos{
        wp[i].x + perp[i].x * dist * sideSign, 0.06f,
        wp[i].y + perp[i].y * dist * sideSign,
    };

    addWaypointProp(renderer, pos, h, renderer.surfaceStyle_);
    if (h % 5 == 0)
        addWaypointNpc(renderer, pos, perp[i], sideSign, h);
    if (h % 9 == 0)
        addWaypointTireStack(
            renderer, track, perp, halfWidth, sideSign, i, h);
    if (h % 7 == 0)
        addWaypointPennant(renderer, pos, h);
}

TrackGrandstandInstance TrackDecorBuilder::makeGrandstandInstance(
    const Track &track, float halfWidth, size_t mid,
    Vector2 along, Vector2 outward, float alongLen)
{
    const auto &wp = track.waypoints();
    TrackGrandstandInstance gs;
    constexpr float kInnerClearance = 5.0f;

    gs.origin = Vector3{
        wp[mid].x + outward.x * (halfWidth + kInnerClearance), 0.0f,
        wp[mid].y + outward.y * (halfWidth + kInnerClearance),
    };
    gs.along = Vector3{along.x, 0.0f, along.y};
    gs.outward = Vector3{outward.x, 0.0f, outward.y};
    gs.length = alongLen * 0.65f;
    return gs;
}

bool TrackDecorBuilder::computeStraightAlong(
    const Track &track, size_t runStart, size_t runEnd,
    Vector2 &along, size_t &mid, float &alongLen)
{
    const auto &wp = track.waypoints();

    if (runEnd <= runStart + 8)
        return false;
    mid = (runStart + runEnd) / 2;
    along = Vector2{
        wp[runEnd].x - wp[runStart].x, wp[runEnd].y - wp[runStart].y,
    };
    alongLen = std::sqrt(along.x * along.x + along.y * along.y);
    if (alongLen <= 30.0f)
        return false;
    along.x /= alongLen;
    along.y /= alongLen;
    return true;
}

void TrackDecorBuilder::tryAddGrandstand(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth,
    size_t runStart, size_t runEnd)
{
    const auto &wp = track.waypoints();
    Vector2 along{};
    size_t mid = 0;
    float alongLen = 0.0f;

    if (!computeStraightAlong(track, runStart, runEnd, along, mid, alongLen))
        return;
    Vector2 outward{perp[mid].x, perp[mid].y};
    float side = (wp[mid].x * outward.x + wp[mid].y * outward.y) > 0.0f
        ? 1.0f : -1.0f;

    outward.x *= side;
    outward.y *= side;
    TrackGrandstandInstance gs = makeGrandstandInstance(
        track, halfWidth, mid, along, outward, alongLen);

    fillGrandstandSpectators(gs, mid);
    renderer.grandstands_.push_back(gs);
}

float TrackDecorBuilder::straightRunDirectionDot(
    const Track &track, size_t i, size_t n)
{
    const auto &wp = track.waypoints();
    Vector2 d0{wp[i].x - wp[i - 1].x, wp[i].y - wp[i - 1].y};
    Vector2 d1{wp[i + 1].x - wp[i].x, wp[i + 1].y - wp[i].y};
    float l0 = std::sqrt(d0.x * d0.x + d0.y * d0.y);
    float l1 = std::sqrt(d1.x * d1.x + d1.y * d1.y);

    if (l0 <= 1e-4f || l1 <= 1e-4f)
        return -2.0f;
    d0.x /= l0;
    d0.y /= l0;
    d1.x /= l1;
    d1.y /= l1;
    return d0.x * d1.x + d0.y * d1.y;
}

bool TrackDecorBuilder::shouldEndStraightRun(
    const Track &track, size_t i, size_t n, bool &endRun)
{
    constexpr float kStraightDotThreshold = 0.995f;

    if (i + 1 >= n)
        return true;
    float dot = straightRunDirectionDot(track, i, n);

    if (dot < -1.5f) {
        endRun = true;
        return true;
    }
    if (dot >= kStraightDotThreshold)
        return false;
    endRun = true;
    return true;
}

bool TrackDecorBuilder::isGrandstandRunEnd(
    const Track &track, size_t i, size_t n)
{
    if (i == n)
        return true;
    bool dummy = false;

    if (shouldEndStraightRun(track, i, n, dummy))
        return true;
    return false;
}

void TrackDecorBuilder::buildGrandstands(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth)
{
    size_t n = track.waypoints().size();
    size_t runStart = 0;

    for (size_t i = 1; i <= n; ++i) {
        if (!isGrandstandRunEnd(track, i, n))
            continue;
        size_t runEnd = (i == n) ? n - 1 : i;

        tryAddGrandstand(renderer, track, perp, halfWidth, runStart, runEnd);
        runStart = i;
    }
}


} // namespace racer
