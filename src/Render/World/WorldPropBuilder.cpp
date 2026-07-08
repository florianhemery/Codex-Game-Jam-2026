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
    UnloadMesh(mesh);
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
    appendCylinderY(
        mb, Vector3{0.0f, trunkH, 0.0f}, 1.5f * scale, 0.2f * scale,
        2.4f * scale * hs, 8, foliage);
    appendSphereApprox(
        mb,
        Vector3{0.0f, trunkH + 2.9f * scale * hs, 0.0f},
        0.85f * scale, Fade(foliage, 0.85f));
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
    appendSphereApprox(
        mb,
        Vector3{0.0f, trunkH + 1.4f * scale * hs, 0.0f},
        1.35f * scale, foliage);
    appendSphereApprox(
        mb,
        Vector3{0.55f * scale, trunkH + 1.0f * scale * hs, 0.2f * scale},
        0.9f * scale, shade(foliage, 0.95f));
}

void WorldPropBuilder::buildPalm(
    MeshBuffers &mb, float scale, float heightScale, Color tint)
{
    float hs = heightScale;
    Color trunk = mulColor(Color{110, 78, 48, 255}, tint);
    Color frond = mulColor(Color{48, 130, 52, 255}, tint);
    float trunkH = 2.2f * scale * hs;

    appendCylinderY(
        mb, Vector3{0.0f, 0.0f, 0.0f}, 0.18f * scale, 0.22f * scale,
        trunkH, 5, trunk);
    Vector3 crown{0.0f, trunkH, 0.0f};
    for (int i = 0; i < 5; ++i) {
        float a = static_cast<float>(i) / 5.0f * kPi * 2.0f;
        appendFrond(mb, crown, a, 1.4f, scale, frond);
    }
}

void WorldPropBuilder::buildRock(
    MeshBuffers &mb, float scale, float heightScale, Color tint)
{
    Color rock = mulColor(Color{68, 62, 58, 255}, tint);
    appendBoxY(
        mb, Vector3{0.0f, 0.8f * scale * heightScale, 0.0f},
        2.2f * scale, 1.6f * scale * heightScale, 2.0f * scale, rock);
    appendBoxY(
        mb,
        Vector3{0.35f * scale, 1.35f * scale * heightScale, -0.2f * scale},
        1.2f * scale, 0.8f * scale, 1.0f * scale, shade(rock, 1.06f));
}

void WorldPropBuilder::buildIndustrialSilo(
    MeshBuffers &mb, float scale, float heightScale, Color tint)
{
    Color silo = mulColor(Color{140, 142, 148, 255}, tint);
    Color annex = mulColor(Color{90, 92, 98, 255}, tint);
    float siloH = 5.5f * scale * heightScale;

    appendCylinderY(
        mb, Vector3{0.0f, 0.0f, 0.0f}, 1.2f * scale, 1.2f * scale,
        siloH, 10, silo);
    appendBoxY(
        mb,
        Vector3{2.0f * scale, 1.5f * scale * heightScale, 0.0f},
        2.5f * scale, 2.0f * scale * heightScale, 3.0f * scale, annex);
    appendBoxY(
        mb,
        Vector3{0.0f, siloH + 0.25f * scale, 0.0f},
        0.5f * scale, 0.5f * scale, 0.5f * scale, shade(silo, 0.75f));
}

