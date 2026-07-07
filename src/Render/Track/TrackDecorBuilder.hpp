/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track scene decor population
*/

#ifndef TRACK_DECOR_BUILDER_HPP_
#define TRACK_DECOR_BUILDER_HPP_

#include <cstddef>
#include <vector>

#include "raylib.h"
#include "Render/Track/TrackMeshBuilder.hpp"
#include "Render/Track/TrackInstanceTypes.hpp"
#include "Track/Track.hpp"

namespace racer {

class TrackRenderer;

struct TrackDecorBuilder {

    static void buildTrackMeshes(
        TrackRenderer &renderer, const Track &track,
        const std::vector<Vector2> &perp, float halfWidth);
    static void buildGroundAndFinish(
        TrackRenderer &renderer, const Track &track,
        const std::vector<Vector2> &perp);
    static void initStartGantry(
        TrackRenderer &renderer, const Track &track,
        const std::vector<Vector2> &perp);
    static void buildCloudRing(TrackRenderer &renderer);
    static void buildCloudPuffs(
        TrackCloudInstance &cloud, int cloudIndex);
    static void buildGrandstands(
        TrackRenderer &renderer, const Track &track,
        const std::vector<Vector2> &perp, float halfWidth);
    static bool shouldEndStraightRun(
        const Track &track, size_t i, size_t n, bool &endRun);
    static void tryAddGrandstand(
        TrackRenderer &renderer, const Track &track,
        const std::vector<Vector2> &perp, float halfWidth,
        size_t runStart, size_t runEnd);
    static void fillGrandstandSpectators(
        TrackGrandstandInstance &gs, size_t mid);
    static void populateWaypointDecor(
        TrackRenderer &renderer, const Track &track,
        const std::vector<Vector2> &perp, float halfWidth, size_t i);
    static void addWaypointProp(
        TrackRenderer &renderer, Vector3 pos, uint32_t h,
        SurfaceStyle style);
    static void addWaypointNpc(
        TrackRenderer &renderer, Vector3 pos,
        const Vector2 &perp, float sideSign, uint32_t h);
    static void addWaypointTireStack(
        TrackRenderer &renderer, const Track &track,
        const std::vector<Vector2> &perp, float halfWidth,
        float sideSign, size_t i, uint32_t h);
    static void addWaypointPennant(
        TrackRenderer &renderer, Vector3 pos, uint32_t h);
    static void buildEdgeLineMeshes(
        TrackRenderer &renderer, const Track &track,
        const std::vector<Vector2> &perp, float halfWidth);
    static void buildCurbMeshes(
        TrackRenderer &renderer, const Track &track,
        const std::vector<Vector2> &perp, float halfWidth);
    static void buildGroundAndFinishMeshes(
        TrackRenderer &renderer, const Track &track,
        const std::vector<Vector2> &perp);
    static void addGrandstandSpectator(
        TrackGrandstandInstance &gs, size_t mid,
        int row, int col);
    static void addAbimeePotholeAt(
        TrackRenderer &renderer, const Track &track,
        const std::vector<Vector2> &perp, size_t i, uint32_t h);
    static void computeSkidBounds(
        TrackRenderer &renderer, const Track &track, float halfWidth);
    static void setupSkidTextureModel(TrackRenderer &renderer);
    static bool isSharpCorner(
        const Track &track, size_t i, size_t n, Vector2 &d0, Vector2 &d1);
    static void appendBarrierRails(
        TrackMeshBuilder::MeshBuffers &barrierBuf, Vector3 pos,
        Vector3 along, Vector3 out, SurfaceStyle style);
    static void addSponsorAtWaypoint(
        TrackMeshBuilder::MeshBuffers &sponsorBuf,
        const Track &track, const std::vector<Vector2> &perp,
        float halfWidth, size_t i);
    static void setupLampTop(
        TrackLampInstance &lamp, const Vector2 &perp,
        float sideSign, bool broken);
    static void addLampAtWaypoint(
        TrackRenderer &renderer, const Track &track,
        const std::vector<Vector2> &perp, float halfWidth, size_t i);
    static void buildAbimeeDamage(
        TrackRenderer &renderer, const Track &track,
        const std::vector<Vector2> &perp);
    static void initSkidOverlay(
        TrackRenderer &renderer, const Track &track, float halfWidth);
    static void buildBarrierMeshes(
        TrackRenderer &renderer, const Track &track,
        const std::vector<Vector2> &perp, float halfWidth, size_t n);
    static void addBarrierAtCorner(
        TrackMeshBuilder::MeshBuffers &barrierBuf, const Track &track,
        const std::vector<Vector2> &perp, float halfWidth,
        SurfaceStyle style, size_t i, size_t n);
    static void buildSponsorMeshes(
        TrackRenderer &renderer, const Track &track,
        const std::vector<Vector2> &perp, float halfWidth);
    static void buildLampRing(
        TrackRenderer &renderer, const Track &track,
        const std::vector<Vector2> &perp, float halfWidth);
    static void initInflatableArch(
        TrackRenderer &renderer, const Track &track,
        const std::vector<Vector2> &perp, float halfWidth, size_t n);
    static void fillBuildingProp(
        TrackPropInstance &prop, uint32_t h);
    static void fillTreeProp(
        TrackPropInstance &prop, uint32_t h, bool isDeadTree);
    static TrackGrandstandInstance makeGrandstandInstance(
        const Track &track, float halfWidth, size_t mid,
        Vector2 along, Vector2 outward, float alongLen);
    static void scanWaypointBounds(
        const Track &track, float &minX, float &maxX,
        float &minZ, float &maxZ);
    static void addAbimeeCrackAt(
        TrackRenderer &renderer, const TrackPotholeInstance &hole,
        const std::vector<Vector2> &perp, size_t i, uint32_t h);
    static void appendSponsorPanel(
        TrackMeshBuilder::MeshBuffers &sponsorBuf, Vector3 base,
        Vector3 face, Vector3 right, Color panelColor);
    static Vector3 barrierCornerPos(
        const Track &track, const std::vector<Vector2> &perp,
        float halfWidth, size_t i, float side);
    static Color makeSpectatorShirtColor(uint32_t sh);
    static Vector3 makeSpectatorPosition(
        const TrackGrandstandInstance &gs,
        float rowH, float alongT, float outwardD);
    static bool computeStraightAlong(
        const Track &track, size_t runStart, size_t runEnd,
        Vector2 &along, size_t &mid, float &alongLen);
    static float straightRunDirectionDot(
        const Track &track, size_t i, size_t n);
    static bool isGrandstandRunEnd(
        const Track &track, size_t i, size_t n);
    static void buildSceneMeshes(
        TrackLampInstance &lamp, const Vector2 &perp,
        float sideSign, bool broken);
    static void buildSceneMeshes(
        TrackRenderer &renderer, const Track &track,
        const std::vector<Vector2> &perp, float halfWidth, size_t n);
    static void buildSceneDecor(
        TrackRenderer &renderer, const Track &track,
        const std::vector<Vector2> &perp, float halfWidth, size_t n);
};

} // namespace racer

#endif /* !TRACK_DECOR_BUILDER_HPP_ */
