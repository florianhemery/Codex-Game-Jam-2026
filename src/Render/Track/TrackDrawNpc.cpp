/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track NPC draw helpers
*/

#include "Render/Track/TrackDrawPass.hpp"
#include "Render/Track/TrackMeshBuilder.hpp"
#include "Render/Track/TrackRenderer.hpp"
#include "rlgl.h"

namespace racer {

void TrackDrawPass::drawNpcBody(
    const TrackNpcInstance &npc, float armWave)
{
    DrawCylinder(
        Vector3{0.0f, 0.9f, 0.0f}, 0.22f, 0.25f, 1.1f, 6, npc.shirtColor);
    DrawSphere(Vector3{0.0f, 1.65f, 0.0f}, 0.22f, Color{240, 200, 170, 255});
    DrawCube(
        Vector3{0.45f, 1.3f + armWave * 0.15f, 0.0f},
        0.12f, 0.5f, 0.12f, npc.shirtColor);
    DrawCube(
        Vector3{-0.35f, 1.1f, 0.0f}, 0.12f, 0.4f, 0.12f, npc.shirtColor);
    DrawCylinder(
        Vector3{0.7f, 1.5f, 0.0f}, 0.03f, 0.03f, 1.2f, 4,
        Color{100, 80, 60, 255});
}

void TrackDrawPass::drawNpcFlag(
    const TrackNpcInstance &npc, float armWave)
{
    rlPushMatrix();
    rlRotatef(armWave * 25.0f, 0.0f, 0.0f, 1.0f);
    DrawCube(
        Vector3{0.7f, 2.2f, 0.0f}, 0.5f, 0.35f, 0.05f, npc.flagColor);
    rlPopMatrix();
}

void TrackDrawPass::drawOneNpc(
    const TrackNpcInstance &npc, float timeSeconds)
{
    TrackMeshBuilder::drawPropShadow(npc.position, 0.45f);
    float armWave = std::sin(timeSeconds * 4.0f + npc.animPhase);

    rlPushMatrix();
    rlTranslatef(npc.position.x, 0.0f, npc.position.z);
    rlRotatef(npc.heading * RAD2DEG, 0.0f, 1.0f, 0.0f);
    drawNpcBody(npc, armWave);
    drawNpcFlag(npc, armWave);
    rlPopMatrix();
}

void TrackDrawPass::drawNpcs(
    const TrackRenderer &renderer, float timeSeconds)
{
    for (const auto &npc : renderer.npcs_)
        drawOneNpc(npc, timeSeconds);
}

} // namespace racer

} // namespace racer
