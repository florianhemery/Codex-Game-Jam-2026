#pragma once

#include <vector>

#include "raylib.h"
#include "track/track.h"

namespace racer {

// Degrade vertical 2D (avant BeginMode3D).
void DrawSkyGradient(int screenWidth, int screenHeight);

class TrackRenderer {
public:
    TrackRenderer(const Track& track, const TrackDef& def);
    ~TrackRenderer();
    TrackRenderer(const TrackRenderer&) = delete;
    TrackRenderer& operator=(const TrackRenderer&) = delete;

    void Draw(float timeSeconds) const;

    /// Geometrie opaque (meshes) pour la passe d'ombres et la scene eclairee.
    void DrawOpaqueGeometry() const;

    // Assigne `shader` aux materiaux de tous les Models du renderer (chaussee,
    // sol, vibreurs, marquages, barrieres, panneaux, overlay de traces...).
    void ApplyShader(Shader shader);

    // Empile une trace de pneu persistante (appelable pendant la simulation,
    // thread principal uniquement). dir = direction du deplacement, normalisee ;
    // strength dans 0..1.
    void QueueSkidMark(Vector3 pos, Vector3 dir, float width, float strength);

    // Ecrit les traces en attente dans la texture persistante. A appeler entre
    // BeginDrawing et BeginMode3D (jamais pendant le mode 3D).
    void FlushSkidMarks();

private:
    struct PropInstance {
        Vector3 position;
        float heightScale;
        int type; // 0 = arbre, 1 = immeuble, 2 = arbre mort
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

    SurfaceStyle surfaceStyle_ = SurfaceStyle::Propre;
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
    Model shoulderOuterModel_{};
    Model shoulderInnerModel_{};
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

    // Traces de pneus persistantes : texture ecran de la zone carree couvrant
    // l'AABB XZ de la piste, plaquee sur un quad au-dessus de l'asphalte.
    RenderTexture2D skidTexture_{};
    Model skidOverlayModel_{};
    Vector2 skidWorldOrigin_{};  // coin (x, z) min de la zone carree couverte
    float skidWorldSize_ = 1.0f; // cote de la zone carree (mapping uniforme)
    std::vector<SkidMarkCmd> skidQueue_;

    // Decor Phase 4 : barrieres et panneaux en meshes fusionnes (1 draw call
    // chacun), lampadaires et arche en primitives (peu d'instances).
    Model barrierModel_{};
    Model sponsorModel_{};
    bool hasBarriers_ = false;
    bool hasSponsors_ = false;
    std::vector<LampInstance> lamps_;
    ArchInstance arch_{};
    bool hasArch_ = false;
};

} // namespace racer
