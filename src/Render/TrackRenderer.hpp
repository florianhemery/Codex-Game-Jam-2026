/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track mesh building and environment rendering
*/

#ifndef TRACK_RENDERER_HPP_
#define TRACK_RENDERER_HPP_

#include <vector>

#include "raylib.h"
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
    friend struct TrackRendererBuild;
    friend struct TrackRendererDraw;

    struct PropInstance {
        Vector3 position;
        float heightScale;
        int type;
        Color color;
    };

    struct CloudInstance {
        Vector3 basePosition;
        float driftSpeed;
        float scale;
        std::vector<Vector3> puffOffsets;
        std::vector<float> puffScales;
    };

    struct SpectatorInstance {
        Vector3 position;
        Color shirtColor;
        float jumpPhase;
        float jumpSpeed;
    };

    struct GrandstandInstance {
        Vector3 origin;
        Vector3 along;
        Vector3 outward;
        float length;
        std::vector<SpectatorInstance> spectators;
    };

    struct NpcInstance {
        Vector3 position;
        float heading;
        Color shirtColor;
        Color flagColor;
        float animPhase;
    };

    struct TireStackInstance {
        Vector3 position;
        int tiers;
    };

    struct PennantInstance {
        Vector3 base;
        Vector3 top;
        Color color;
        float phase;
    };

    struct PotholeInstance {
        Vector3 position;
        float radius;
    };

    struct CrackInstance {
        Vector3 center;
        Vector3 tangent;
        float length;
    };

    struct SkidMarkCmd {
        Vector3 position;
        Vector3 direction;
        float width;
        float strength;
    };

    struct LampInstance {
        Vector3 base;
        Vector3 top;
        Color headColor;
        bool lit;
    };

    struct ArchInstance {
        Vector3 leftBase;
        Vector3 rightBase;
        Color colorA;
        Color colorB;
    };

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
