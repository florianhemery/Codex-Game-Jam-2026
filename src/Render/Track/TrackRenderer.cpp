/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track mesh building and environment rendering
*/

#include "Render/Track/TrackRenderer.hpp"

#include <cmath>
#include <vector>

#include "Render/Track/TrackDecorBuilder.hpp"
#include "Render/Track/TrackDrawPass.hpp"
#include "Render/Track/TrackMeshBuilder.hpp"
#include "Render/Track/TrackSkidMarks.hpp"
#include "Render/Track/TrackSkyDraw.hpp"
#include "rlgl.h"

namespace racer {

namespace {
constexpr float kGroundDrawY = -0.10f;
} // namespace

TrackRenderer::TrackRenderer(const Track &track, const TrackDef &def)
    : surfaceStyle_(def.surfaceStyle)
{
    std::vector<Vector2> perp =
        TrackMeshBuilder::computePerpendiculars(track);
    float halfWidth = track.width() * 0.5f;

    trackHalfWidth_ = halfWidth;
    size_t n = track.waypoints().size();

    TrackDecorBuilder::buildSceneMeshes(
        *this, track, perp, halfWidth, n);
    TrackDecorBuilder::buildSceneDecor(
        *this, track, perp, halfWidth, n);
}

TrackRenderer::~TrackRenderer()
{
    UnloadModel(trackModel_);
    UnloadModel(rubberLineModel_);
    UnloadModel(centerDashModel_);
    UnloadModel(edgeLineOuterModel_);
    UnloadModel(edgeLineInnerModel_);
    UnloadModel(curbModelOuter_);
    UnloadModel(curbModelInner_);
    UnloadModel(groundModel_);
    UnloadModel(finishLineModel_);
    if (skidTexture_.id != 0)
        UnloadRenderTexture(skidTexture_);
    UnloadModel(skidOverlayModel_);
    if (hasBarriers_)
        UnloadModel(barrierModel_);
    if (hasSponsors_)
        UnloadModel(sponsorModel_);
}

void TrackRenderer::drawOpaqueGeometry() const
{
    DrawModel(
        groundModel_,
        Vector3{groundCenter_.x, kGroundDrawY, groundCenter_.z},
        1.0f, WHITE);
    rlDisableBackfaceCulling();
    DrawModel(trackModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    DrawModel(rubberLineModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    DrawModel(centerDashModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    DrawModel(edgeLineOuterModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    DrawModel(edgeLineInnerModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    DrawModel(curbModelOuter_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    DrawModel(curbModelInner_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    rlEnableBackfaceCulling();
    if (hasBarriers_)
        DrawModel(barrierModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    if (hasSponsors_)
        DrawModel(sponsorModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    BeginBlendMode(BLEND_ALPHA);
    rlDisableDepthMask();
    DrawModel(skidOverlayModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    rlEnableDepthMask();
    EndBlendMode();
    rlDisableBackfaceCulling();
    DrawModel(finishLineModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    rlEnableBackfaceCulling();
}

void TrackRenderer::draw(float timeSeconds) const
{
    TrackMeshBuilder::drawMountainsRing(groundSpan_ * 0.55f, 2.0f, 56);
    TrackDrawPass::drawClouds(*this, timeSeconds);
    drawOpaqueGeometry();
    TrackDrawPass::drawLamps(*this, timeSeconds);
    TrackDrawPass::drawArch(*this);
    TrackDrawPass::drawRoadDamage(*this);
    TrackDrawPass::drawAbimeeDebris(*this);
    TrackDrawPass::drawStartGantry(*this, timeSeconds);
    TrackDrawPass::drawGrandstandStructure(*this);
    TrackDrawPass::drawSpectators(*this, timeSeconds);
    TrackDrawPass::drawProps(*this);
    TrackDrawPass::drawTireStacks(*this);
    TrackDrawPass::drawPennants(*this, timeSeconds);
    TrackDrawPass::drawNpcs(*this, timeSeconds);
}

void TrackRenderer::applyShader(Shader shader)
{
    TrackMeshBuilder::applyShaderToModel(trackModel_, shader);
    TrackMeshBuilder::applyShaderToModel(rubberLineModel_, shader);
    TrackMeshBuilder::applyShaderToModel(centerDashModel_, shader);
    TrackMeshBuilder::applyShaderToModel(edgeLineOuterModel_, shader);
    TrackMeshBuilder::applyShaderToModel(edgeLineInnerModel_, shader);
    TrackMeshBuilder::applyShaderToModel(curbModelOuter_, shader);
    TrackMeshBuilder::applyShaderToModel(curbModelInner_, shader);
    TrackMeshBuilder::applyShaderToModel(groundModel_, shader);
    TrackMeshBuilder::applyShaderToModel(finishLineModel_, shader);
    TrackMeshBuilder::applyShaderToModel(skidOverlayModel_, shader);
    if (hasBarriers_)
        TrackMeshBuilder::applyShaderToModel(barrierModel_, shader);
    if (hasSponsors_)
        TrackMeshBuilder::applyShaderToModel(sponsorModel_, shader);
}

void TrackRenderer::drawSkyGradient(int screenWidth, int screenHeight) const
{
    TrackSkyDraw::drawGradient(screenWidth, screenHeight);
}

void TrackRenderer::queueSkidMark(
    Vector3 pos, Vector3 dir, float width, float strength)
{
    TrackSkidMarks::queue(*this, pos, dir, width, strength);
}

void TrackRenderer::flushSkidMarks()
{
    TrackSkidMarks::flush(*this);
}

} // namespace racer
