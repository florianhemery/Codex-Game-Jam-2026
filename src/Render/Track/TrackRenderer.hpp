/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track mesh building and environment rendering facade
*/

#ifndef TRACK_RENDERER_HPP_
#define TRACK_RENDERER_HPP_

#include <vector>

#include "raylib.h"
#include "Render/Track/TrackInstanceTypes.hpp"
#include "Track/Track.hpp"

namespace racer {

class TrackRenderer {
public:
    TrackRenderer(const Track &track, const TrackDef &def);
    ~TrackRenderer();
    TrackRenderer(const TrackRenderer &) = delete;
    TrackRenderer &operator=(const TrackRenderer &) = delete;

    void draw(float timeSeconds) const;
    void drawOpaqueGeometry() const;
    void drawSkyGradient(int screenWidth, int screenHeight) const;
    void applyShader(Shader shader);
    void queueSkidMark(Vector3 pos, Vector3 dir, float width, float strength);
    void flushSkidMarks();

private:
    friend struct TrackDecorBuilder;
    friend struct TrackDrawPass;
    friend struct TrackSkidMarks;

    using PropInstance = TrackPropInstance;
    using CloudInstance = TrackCloudInstance;
    using SpectatorInstance = TrackSpectatorInstance;
    using GrandstandInstance = TrackGrandstandInstance;
    using NpcInstance = TrackNpcInstance;
    using TireStackInstance = TrackTireStackInstance;
    using PennantInstance = TrackPennantInstance;
    using PotholeInstance = TrackPotholeInstance;
    using CrackInstance = TrackCrackInstance;
    using SkidMarkCmd = TrackSkidMarkCmd;
    using LampInstance = TrackLampInstance;
    using ArchInstance = TrackArchInstance;

    SurfaceStyle surfaceStyle_ = SurfaceStyle::PROPRE;
    Vector3 startGantryBase_{};
    Vector3 startGantryAlong_{};
    Vector3 startGantryPerp_{};
    float trackHalfWidth_ = 6.0f;

    Model trackModel_{};
    Model rubberLineModel_{};
    Model centerDashModel_{};
    Model edgeLineOuterModel_{};
    Model edgeLineInnerModel_{};
    Model curbModelOuter_{};
    Model curbModelInner_{};
    Model groundModel_{};
    Model finishLineModel_{};
    std::vector<PropInstance> props_;
    std::vector<CloudInstance> clouds_;
    std::vector<GrandstandInstance> grandstands_;
    std::vector<NpcInstance> npcs_;
    std::vector<TireStackInstance> tireStacks_;
    std::vector<PennantInstance> pennants_;
    std::vector<PotholeInstance> potholes_;
    std::vector<CrackInstance> cracks_;

    RenderTexture2D skidTexture_{};
    Model skidOverlayModel_{};
    Vector2 skidWorldOrigin_{};
    float skidWorldSize_ = 1.0f;
    std::vector<SkidMarkCmd> skidQueue_;

    Model barrierModel_{};
    Model sponsorModel_{};
    bool hasBarriers_ = false;
    bool hasSponsors_ = false;
    std::vector<LampInstance> lamps_;
    ArchInstance arch_{};
    bool hasArch_ = false;
};

} // namespace racer

#endif /* !TRACK_RENDERER_HPP_ */
