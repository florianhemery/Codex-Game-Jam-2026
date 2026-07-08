/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track rendering module
*/

#include "Render/Track/TrackDrawPass.hpp"
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

void drawOrientedBox(
    Vector3 center, Vector3 along, float alongLen,
    float height, float depth, Color color)
{
    float yaw = std::atan2(-along.z, along.x) * RAD2DEG;

    rlPushMatrix();
    rlTranslatef(center.x, center.y, center.z);
    rlRotatef(yaw, 0.0f, 1.0f, 0.0f);
    DrawCube(Vector3{0.0f, 0.0f, 0.0f}, alongLen, height, depth, color);
    DrawCubeWires(
        Vector3{0.0f, 0.0f, 0.0f}, alongLen, height, depth,
        Fade(BLACK, 0.3f));
    rlPopMatrix();
}

} // namespace

void TrackDrawPass::drawOneCloud(
    const TrackCloudInstance &cloud, float timeSeconds)
{
    float drift = timeSeconds * cloud.driftSpeed;
    Vector3 center{
        cloud.basePosition.x + std::sin(drift * 0.15f) * 8.0f,
        cloud.basePosition.y + std::sin(drift * 0.08f) * 0.6f,
        cloud.basePosition.z + std::cos(drift * 0.12f) * 8.0f,
    };

    for (size_t i = 0; i < cloud.puffOffsets.size(); ++i) {
        Vector3 off = cloud.puffOffsets[i];
        float s = cloud.puffScales[i] * cloud.scale;

        DrawSphere(
            Vector3{center.x + off.x, center.y + off.y, center.z + off.z},
            s, Color{255, 255, 255, 200});
    }
}

void TrackDrawPass::drawClouds(
    const TrackRenderer &renderer, float timeSeconds)
{
    for (const auto &cloud : renderer.clouds_)
        drawOneCloud(cloud, timeSeconds);
}

void TrackDrawPass::drawOneLamp(
    const TrackLampInstance &lamp, float timeSeconds)
{
    DrawCylinderEx(
        lamp.base, lamp.top, 0.08f, 0.06f, 6, Color{90, 90, 95, 255});
    if (lamp.lit) {
        float pulse = 0.85f + 0.15f * std::sin(timeSeconds * 5.5f
            + lamp.base.x * 0.2f + lamp.base.z * 0.17f);
        Color glow = Color{
            static_cast<unsigned char>(lamp.headColor.r * pulse),
            static_cast<unsigned char>(lamp.headColor.g * pulse),
            static_cast<unsigned char>(lamp.headColor.b * pulse),
            255,
        };

        DrawSphere(lamp.top, 0.22f, glow);
        DrawCylinder(
            Vector3{lamp.top.x, 0.02f, lamp.top.z},
            0.8f + pulse * 0.4f, 0.8f + pulse * 0.4f, 0.02f, 10,
            Fade(glow, 0.14f));
    } else {
        DrawSphere(lamp.top, 0.15f, lamp.headColor);
    }
}

void TrackDrawPass::drawLamps(
    const TrackRenderer &renderer, float timeSeconds)
{
    for (const auto &lamp : renderer.lamps_)
        drawOneLamp(lamp, timeSeconds);
}

void TrackDrawPass::drawArchSpan(const TrackRenderer &renderer)
{
    constexpr float pillarH = 5.5f;
    Vector3 leftTop{
        renderer.arch_.leftBase.x, pillarH, renderer.arch_.leftBase.z,
    };
    Vector3 rightTop{
        renderer.arch_.rightBase.x, pillarH, renderer.arch_.rightBase.z,
    };

    for (int s = 0; s < 8; ++s) {
        float t = static_cast<float>(s) / 7.0f;
        float archY = pillarH + 1.8f * std::sin(t * PI);
        Vector3 p{
            leftTop.x + (rightTop.x - leftTop.x) * t,
            archY,
            leftTop.z + (rightTop.z - leftTop.z) * t,
        };
        Color c = (s % 2 == 0) ? renderer.arch_.colorA : renderer.arch_.colorB;

        DrawCube(p, 1.4f, 0.5f, 0.5f, c);
    }
}

void TrackDrawPass::drawArch(const TrackRenderer &renderer)
{
    if (!renderer.hasArch_)
        return;
    constexpr float pillarH = 5.5f;

    DrawCylinder(
        renderer.arch_.leftBase, 0.35f, 0.35f, pillarH, 8,
        renderer.arch_.colorA);
    DrawCylinder(
        renderer.arch_.rightBase, 0.35f, 0.35f, pillarH, 8,
        renderer.arch_.colorB);
    drawArchSpan(renderer);
}

void TrackDrawPass::drawPotholes(const TrackRenderer &renderer)
{
    for (const auto &hole : renderer.potholes_) {
        DrawCylinder(
            hole.position, hole.radius, hole.radius * 0.85f, 0.04f, 12,
            Color{35, 32, 30, 255});
        DrawCylinder(
            hole.position, hole.radius * 0.6f, hole.radius * 0.5f, 0.05f,
            10, Color{22, 20, 18, 255});
    }
}

