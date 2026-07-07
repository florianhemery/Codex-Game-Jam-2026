/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track prop and spectator draw pass
*/

#include "Render/Track/TrackDrawPass.hpp"
#include "Render/Track/TrackMeshBuilder.hpp"
#include "Render/Track/TrackRenderer.hpp"
#include "rlgl.h"

namespace racer {

void TrackDrawPass::drawOneSpectator(
    const TrackSpectatorInstance &spec, float timeSeconds)
{
    float jump = std::max(
        0.0f, std::sin(timeSeconds * spec.jumpSpeed + spec.jumpPhase)) * 0.35f;

    if (std::sin(timeSeconds * spec.jumpSpeed + spec.jumpPhase) < 0.85f)
        jump = 0.0f;
    Vector3 pos{spec.position.x, spec.position.y + jump, spec.position.z};

    DrawCube(
        Vector3{pos.x, pos.y + 0.35f, pos.z},
        0.35f, 0.7f, 0.25f, spec.shirtColor);
    DrawSphere(
        Vector3{pos.x, pos.y + 0.85f, pos.z}, 0.18f,
        Color{240, 200, 170, 255});
}

void TrackDrawPass::drawSpectators(
    const TrackRenderer &renderer, float timeSeconds)
{
    for (const auto &gs : renderer.grandstands_) {
        for (const auto &spec : gs.spectators)
            drawOneSpectator(spec, timeSeconds);
    }
}

void TrackDrawPass::drawBuildingProp(
    const TrackPropInstance &prop)
{
    float w = 2.5f;
    float h = 5.0f * prop.heightScale;

    DrawCube(
        Vector3{prop.position.x, h * 0.5f, prop.position.z},
        w, h, w, prop.color);
    DrawCubeWires(
        Vector3{prop.position.x, h * 0.5f, prop.position.z},
        w, h, w, Fade(BLACK, 0.4f));
    for (int win = 0; win < 3; ++win) {
        DrawCube(
            Vector3{
                prop.position.x + 0.8f,
                1.5f + static_cast<float>(win) * 1.4f,
                prop.position.z + 1.26f,
            },
            0.4f, 0.5f, 0.05f, Color{200, 220, 255, 200});
    }
}

void TrackDrawPass::drawDeadTreeProp(
    const TrackPropInstance &prop)
{
    float trunkH = 1.2f * prop.heightScale;

    DrawCylinder(
        Vector3{prop.position.x, 0.0f, prop.position.z},
        0.2f, 0.25f, trunkH, 5, Color{72, 52, 36, 255});
    DrawSphere(
        Vector3{prop.position.x, trunkH + 0.3f, prop.position.z},
        0.5f * prop.heightScale, prop.color);
    DrawSphere(
        Vector3{
            prop.position.x + 0.4f, trunkH,
            prop.position.z - 0.2f,
        },
        0.35f * prop.heightScale, prop.color);
}

void TrackDrawPass::drawLiveTreeProp(
    const TrackPropInstance &prop)
{
    float trunkH = 1.6f * prop.heightScale;

    DrawCylinder(
        Vector3{prop.position.x, 0.0f, prop.position.z},
        0.25f, 0.3f, trunkH, 6, Color{92, 64, 40, 255});
    DrawCylinder(
        Vector3{prop.position.x, trunkH, prop.position.z},
        1.6f * prop.heightScale, 0.0f, 2.2f * prop.heightScale, 8,
        prop.color);
}

void TrackDrawPass::drawOneProp(
    const TrackPropInstance &prop)
{
    float shadowR = (prop.type == 1) ? 1.8f : 1.2f;

    TrackMeshBuilder::drawPropShadow(prop.position, shadowR);
    if (prop.type == 1)
        drawBuildingProp(prop);
    else if (prop.type == 2)
        drawDeadTreeProp(prop);
    else
        drawLiveTreeProp(prop);
}

void TrackDrawPass::drawProps(const TrackRenderer &renderer)
{
    for (const auto &prop : renderer.props_)
        drawOneProp(prop);
}

void TrackDrawPass::drawTireStacks(const TrackRenderer &renderer)
{
    for (const auto &stack : renderer.tireStacks_) {
        TrackMeshBuilder::drawPropShadow(stack.position, 0.9f);
        for (int t = 0; t < stack.tiers; ++t) {
            Color tc = (t % 2 == 0)
                ? Color{30, 30, 32, 255} : Color{220, 220, 220, 255};

            DrawCylinder(
                Vector3{
                    stack.position.x,
                    static_cast<float>(t) * 0.35f + 0.18f,
                    stack.position.z,
                },
                0.55f, 0.55f, 0.35f, 10, tc);
        }
    }
}

void TrackDrawPass::drawOnePennant(
    const TrackPennantInstance &pen, float timeSeconds)
{
    float wave = std::sin(timeSeconds * 3.0f + pen.phase) * 0.3f;

    DrawCylinderEx(
        pen.base, pen.top, 0.06f, 0.04f, 4, Color{120, 120, 130, 255});
    Vector3 flagEnd{
        pen.top.x + 1.2f + wave,
        pen.top.y + std::sin(timeSeconds * 4.0f + pen.phase) * 0.15f,
        pen.top.z,
    };

    DrawCylinderEx(pen.top, flagEnd, 0.02f, 0.02f, 3, pen.color);
    DrawTriangle3D(
        pen.top, flagEnd,
        Vector3{pen.top.x, pen.top.y - 0.5f, pen.top.z}, pen.color);
}

void TrackDrawPass::drawPennants(
    const TrackRenderer &renderer, float timeSeconds)
{
    for (const auto &pen : renderer.pennants_)
        drawOnePennant(pen, timeSeconds);
}


} // namespace racer