void WorldPropBuilder::buildIndustrialTank(
    MeshBuffers &mb, float scale, float heightScale, Color tint)
{
    Color tank = mulColor(Color{118, 122, 130, 255}, tint);
    Color pipe = mulColor(Color{72, 74, 80, 255}, tint);
    float tankR = 1.6f * scale;

    appendCylinderY(
        mb, Vector3{0.0f, 0.6f * scale, 0.0f}, tankR, tankR,
        2.2f * scale * heightScale, 10, tank);
    appendBoxY(
        mb,
        Vector3{0.0f, 0.25f * scale, 0.0f},
        tankR * 2.2f, 0.5f * scale, tankR * 2.0f, shade(tank, 0.9f));
    appendCylinderY(
        mb,
        Vector3{tankR + 0.3f * scale, 1.2f * scale * heightScale, 0.0f},
        0.15f * scale, 0.15f * scale, 2.8f * scale * heightScale, 6, pipe);
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

    appendBoxY(mb, Vector3{0.0f, 2.5f * scale, 0.0f}, 8.0f * scale, 5.0f * scale, 6.0f * scale, wall);
    appendBoxY(mb, Vector3{0.0f, 5.8f * scale, 0.0f}, 7.0f * scale, 1.2f * scale, 5.0f * scale, roof);
    for (int i = 0; i < 4; ++i) {
        appendBoxY(
            mb,
            Vector3{
                (-2.4f + static_cast<float>(i) * 1.6f) * scale,
                2.2f * scale, 3.05f * scale,
            },
            0.6f * scale, 0.9f * scale, 0.08f * scale,
            Color{120, 180, 220, 210});
    }
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

    appendBoxY(mb, Vector3{0.0f, 2.0f, 0.0f}, 7.0f, 4.0f, 8.0f, wall);
    appendBoxY(mb, Vector3{0.0f, 4.15f, 0.0f}, 7.4f, 0.35f, 8.4f, shade(wall, 0.82f));
    appendBoxY(mb, Vector3{0.0f, 1.2f, 4.05f}, 5.0f, 3.0f, 0.15f, door);
    appendBoxY(mb, Vector3{-3.55f, 2.0f, 0.0f}, 0.12f, 4.2f, 8.2f, frame);
    appendBoxY(mb, Vector3{3.55f, 2.0f, 0.0f}, 0.12f, 4.2f, 8.2f, frame);
    appendBoxY(mb, Vector3{0.0f, 2.0f, -4.05f}, 7.2f, 4.2f, 0.12f, frame);
    appendBoxY(mb, Vector3{0.0f, 0.35f, 0.0f}, 6.2f, 0.25f, 7.2f, shade(wall, 0.7f));
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

void WorldPropBuilder::buildTrafficCar(MeshBuffers &mb, Color body)
{
    Color dark = shade(body, 0.55f);
    Color cabin = shade(body, 0.92f);

    appendBoxY(mb, Vector3{0.0f, 0.0f, 0.0f}, 1.8f, 0.5f, 3.6f, dark);
    appendBoxY(mb, Vector3{0.0f, 0.35f, 0.15f}, 1.6f, 0.55f, 2.2f, cabin);
    appendBoxY(mb, Vector3{0.0f, 0.28f, 1.55f}, 1.3f, 0.35f, 0.7f, body);
    appendBoxY(mb, Vector3{0.0f, 0.22f, -1.55f}, 1.5f, 0.3f, 0.5f, shade(body, 0.78f));
}

void WorldPropBuilder::placeScatterProp(
    std::uint8_t type, Vector3 base, float yaw, float scale, BiomeId biome)
{
    uint32_t h = hashPosition(base.x, base.z);
    PropVariation var = variationFromHash(h, WHITE);
    float finalScale = scale * var.scaleMul;
    MeshBuffers geom{};

    switch (type) {
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
    (void)biome;
    appendToBatch(
        BatchId::SCATTER, geom, base, yaw, finalScale, var.heightScale,
        WHITE);
}

void WorldPropBuilder::placeMarinaLandmarks(float groundY)
{
    Vector3 origin{24.0f, groundY, 18.0f};
    MeshBuffers building{};
    buildCoastalBuilding(building, 1.0f, WHITE);
    appendToBatch(BatchId::LANDMARK, building, origin, 0.0f, 1.0f, 1.0f, WHITE);

    Color wood{120, 100, 80, 255};
    Color post{90, 75, 60, 255};
    for (int i = 0; i < 5; ++i) {
        float px = 10.0f + static_cast<float>(i) * 5.5f;
        MeshBuffers pier{};
        buildPierSegment(pier, 4.0f, wood, post);
        appendToBatch(
            BatchId::LANDMARK, pier, Vector3{px, groundY, 28.0f}, 0.0f,
            1.0f, 1.0f, WHITE);
    }

    MeshBuffers water{};
    buildWaterPlane(water, 14.0f, 7.0f, Fade(SKYBLUE, 0.48f));
    appendToBatch(
        BatchId::LANDMARK, water, Vector3{22.0f, groundY - 0.15f, 40.0f}, 0.0f,
        1.0f, 1.0f, WHITE);
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