void TrackDrawPass::drawCracks(const TrackRenderer &renderer)
{
    for (const auto &crack : renderer.cracks_) {
        Vector3 a{
            crack.center.x - crack.tangent.x * crack.length * 0.5f,
            crack.center.y + 0.01f,
            crack.center.z - crack.tangent.z * crack.length * 0.5f,
        };
        Vector3 b{
            crack.center.x + crack.tangent.x * crack.length * 0.5f,
            crack.center.y + 0.01f,
            crack.center.z + crack.tangent.z * crack.length * 0.5f,
        };

        DrawCylinderEx(a, b, 0.06f, 0.06f, 4, Color{48, 44, 40, 255});
    }
}

void TrackDrawPass::drawRoadDamage(const TrackRenderer &renderer)
{
    drawPotholes(renderer);
    drawCracks(renderer);
}

void TrackDrawPass::drawAbimeeDebris(const TrackRenderer &renderer)
{
    if (renderer.surfaceStyle_ != SurfaceStyle::ABIMEE)
        return;
    for (size_t i = 0; i < renderer.potholes_.size(); i += 2) {
        const auto &h = renderer.potholes_[i];

        DrawCube(
            Vector3{h.position.x + 0.8f, 0.08f, h.position.z},
            0.5f, 0.16f, 0.5f, Color{90, 110, 45, 255});
    }
}

void TrackDrawPass::drawGantryBeam(
    Vector3 leftBase, Vector3 rightBase, Vector3 p, float hw,
    float pillarH)
{
    Color pillarColor{180, 180, 190, 255};

    DrawCube(
        Vector3{leftBase.x, pillarH * 0.5f, leftBase.z},
        0.5f, pillarH, 0.5f, pillarColor);
    DrawCube(
        Vector3{rightBase.x, pillarH * 0.5f, rightBase.z},
        0.5f, pillarH, 0.5f, pillarColor);
    Vector3 beamMid{
        (leftBase.x + rightBase.x) * 0.5f, pillarH,
        (leftBase.z + rightBase.z) * 0.5f,
    };

    DrawCube(beamMid, hw * 2.0f + 1.0f, 0.4f, 0.5f, DARKGRAY);
    DrawCube(
        Vector3{beamMid.x, pillarH + 0.6f, beamMid.z},
        hw * 1.6f, 0.5f, 0.15f, WHITE);
    drawGantryCheckerCells(beamMid, p, hw, pillarH);
}

void TrackDrawPass::drawGantryCheckerCells(
    Vector3 beamMid, Vector3 p, float hw, float pillarH)
{
    for (int c = 0; c < 6; ++c) {
        Color cell = (c % 2 == 0) ? BLACK : WHITE;
        float t = -0.5f + (static_cast<float>(c) + 0.5f) / 6.0f;

        DrawCube(
            Vector3{
                beamMid.x + p.x * t * hw * 1.4f, pillarH + 0.6f,
                beamMid.z + p.z * t * hw * 1.4f,
            },
            hw * 0.22f, 0.48f, 0.12f, cell);
    }
}

void TrackDrawPass::drawStartGantry(
    const TrackRenderer &renderer, float timeSeconds)
{
    (void)timeSeconds;
    Vector3 p = renderer.startGantryPerp_;
    float hw = renderer.trackHalfWidth_ + 1.5f;
    Vector3 leftBase{
        renderer.startGantryBase_.x - p.x * hw,
        renderer.startGantryBase_.y,
        renderer.startGantryBase_.z - p.z * hw,
    };
    Vector3 rightBase{
        renderer.startGantryBase_.x + p.x * hw,
        renderer.startGantryBase_.y,
        renderer.startGantryBase_.z + p.z * hw,
    };
    constexpr float pillarH = 7.5f;

    drawGantryBeam(leftBase, rightBase, p, hw, pillarH);
}

void TrackDrawPass::drawGrandstandRows(
    const TrackGrandstandInstance &gs)
{
    constexpr int kRows = 4;
    float stepDepth = 1.8f;
    Color seatColor{110, 110, 118, 255};

    for (int row = 0; row < kRows; ++row) {
        float outwardD = static_cast<float>(row) * stepDepth + stepDepth * 0.5f;
        Vector3 seatPos{
            gs.origin.x + gs.outward.x * outwardD,
            static_cast<float>(row) * 1.1f + 0.55f,
            gs.origin.z + gs.outward.z * outwardD,
        };

        drawOrientedBox(
            seatPos, gs.along, gs.length, 1.1f, stepDepth, seatColor);
    }
}

void TrackDrawPass::drawGrandstandRoof(
    const TrackGrandstandInstance &gs)
{
    constexpr int kRows = 4;
    float stepDepth = 1.8f;
    float outwardD = static_cast<float>(kRows) * stepDepth + stepDepth * 0.5f;
    Vector3 roofPos{
        gs.origin.x + gs.outward.x * outwardD,
        static_cast<float>(kRows) * 1.1f + 1.2f,
        gs.origin.z + gs.outward.z * outwardD,
    };

    drawOrientedBox(
        roofPos, gs.along, gs.length + 1.0f, 0.25f, stepDepth + 1.5f,
        Color{180, 50, 50, 255});
}

void TrackDrawPass::drawOneGrandstand(
    const TrackGrandstandInstance &gs)
{
    drawGrandstandRows(gs);
    drawGrandstandRoof(gs);
}

void TrackDrawPass::drawGrandstandStructure(
    const TrackRenderer &renderer)
{
    for (const auto &gs : renderer.grandstands_)
        drawOneGrandstand(gs);
}

} // namespace racer

