/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Procedural world prop mesh builder (batched per category)
*/

#include "Render/World/WorldPropBuilder.hpp"

#include <algorithm>
#include <cmath>

#include "raymath.h"
#include "rlgl.h"

namespace racer::world {

namespace {

constexpr float kPi = 3.14159265f;

unsigned char clampU8(int v)
{
    return static_cast<unsigned char>(std::clamp(v, 0, 255));
}

Color mulColor(Color a, Color b)
{
    return Color{
        clampU8(static_cast<int>(a.r) * b.r / 255),
        clampU8(static_cast<int>(a.g) * b.g / 255),
        clampU8(static_cast<int>(a.b) * b.b / 255),
        clampU8(static_cast<int>(a.a) * b.a / 255),
    };
}

Color shade(Color c, float factor)
{
    return Color{
        clampU8(static_cast<int>(c.r * factor)),
        clampU8(static_cast<int>(c.g * factor)),
        clampU8(static_cast<int>(c.b * factor)),
        c.a,
    };
}

Vector3 rotateY(Vector3 v, float yaw)
{
    float c = std::cos(yaw);
    float s = std::sin(yaw);
    return Vector3{v.x * c - v.z * s, v.y, v.x * s + v.z * c};
}

void appendBoxY(
    TrackMeshBuilder::MeshBuffers &mb, Vector3 center, float w, float h,
    float d, Color color)
{
    TrackMeshBuilder::appendBox(
        mb, center, Vector3{1.0f, 0.0f, 0.0f}, Vector3{0.0f, 1.0f, 0.0f},
        Vector3{0.0f, 0.0f, 1.0f}, w * 0.5f, h * 0.5f, d * 0.5f, color);
}

void appendCylinderY(
    TrackMeshBuilder::MeshBuffers &mb, Vector3 base, float rBot, float rTop,
    float height, int sides, Color color)
{
    if (sides < 3) {
        sides = 3;
    }
    for (int i = 0; i < sides; ++i) {
        float a0 = kPi * 2.0f * static_cast<float>(i) / static_cast<float>(sides);
        float a1 = kPi * 2.0f * static_cast<float>(i + 1)
            / static_cast<float>(sides);
        Vector3 b0{
            base.x + std::cos(a0) * rBot, base.y,
            base.z + std::sin(a0) * rBot,
        };
        Vector3 b1{
            base.x + std::cos(a1) * rBot, base.y,
            base.z + std::sin(a1) * rBot,
        };
        Vector3 t0{
            base.x + std::cos(a0) * rTop, base.y + height,
            base.z + std::sin(a0) * rTop,
        };
        Vector3 t1{
            base.x + std::cos(a1) * rTop, base.y + height,
            base.z + std::sin(a1) * rTop,
        };
        float midA = (a0 + a1) * 0.5f;
        Vector3 n{std::cos(midA), 0.0f, std::sin(midA)};

        TrackMeshBuilder::appendQuad(mb, b0, b1, t1, t0, n, color);
    }
}

void appendCylinderX(
    TrackMeshBuilder::MeshBuffers &mb, Vector3 base, float radius,
    float width, int sides, Color color)
{
    if (sides < 3) {
        sides = 3;
    }
    for (int i = 0; i < sides; ++i) {
        float a0 = kPi * 2.0f * static_cast<float>(i) / static_cast<float>(sides);
        float a1 = kPi * 2.0f * static_cast<float>(i + 1)
            / static_cast<float>(sides);
        Vector3 b0{
            base.x, base.y + std::cos(a0) * radius,
            base.z + std::sin(a0) * radius,
        };
        Vector3 b1{
            base.x, base.y + std::cos(a1) * radius,
            base.z + std::sin(a1) * radius,
        };
        Vector3 t0{
            base.x + width, base.y + std::cos(a0) * radius,
            base.z + std::sin(a0) * radius,
        };
        Vector3 t1{
            base.x + width, base.y + std::cos(a1) * radius,
            base.z + std::sin(a1) * radius,
        };
        float midA = (a0 + a1) * 0.5f;
        Vector3 n{0.0f, std::cos(midA), std::sin(midA)};

        TrackMeshBuilder::appendQuad(mb, b0, b1, t1, t0, n, color);
    }
}

// Builds a box tilted around the X axis: `w` stays the ridge-parallel
// width (along X, unrotated), `d` becomes the slope run — it sweeps into
// both Z and Y as `pitch` increases, which is what a roof panel or a
// sloped hood/trunk deck needs (rising as it goes front-to-back), rather
// than a Z-axis rotation which would swing the panel sideways past the
// building instead of down toward the eaves.
void appendBoxPitchX(
    TrackMeshBuilder::MeshBuffers &mb, Vector3 center, float pitch, float w,
    float h, float d, Color color)
{
    float c = std::cos(pitch);
    float s = std::sin(pitch);
    TrackMeshBuilder::appendBox(
        mb, center, Vector3{1.0f, 0.0f, 0.0f}, Vector3{0.0f, c, -s},
        Vector3{0.0f, s, c}, w * 0.5f, h * 0.5f, d * 0.5f, color);
}

void appendFrond(
    TrackMeshBuilder::MeshBuffers &mb, Vector3 root, float yaw, float len,
    float scale, Color color)
{
    Vector3 tip{
        root.x + std::cos(yaw) * len * scale,
        root.y + 0.15f * scale,
        root.z + std::sin(yaw) * len * scale,
    };
    Vector3 mid{
        (root.x + tip.x) * 0.5f, (root.y + tip.y) * 0.5f,
        (root.z + tip.z) * 0.5f,
    };
    float frondW = 0.18f * scale;
    appendBoxY(mb, mid, frondW, 0.06f * scale, len * 0.55f, color);
}

void appendSphereApprox(
    TrackMeshBuilder::MeshBuffers &mb, Vector3 center, float radius,
    Color color)
{
    appendBoxY(mb, center, radius * 1.6f, radius * 1.4f, radius * 1.5f, color);
    appendBoxY(
        mb,
        Vector3{center.x, center.y + radius * 0.35f, center.z},
        radius * 1.2f, radius * 0.9f, radius * 1.1f, shade(color, 1.08f));
    appendBoxY(
        mb,
        Vector3{center.x, center.y - radius * 0.25f, center.z},
        radius * 1.3f, radius * 0.7f, radius * 1.35f, shade(color, 0.92f));
}

void appendTransformed(
    TrackMeshBuilder::MeshBuffers &dst, const TrackMeshBuilder::MeshBuffers &src,
    Vector3 pos, float yaw, float uniformScale, float heightScale, Color tint)
{
    if (src.vertices.empty()) {
        return;
    }
    const size_t baseVert = dst.vertices.size() / 3;
    const size_t vertCount = src.vertices.size() / 3;

    for (size_t i = 0; i < vertCount; ++i) {
        Vector3 local{
            src.vertices[i * 3 + 0] * uniformScale,
            src.vertices[i * 3 + 1] * uniformScale * heightScale,
            src.vertices[i * 3 + 2] * uniformScale,
        };
        Vector3 world = rotateY(local, yaw);
        world.x += pos.x;
        world.y += pos.y;
        world.z += pos.z;
        Vector3 normal = rotateY(
            Vector3{
                src.normals[i * 3 + 0], src.normals[i * 3 + 1],
                src.normals[i * 3 + 2],
            },
            yaw);
        Color vc{
            src.colors[i * 4 + 0], src.colors[i * 4 + 1],
            src.colors[i * 4 + 2], src.colors[i * 4 + 3],
        };
        Color tc = mulColor(vc, tint);

        dst.vertices.push_back(world.x);
        dst.vertices.push_back(world.y);
        dst.vertices.push_back(world.z);
        dst.normals.push_back(normal.x);
        dst.normals.push_back(normal.y);
        dst.normals.push_back(normal.z);
        dst.colors.push_back(tc.r);
        dst.colors.push_back(tc.g);
        dst.colors.push_back(tc.b);
        dst.colors.push_back(tc.a);
    }
    for (unsigned short idx : src.indices) {
        size_t outIdx = baseVert + idx;
        if (outIdx >= 65536u) {
            continue;
        }
        dst.indices.push_back(static_cast<unsigned short>(outIdx));
    }
}

} // namespace

WorldPropBuilder::WorldPropBuilder() = default;

WorldPropBuilder::~WorldPropBuilder()
{
    for (PropBatch &batch : batches_) {
        if (batch.hasModel) {
            UnloadModel(batch.model);
            batch.hasModel = false;
        }
    }
}

void WorldPropBuilder::setShader(Shader shader)
{
    shader_ = shader;
    hasShader_ = shader.id != 0;
    for (PropBatch &batch : batches_) {
        if (batch.hasModel) {
            TrackMeshBuilder::applyShaderToModel(batch.model, shader);
        }
    }
}

void WorldPropBuilder::beginBatch(BatchId id)
{
    PropBatch &batch = batches_[static_cast<size_t>(id)];
    batch.buffers.vertices.clear();
    batch.buffers.normals.clear();
    batch.buffers.colors.clear();
    batch.buffers.indices.clear();
}

void WorldPropBuilder::uploadAndDraw(BatchId id)
{
    PropBatch &batch = batches_[static_cast<size_t>(id)];
    if (batch.buffers.vertices.empty()) {
        return;
    }
    if (batch.hasModel) {
        UnloadModel(batch.model);
        batch.hasModel = false;
    }
    Mesh mesh = TrackMeshBuilder::meshFromBuffers(batch.buffers);
    if (mesh.vertexCount <= 0 || mesh.triangleCount <= 0) {
        return;
    }
    batch.model = LoadModelFromMesh(mesh);
    if (hasShader_) {
        TrackMeshBuilder::applyShaderToModel(batch.model, shader_);
    }
    batch.hasModel = true;
    rlDisableBackfaceCulling();
    DrawModel(batch.model, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    rlEnableBackfaceCulling();
}

void WorldPropBuilder::flush(BatchId id)
{
    uploadAndDraw(id);
}

uint32_t WorldPropBuilder::hashPosition(float x, float z)
{
    uint32_t hx = static_cast<uint32_t>(x * 12.9898f + z * 78.233f);
    hx = (hx ^ (hx >> 13)) * 1274126177u;
    return hx ^ (hx >> 16);
}

PropVariation WorldPropBuilder::variationFromHash(uint32_t h, Color baseTint)
{
    PropVariation v{};
    v.scaleMul = 0.88f + static_cast<float>(h % 37) / 100.0f;
    v.heightScale = 0.92f + static_cast<float>((h >> 6) % 29) / 100.0f;
    v.subVariant = static_cast<int>((h >> 12) % 3);
    float tintMul = 0.9f + static_cast<float>((h >> 4) % 21) / 100.0f;
    v.tint = mulColor(baseTint, Color{
        clampU8(static_cast<int>(255 * tintMul)),
        clampU8(static_cast<int>(255 * (tintMul + 0.02f))),
        clampU8(static_cast<int>(255 * (tintMul - 0.03f))),
        255,
    });
    return v;
}

void WorldPropBuilder::appendToBatch(
    BatchId id, const MeshBuffers &geom, Vector3 pos, float yaw,
    float uniformScale, float heightScale, Color tint)
{
    appendTransformed(
        batches_[static_cast<size_t>(id)].buffers, geom, pos, yaw,
        uniformScale, heightScale, tint);
}

void WorldPropBuilder::buildPine(
    MeshBuffers &mb, float scale, float heightScale, Color tint)
{
    float hs = heightScale;
    Color trunk = mulColor(Color{92, 64, 40, 255}, tint);
    Color foliage = mulColor(Color{28, 96, 42, 255}, tint);
    float trunkH = 1.8f * scale * hs;

    appendCylinderY(
        mb, Vector3{0.0f, 0.0f, 0.0f}, 0.22f * scale, 0.28f * scale,
        trunkH, 6, trunk);

    // Three stacked, shrinking tiers give a layered conifer silhouette
    // instead of a single cone blob.
    float tierH = 1.15f * scale * hs;
    float baseR = 1.55f * scale;
    for (int tier = 0; tier < 3; ++tier) {
        float t = static_cast<float>(tier);
        float y = trunkH + t * tierH * 0.72f;
        float rBot = baseR * (1.0f - t * 0.3f);
        float rTop = rBot * 0.18f;
        Color tierColor = shade(foliage, 1.0f - t * 0.06f);
        appendCylinderY(mb, Vector3{0.0f, y, 0.0f}, rBot, rTop, tierH, 8,
            tierColor);
    }
    appendSphereApprox(
        mb,
        Vector3{0.0f, trunkH + tierH * 2.5f, 0.0f},
        0.4f * scale, Fade(shade(foliage, 0.9f), 0.9f));
}

void WorldPropBuilder::buildBroadleaf(
    MeshBuffers &mb, float scale, float heightScale, Color tint)
{
    float hs = heightScale;
    Color trunk = mulColor(Color{82, 58, 36, 255}, tint);
    Color foliage = mulColor(Color{52, 128, 48, 255}, tint);
    float trunkH = 1.5f * scale * hs;

    appendCylinderY(
        mb, Vector3{0.0f, 0.0f, 0.0f}, 0.26f * scale, 0.3f * scale,
        trunkH, 6, trunk);
    // A short branch fork before the canopy breaks up the straight trunk.
    appendCylinderY(
        mb, Vector3{0.0f, trunkH * 0.72f, 0.0f}, 0.16f * scale, 0.1f * scale,
        0.55f * scale * hs, 5, shade(trunk, 0.92f));

    // An asymmetric cluster of offset lobes reads as a fuller, less
    // perfectly-round canopy than a couple of centered spheres.
    appendSphereApprox(
        mb, Vector3{0.0f, trunkH + 1.35f * scale * hs, 0.0f},
        1.15f * scale, foliage);
    appendSphereApprox(
        mb,
        Vector3{0.62f * scale, trunkH + 0.95f * scale * hs, 0.25f * scale},
        0.82f * scale, shade(foliage, 0.95f));
    appendSphereApprox(
        mb,
        Vector3{-0.5f * scale, trunkH + 1.05f * scale * hs, -0.35f * scale},
        0.7f * scale, shade(foliage, 1.06f));
    appendSphereApprox(
        mb,
        Vector3{0.1f * scale, trunkH + 1.75f * scale * hs, 0.35f * scale},
        0.6f * scale, shade(foliage, 1.1f));
}

void WorldPropBuilder::buildPalm(
    MeshBuffers &mb, float scale, float heightScale, Color tint)
{
    float hs = heightScale;
    Color trunk = mulColor(Color{110, 78, 48, 255}, tint);
    Color frond = mulColor(Color{48, 130, 52, 255}, tint);
    float trunkH = 2.2f * scale * hs;

    // A slight lean, built from two stacked segments offset in X, reads
    // much more like a real palm than a dead-straight trunk.
    float seg1H = trunkH * 0.55f;
    float seg2H = trunkH - seg1H;
    float lean = 0.35f * scale;
    appendCylinderY(
        mb, Vector3{0.0f, 0.0f, 0.0f}, 0.2f * scale, 0.17f * scale,
        seg1H, 6, trunk);
    appendCylinderY(
        mb, Vector3{lean * 0.5f, seg1H, 0.0f}, 0.17f * scale, 0.13f * scale,
        seg2H, 6, shade(trunk, 0.95f));
    Vector3 crown{lean, trunkH, 0.0f};
    for (int i = 0; i < 6; ++i) {
        float a = static_cast<float>(i) / 6.0f * kPi * 2.0f;
        Color frondColor = (i % 2 == 0) ? frond : shade(frond, 1.08f);
        appendFrond(mb, crown, a, 1.5f, scale, frondColor);
    }
    for (int i = 0; i < 3; ++i) {
        float a = static_cast<float>(i) / 3.0f * kPi * 2.0f + 0.4f;
        Vector3 coco{
            crown.x + std::cos(a) * 0.3f * scale,
            crown.y - 0.15f * scale,
            crown.z + std::sin(a) * 0.3f * scale,
        };
        appendSphereApprox(mb, coco, 0.16f * scale, Color{92, 66, 40, 255});
    }
}

void WorldPropBuilder::buildRock(
    MeshBuffers &mb, float scale, float heightScale, Color tint)
{
    Color rock = mulColor(Color{68, 62, 58, 255}, tint);
    // A cluster of irregularly-rotated, unevenly-sized chunks reads as a
    // weathered boulder pile instead of two stacked cubes.
    struct Chunk {
        float x, y, z, w, h, d, yaw, shadeMul;
    };
    const Chunk chunks[] = {
        {0.0f, 0.75f, 0.0f, 2.0f, 1.5f, 1.9f, 0.15f, 1.0f},
        {0.55f, 1.25f, -0.3f, 1.1f, 0.85f, 1.0f, -0.35f, 1.08f},
        {-0.55f, 0.55f, 0.35f, 1.05f, 0.75f, 1.05f, 0.6f, 0.9f},
        {0.15f, 1.6f, 0.35f, 0.7f, 0.55f, 0.65f, 1.0f, 1.12f},
    };
    for (const Chunk &c : chunks) {
        float ch = std::cos(c.yaw);
        float sh = std::sin(c.yaw);
        Vector3 center{c.x * scale, c.y * scale * heightScale, c.z * scale};
        TrackMeshBuilder::appendBox(
            mb, center, Vector3{ch, 0.0f, sh}, Vector3{0.0f, 1.0f, 0.0f},
            Vector3{-sh, 0.0f, ch}, c.w * 0.5f * scale,
            c.h * 0.5f * scale * heightScale, c.d * 0.5f * scale,
            shade(rock, c.shadeMul));
    }
}

void WorldPropBuilder::buildIndustrialSilo(
    MeshBuffers &mb, float scale, float heightScale, Color tint)
{
    Color silo = mulColor(Color{140, 142, 148, 255}, tint);
    Color annex = mulColor(Color{90, 92, 98, 255}, tint);
    Color band = shade(silo, 0.68f);
    float siloH = 5.5f * scale * heightScale;

    appendCylinderY(
        mb, Vector3{0.0f, 0.0f, 0.0f}, 1.2f * scale, 1.2f * scale,
        siloH, 10, silo);
    for (int i = 1; i < 4; ++i) {
        float y = siloH * static_cast<float>(i) / 4.0f;
        appendCylinderY(mb, Vector3{0.0f, y, 0.0f}, 1.22f * scale,
            1.22f * scale, 0.12f * scale, 10, band);
    }
    appendCylinderY(
        mb, Vector3{0.95f * scale, 0.2f * scale, 0.75f * scale}, 0.05f * scale,
        0.05f * scale, siloH * 0.85f, 4, shade(silo, 0.6f));
    appendBoxY(
        mb,
        Vector3{2.0f * scale, 1.5f * scale * heightScale, 0.0f},
        2.5f * scale, 2.0f * scale * heightScale, 3.0f * scale, annex);
    appendCylinderY(
        mb, Vector3{0.0f, siloH, 0.0f}, 1.25f * scale, 0.15f * scale,
        0.9f * scale, 10, shade(silo, 0.75f));
    appendBoxY(
        mb,
        Vector3{0.0f, siloH + 1.05f * scale, 0.0f},
        0.35f * scale, 0.35f * scale, 0.35f * scale, shade(silo, 0.65f));
}

void WorldPropBuilder::buildIndustrialTank(
    MeshBuffers &mb, float scale, float heightScale, Color tint)
{
    Color tank = mulColor(Color{118, 122, 130, 255}, tint);
    Color pipe = mulColor(Color{72, 74, 80, 255}, tint);
    float tankR = 1.6f * scale;
    float tankH = 2.2f * scale * heightScale;

    appendCylinderY(
        mb, Vector3{0.0f, 0.6f * scale, 0.0f}, tankR, tankR,
        tankH, 10, tank);
    appendCylinderY(
        mb, Vector3{0.0f, 0.6f * scale + tankH, 0.0f}, tankR, tankR * 0.15f,
        0.5f * scale, 10, shade(tank, 0.85f));
    appendBoxY(
        mb,
        Vector3{0.0f, 0.25f * scale, 0.0f},
        tankR * 2.2f, 0.5f * scale, tankR * 2.0f, shade(tank, 0.9f));
    appendCylinderY(
        mb,
        Vector3{tankR + 0.3f * scale, 1.2f * scale * heightScale, 0.0f},
        0.15f * scale, 0.15f * scale, 2.8f * scale * heightScale, 6, pipe);
    appendCylinderY(
        mb,
        Vector3{-tankR - 0.3f * scale, 0.5f * scale, 0.0f},
        0.13f * scale, 0.13f * scale, 1.6f * scale, 6, shade(pipe, 1.1f));
    // Valve wheel on the near-side pipe.
    appendCylinderX(
        mb, Vector3{tankR + 0.15f * scale, 0.9f * scale, 0.0f}, 0.22f * scale,
        0.05f * scale, 8, shade(pipe, 1.2f));
}

void WorldPropBuilder::buildIndustrialAnnex(
    MeshBuffers &mb, float scale, float heightScale, Color tint)
{
    Color wall = mulColor(Color{96, 98, 104, 255}, tint);
    Color roof = mulColor(Color{72, 74, 80, 255}, tint);

    appendBoxY(
        mb, Vector3{0.0f, 1.4f * scale * heightScale, 0.0f},
        4.0f * scale, 2.8f * scale * heightScale, 3.5f * scale, wall);
    appendBoxY(
        mb,
        Vector3{0.0f, 2.9f * scale * heightScale, 0.0f},
        4.2f * scale, 0.35f * scale, 3.7f * scale, roof);
    for (int i = 0; i < 3; ++i) {
        appendBoxY(
            mb,
            Vector3{
                -1.2f * scale + static_cast<float>(i) * 1.2f * scale,
                1.5f * scale * heightScale, 1.76f * scale,
            },
            0.5f * scale, 0.7f * scale, 0.06f * scale,
            Color{140, 180, 210, 200});
    }
}

void WorldPropBuilder::buildCoastalBuilding(MeshBuffers &mb, float scale,
    Color tint)
{
    Color wall = mulColor(Color{220, 210, 190, 255}, tint);
    Color roof = mulColor(Color{180, 70, 50, 255}, tint);
    Color trim = shade(wall, 0.8f);

    appendBoxY(mb, Vector3{0.0f, 2.5f * scale, 0.0f}, 8.0f * scale,
        5.0f * scale, 6.0f * scale, wall);
    appendBoxY(mb, Vector3{0.0f, 5.05f * scale, 0.0f}, 8.2f * scale,
        0.2f * scale, 6.2f * scale, trim);

    // Pitched roof instead of a flat slab, matching the garage's language.
    appendBoxPitchX(
        mb, Vector3{0.0f, 5.9f * scale, 1.7f * scale}, -0.5f, 8.6f * scale,
        0.18f * scale, 3.6f * scale, roof);
    appendBoxPitchX(
        mb, Vector3{0.0f, 5.9f * scale, -1.7f * scale}, 0.5f, 8.6f * scale,
        0.18f * scale, 3.6f * scale, shade(roof, 0.9f));
    appendBoxY(mb, Vector3{0.0f, 6.85f * scale, 0.0f}, 8.7f * scale,
        0.22f * scale, 0.5f * scale, shade(roof, 1.1f));
    appendCylinderY(mb, Vector3{2.6f * scale, 5.9f * scale, -0.6f * scale},
        0.3f * scale, 0.3f * scale, 2.0f * scale, 6, shade(trim, 0.75f));

    for (int i = 0; i < 4; ++i) {
        appendBoxY(
            mb,
            Vector3{
                (-2.4f + static_cast<float>(i) * 1.6f) * scale,
                2.2f * scale, 3.05f * scale,
            },
            0.6f * scale, 0.9f * scale, 0.08f * scale,
            Color{120, 180, 220, 210});
        appendBoxY(
            mb,
            Vector3{
                (-2.4f + static_cast<float>(i) * 1.6f) * scale,
                2.2f * scale, 3.1f * scale,
            },
            0.66f * scale, 0.98f * scale, 0.04f * scale, trim);
    }
    appendBoxY(mb, Vector3{0.0f, 0.6f * scale, 3.02f * scale}, 2.2f * scale,
        1.6f * scale, 0.1f * scale, Color{110, 78, 48, 255});
}

void WorldPropBuilder::buildPierSegment(
    MeshBuffers &mb, float length, Color wood, Color post)
{
    appendBoxY(mb, Vector3{0.0f, 0.6f, 0.0f}, 1.2f, 1.2f, length, wood);
    appendCylinderY(
        mb, Vector3{0.0f, 0.9f, length * 0.45f}, 0.35f, 0.35f, 1.8f, 6, post);
    appendCylinderY(
        mb, Vector3{0.0f, 0.9f, -length * 0.45f}, 0.35f, 0.35f, 1.8f, 6, post);
}

void WorldPropBuilder::buildWaterPlane(
    MeshBuffers &mb, float width, float depth, Color color)
{
    float hw = width * 0.5f;
    float hd = depth * 0.5f;
    Vector3 p0{-hw, 0.05f, -hd};
    Vector3 p1{hw, 0.05f, -hd};
    Vector3 p2{hw, 0.05f, hd};
    Vector3 p3{-hw, 0.05f, hd};
    TrackMeshBuilder::appendQuad(
        mb, p0, p1, p2, p3, Vector3{0.0f, 1.0f, 0.0f}, color);
}

void WorldPropBuilder::buildRaceGate(
    MeshBuffers &mb, Color colorA, Color colorB, float pulse)
{
    constexpr float pillarH = 5.0f;
    constexpr float span = 5.0f;

    appendCylinderY(
        mb, Vector3{-span, 0.0f, 0.0f}, 0.35f, 0.35f, pillarH, 8, colorA);
    appendCylinderY(
        mb, Vector3{span, 0.0f, 0.0f}, 0.35f, 0.35f, pillarH, 8, colorB);
    for (int s = 0; s < 7; ++s) {
        float t = static_cast<float>(s) / 6.0f;
        float archY = pillarH + 1.4f * std::sin(t * kPi);
        float px = -span + (span * 2.0f) * t;
        Color c = (s % 2 == 0) ? colorA : colorB;
        appendBoxY(mb, Vector3{px, archY, 0.0f}, 1.2f, 0.45f, 0.45f, c);
    }
    appendSphereApprox(
        mb, Vector3{0.0f, pillarH + 0.6f, 0.0f},
        0.35f + pulse * 0.1f, Fade(WHITE, 0.9f));
}

void WorldPropBuilder::buildGarage(MeshBuffers &mb, Color accent)
{
    Color wall = Color{160, 168, 180, 255};
    Color frame = Fade(accent, 0.8f);
    Color door = Color{80, 180, 255, 200};
    Color roof = shade(accent, 0.55f);
    Color glass = Color{150, 200, 230, 200};

    appendBoxY(mb, Vector3{0.0f, 2.0f, 0.0f}, 7.0f, 3.4f, 8.0f, wall);
    appendBoxY(mb, Vector3{0.0f, 3.72f, 0.0f}, 7.3f, 0.28f, 8.3f,
        shade(wall, 0.82f));

    // Pitched roof: two panels meeting at a ridge, plus gable end caps.
    appendBoxPitchX(
        mb, Vector3{0.0f, 4.25f, 2.3f}, -0.42f, 7.6f, 0.16f, 4.7f, roof);
    appendBoxPitchX(
        mb, Vector3{0.0f, 4.25f, -2.3f}, 0.42f, 7.6f, 0.16f, 4.7f,
        shade(roof, 0.92f));
    appendBoxY(mb, Vector3{0.0f, 5.0f, 0.0f}, 7.65f, 0.2f, 0.5f,
        shade(roof, 1.08f));

    // Roll-up door with visible ribbing.
    appendBoxY(mb, Vector3{0.0f, 1.1f, 4.05f}, 5.0f, 2.6f, 0.15f, door);
    for (int i = 1; i < 5; ++i) {
        float y = 0.05f + static_cast<float>(i) * 0.52f;
        appendBoxY(mb, Vector3{0.0f, y, 4.13f}, 4.9f, 0.05f, 0.03f,
            shade(door, 0.7f));
    }

    // Side windows and corner posts.
    for (float side : {-1.0f, 1.0f}) {
        appendBoxY(mb, Vector3{side * 3.58f, 2.4f, 1.5f}, 0.08f, 0.9f, 1.4f,
            glass);
        appendBoxY(mb, Vector3{side * 3.55f, 2.0f, 0.0f}, 0.12f, 3.6f, 8.2f,
            frame);
    }
    appendBoxY(mb, Vector3{0.0f, 2.0f, -4.05f}, 7.2f, 3.6f, 0.12f, frame);
    appendBoxY(mb, Vector3{0.0f, 0.15f, 0.0f}, 7.2f, 0.3f, 8.2f,
        shade(wall, 0.6f));

    // Corner drainpipe for a lived-in detail.
    appendCylinderY(mb, Vector3{3.6f, 0.0f, -3.9f}, 0.08f, 0.08f, 3.8f, 6,
        shade(wall, 0.6f));

    // Wall-mounted sign above the door, tinted with the region accent.
    appendBoxY(mb, Vector3{0.0f, 3.35f, 4.02f}, 3.0f, 0.5f, 0.08f,
        Fade(accent, 0.92f));
}

void WorldPropBuilder::buildMissionMarker(
    MeshBuffers &mb, Color color, float bob)
{
    Color post = Fade(color, 0.7f);
    appendCylinderY(mb, Vector3{0.0f, 0.0f, 0.0f}, 0.5f, 0.5f, 1.0f, 8, post);
    appendBoxY(
        mb, Vector3{0.0f, 1.8f + bob, 0.0f}, 0.9f, 0.55f, 0.08f,
        color);
    appendSphereApprox(
        mb, Vector3{0.0f, 2.2f + bob, 0.0f}, 0.55f, color);
}

void WorldPropBuilder::buildTrafficCar(MeshBuffers &mb, Color color)
{
    Color body = color;
    Color dark = shade(body, 0.55f);
    Color glass = Color{40, 52, 62, 235};
    Color tire = Color{18, 18, 20, 255};
    Color hub = Color{140, 142, 148, 255};
    Color lightF = Color{255, 244, 200, 255};
    Color lightR = Color{200, 40, 30, 255};

    appendBoxY(mb, Vector3{0.0f, 0.22f, 0.0f}, 1.75f, 0.42f, 3.5f, dark);
    // Hood/trunk deck built as two pitched panels instead of flat boxes,
    // sloping down toward the bumpers for a wedge silhouette.
    appendBoxPitchX(
        mb, Vector3{0.0f, 0.44f, 1.15f}, -0.22f, 1.55f, 0.12f, 1.3f, body);
    appendBoxPitchX(
        mb, Vector3{0.0f, 0.4f, -1.15f}, 0.18f, 1.55f, 0.12f, 1.3f,
        shade(body, 0.9f));
    appendBoxY(mb, Vector3{0.0f, 0.62f, 0.05f}, 1.55f, 0.5f, 2.0f, body);
    appendBoxY(mb, Vector3{0.0f, 0.86f, 0.0f}, 1.35f, 0.42f, 1.65f, glass);
    appendBoxY(mb, Vector3{0.0f, 1.06f, -0.05f}, 1.3f, 0.1f, 1.5f,
        shade(body, 1.05f));

    // Headlights / taillights.
    appendBoxY(mb, Vector3{0.68f, 0.32f, 1.78f}, 0.32f, 0.16f, 0.06f, lightF);
    appendBoxY(mb, Vector3{-0.68f, 0.32f, 1.78f}, 0.32f, 0.16f, 0.06f, lightF);
    appendBoxY(mb, Vector3{0.68f, 0.3f, -1.78f}, 0.3f, 0.16f, 0.06f, lightR);
    appendBoxY(mb, Vector3{-0.68f, 0.3f, -1.78f}, 0.3f, 0.16f, 0.06f, lightR);
    // Side mirrors.
    appendBoxY(mb, Vector3{0.92f, 0.78f, 0.75f}, 0.14f, 0.12f, 0.16f,
        shade(body, 0.85f));
    appendBoxY(mb, Vector3{-0.92f, 0.78f, 0.75f}, 0.14f, 0.12f, 0.16f,
        shade(body, 0.85f));

    // Four wheels: a dark tire cylinder oriented with its axle along X
    // (so the round face reads correctly from the side) plus a small
    // hubcap cap on the outward face.
    const float wheelX[4] = {1.0f, -1.0f, 1.0f, -1.0f};
    const float wheelZ[4] = {1.25f, 1.25f, -1.25f, -1.25f};
    for (int i = 0; i < 4; ++i) {
        float side = wheelX[i] > 0.0f ? 1.0f : -1.0f;
        appendCylinderX(
            mb, Vector3{wheelX[i] - 0.14f, 0.42f, wheelZ[i]}, 0.42f, 0.28f,
            10, tire);
        appendCylinderX(
            mb, Vector3{wheelX[i] + side * 0.14f, 0.42f, wheelZ[i]}, 0.2f,
            side * 0.03f, 8, hub);
    }
}

void WorldPropBuilder::buildObservatoryTower(MeshBuffers &mb, Color accent)
{
    Color wall = Color{92, 86, 84, 255};
    Color dome = mulColor(Color{215, 210, 200, 255}, accent);
    float towerH = 7.5f;

    appendCylinderY(mb, Vector3{0.0f, 0.0f, 0.0f}, 2.0f, 1.6f, towerH, 12,
        wall);
    appendCylinderY(
        mb, Vector3{0.0f, towerH, 0.0f}, 1.7f, 1.7f, 0.4f, 12, shade(wall, 1.1f));
    appendSphereApprox(
        mb, Vector3{0.0f, towerH + 1.6f, 0.0f}, 1.9f, Fade(dome, 0.92f));
    appendCylinderY(
        mb, Vector3{0.0f, towerH + 0.4f, 0.0f}, 0.25f, 0.25f, 2.6f, 6,
        shade(dome, 0.7f));
}

void WorldPropBuilder::buildWatchtower(MeshBuffers &mb, Color accent)
{
    Color wood = accent;
    Color roof = shade(accent, 0.7f);
    float legH = 4.5f;
    float platformY = legH;

    const float legX[4] = {-1.6f, 1.6f, 1.6f, -1.6f};
    const float legZ[4] = {-1.6f, -1.6f, 1.6f, 1.6f};
    for (int i = 0; i < 4; ++i) {
        appendCylinderY(
            mb, Vector3{legX[i], 0.0f, legZ[i]}, 0.22f, 0.22f, legH, 6, wood);
    }
    appendBoxY(
        mb, Vector3{0.0f, platformY + 0.2f, 0.0f}, 3.6f, 0.4f, 3.6f,
        shade(wood, 0.85f));
    appendBoxY(
        mb, Vector3{0.0f, platformY + 1.6f, 0.0f}, 3.0f, 2.4f, 3.0f,
        shade(wood, 1.05f));
    appendBoxY(
        mb, Vector3{0.0f, platformY + 3.1f, 0.0f}, 3.6f, 0.35f, 3.6f, roof);
}

Model WorldPropBuilder::finalizeModel(const MeshBuffers &scratch) const
{
    if (scratch.vertices.empty()) {
        return Model{};
    }
    Mesh mesh = TrackMeshBuilder::meshFromBuffers(scratch);
    if (mesh.vertexCount <= 0 || mesh.triangleCount <= 0) {
        return Model{};
    }
    Model model = LoadModelFromMesh(mesh);
    if (hasShader_) {
        TrackMeshBuilder::applyShaderToModel(model, shader_);
    }
    return model;
}

Model WorldPropBuilder::buildScatterModel(
    const std::vector<PropInstance> &props, Vector3 chunkOrigin,
    const std::vector<float> &groundY) const
{
    MeshBuffers scratch{};

    for (size_t i = 0; i < props.size(); ++i) {
        const PropInstance &prop = props[i];
        float wx = chunkOrigin.x + prop.localX;
        float wz = chunkOrigin.z + prop.localZ;
        uint32_t h = hashPosition(wx, wz);
        PropVariation var = variationFromHash(h, WHITE);
        float finalScale = prop.scale * var.scaleMul;
        MeshBuffers geom{};

        switch (prop.type) {
        case 1:
            if (var.subVariant == 0) {
                buildBroadleaf(geom, 1.0f, var.heightScale, var.tint);
            } else {
                buildPine(geom, 1.0f, var.heightScale, var.tint);
            }
            break;
        case 2:
            if (var.subVariant == 0) {
                buildIndustrialSilo(geom, 1.0f, var.heightScale, var.tint);
            } else if (var.subVariant == 1) {
                buildIndustrialTank(geom, 1.0f, var.heightScale, var.tint);
            } else {
                buildIndustrialAnnex(geom, 1.0f, var.heightScale, var.tint);
            }
            break;
        case 3:
            buildRock(geom, 1.0f, var.heightScale, var.tint);
            break;
        default:
            buildPalm(geom, 1.0f, var.heightScale, var.tint);
            break;
        }

        appendTransformed(
            scratch, geom, Vector3{wx, groundY[i], wz}, prop.yaw, finalScale,
            var.heightScale, WHITE);
    }

    return finalizeModel(scratch);
}

Model WorldPropBuilder::buildMarinaLandmarksModel(float groundY) const
{
    MeshBuffers scratch{};

    Vector3 origin{24.0f, groundY, 18.0f};
    MeshBuffers building{};
    buildCoastalBuilding(building, 1.0f, WHITE);
    appendTransformed(scratch, building, origin, 0.0f, 1.0f, 1.0f, WHITE);

    Color wood{120, 100, 80, 255};
    Color post{90, 75, 60, 255};
    for (int i = 0; i < 5; ++i) {
        float px = 10.0f + static_cast<float>(i) * 5.5f;
        MeshBuffers pier{};
        buildPierSegment(pier, 4.0f, wood, post);
        appendTransformed(
            scratch, pier, Vector3{px, groundY, 28.0f}, 0.0f, 1.0f, 1.0f,
            WHITE);
    }

    MeshBuffers water{};
    buildWaterPlane(water, 14.0f, 7.0f, Fade(SKYBLUE, 0.48f));
    appendTransformed(
        scratch, water, Vector3{22.0f, groundY - 0.15f, 40.0f}, 0.0f, 1.0f,
        1.0f, WHITE);

    return finalizeModel(scratch);
}

Model WorldPropBuilder::buildPortLandmarksModel(float groundY) const
{
    MeshBuffers scratch{};
    Color tint = WHITE;

    MeshBuffers silo{};
    buildIndustrialSilo(silo, 1.8f, 1.2f, tint);
    appendTransformed(
        scratch, silo, Vector3{0.0f, groundY, 0.0f}, 0.3f, 1.0f, 1.0f, WHITE);

    MeshBuffers silo2{};
    buildIndustrialSilo(silo2, 1.6f, 1.0f, tint);
    appendTransformed(
        scratch, silo2, Vector3{5.5f, groundY, -2.0f}, -0.4f, 1.0f, 1.0f,
        WHITE);

    MeshBuffers tank{};
    buildIndustrialTank(tank, 1.5f, 1.0f, tint);
    appendTransformed(
        scratch, tank, Vector3{-6.0f, groundY, 3.0f}, 0.0f, 1.0f, 1.0f,
        WHITE);

    MeshBuffers annex{};
    buildIndustrialAnnex(annex, 1.6f, 1.3f, tint);
    appendTransformed(
        scratch, annex, Vector3{0.0f, groundY, 9.0f}, 0.0f, 1.0f, 1.0f,
        WHITE);

    return finalizeModel(scratch);
}

Model WorldPropBuilder::buildVolcanoLandmarksModel(float groundY) const
{
    MeshBuffers scratch{};

    MeshBuffers tower{};
    buildObservatoryTower(tower, Color{200, 90, 60, 255});
    appendTransformed(
        scratch, tower, Vector3{0.0f, groundY, 0.0f}, 0.0f, 1.0f, 1.0f,
        WHITE);

    MeshBuffers rock{};
    buildRock(rock, 1.6f, 1.1f, Color{110, 90, 82, 255});
    appendTransformed(
        scratch, rock, Vector3{6.5f, groundY, 4.0f}, 0.6f, 1.0f, 1.0f, WHITE);

    return finalizeModel(scratch);
}

Model WorldPropBuilder::buildForestLandmarksModel(float groundY) const
{
    MeshBuffers scratch{};

    MeshBuffers tower{};
    buildWatchtower(tower, Color{110, 82, 56, 255});
    appendTransformed(
        scratch, tower, Vector3{0.0f, groundY, 0.0f}, 0.0f, 1.0f, 1.0f,
        WHITE);

    MeshBuffers pine{};
    buildPine(pine, 1.8f, 1.4f, Color{200, 210, 200, 255});
    appendTransformed(
        scratch, pine, Vector3{5.5f, groundY, -3.5f}, 0.4f, 1.0f, 1.0f,
        WHITE);

    MeshBuffers broadleaf{};
    buildBroadleaf(broadleaf, 1.6f, 1.3f, Color{200, 210, 200, 255});
    appendTransformed(
        scratch, broadleaf, Vector3{-5.0f, groundY, 4.0f}, -0.3f, 1.0f, 1.0f,
        WHITE);

    return finalizeModel(scratch);
}

void WorldPropBuilder::placeRaceGate(
    const PoiInstance &poi, float groundY, float timeSec)
{
    float pulse = 0.85f + 0.15f * std::sin(timeSec * 4.0f);
    MeshBuffers gate{};
    buildRaceGate(
        gate, poi.color, Fade(poi.color, 0.65f), pulse);
    appendToBatch(
        BatchId::POI, gate, Vector3{poi.worldX, groundY, poi.worldZ},
        0.0f, 1.0f, 1.0f, WHITE);
}

void WorldPropBuilder::placeGarage(const PoiInstance &poi, float groundY)
{
    MeshBuffers garage{};
    buildGarage(garage, poi.color);
    appendToBatch(
        BatchId::POI, garage, Vector3{poi.worldX, groundY, poi.worldZ},
        0.0f, 1.0f, 1.0f, WHITE);
}

void WorldPropBuilder::placeMissionMarker(
    const PoiInstance &poi, float groundY, float timeSec)
{
    float bob = std::sin(timeSec * 3.0f) * 0.25f;
    MeshBuffers marker{};
    buildMissionMarker(marker, poi.color, bob);
    appendToBatch(
        BatchId::POI, marker, Vector3{poi.worldX, groundY, poi.worldZ},
        0.0f, 1.0f, 1.0f, WHITE);
}

void WorldPropBuilder::placeTrafficCar(
    Vector3 pos, float heading, Color color)
{
    MeshBuffers car{};
    buildTrafficCar(car, color);
    appendToBatch(
        BatchId::TRAFFIC, car, pos, heading, 1.0f, 1.0f, WHITE);
}

} // namespace racer::world
