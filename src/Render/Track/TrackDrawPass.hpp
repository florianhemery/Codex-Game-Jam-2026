/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track environment draw pass
*/

#ifndef TRACK_DRAW_PASS_HPP_
#define TRACK_DRAW_PASS_HPP_

#include "raylib.h"
#include "Render/Track/TrackInstanceTypes.hpp"

namespace racer {

class TrackRenderer;

struct TrackDrawPass {

    static void drawClouds(
        const TrackRenderer &renderer, float timeSeconds);
    static void drawOneCloud(
        const TrackCloudInstance &cloud, float timeSeconds);
    static void drawLamps(
        const TrackRenderer &renderer, float timeSeconds);
    static void drawOneLamp(
        const TrackLampInstance &lamp, float timeSeconds);
    static void drawArch(const TrackRenderer &renderer);
    static void drawArchSpan(const TrackRenderer &renderer);
    static void drawRoadDamage(const TrackRenderer &renderer);
    static void drawAbimeeDebris(const TrackRenderer &renderer);
    static void drawStartGantry(
        const TrackRenderer &renderer, float timeSeconds);
    static void drawGantryBeam(
        Vector3 leftBase, Vector3 rightBase, Vector3 p, float hw,
        float pillarH);
    static void drawGantryCheckerCells(
        Vector3 beamMid, Vector3 p, float hw, float pillarH);
    static void drawGrandstandRows(
        const TrackGrandstandInstance &gs);
    static void drawGrandstandRoof(
        const TrackGrandstandInstance &gs);
    static void drawOneSkidMark(
        const TrackSkidMarkCmd &cmd, Vector2 origin, float worldSize);
    static void drawGrandstandStructure(const TrackRenderer &renderer);
    static void drawOneGrandstand(
        const TrackGrandstandInstance &gs);
    static void drawSpectators(
        const TrackRenderer &renderer, float timeSeconds);
    static void drawOneSpectator(
        const TrackSpectatorInstance &spec, float timeSeconds);
    static void drawProps(const TrackRenderer &renderer);
    static void drawOneProp(const TrackPropInstance &prop);
    static void drawBuildingProp(const TrackPropInstance &prop);
    static void drawDeadTreeProp(const TrackPropInstance &prop);
    static void drawLiveTreeProp(const TrackPropInstance &prop);
    static void drawTireStacks(const TrackRenderer &renderer);
    static void drawPennants(
        const TrackRenderer &renderer, float timeSeconds);
    static void drawOnePennant(
        const TrackPennantInstance &pen, float timeSeconds);
    static void drawNpcs(
        const TrackRenderer &renderer, float timeSeconds);
    static void drawPotholes(const TrackRenderer &renderer);
    static void drawCracks(const TrackRenderer &renderer);
    static void drawNpcBody(
        const TrackNpcInstance &npc, float armWave);
    static void drawNpcFlag(
        const TrackNpcInstance &npc, float armWave);
    static void drawOneNpc(
        const TrackNpcInstance &npc, float timeSeconds);
};

} // namespace racer

#endif /* !TRACK_DRAW_PASS_HPP_ */
