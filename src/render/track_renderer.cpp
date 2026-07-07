#include "render/track_renderer.h"

#include <cmath>
#include <cstdint>
#include <vector>

#include "rlgl.h"

namespace racer {

namespace {

std::vector<Vector2> ComputePerpendiculars(const Track& track) {
    const auto& wp = track.Waypoints();
    size_t n = wp.size();
    std::vector<Vector2> perp(n);

    for (size_t i = 0; i < n; ++i) {
        Vector2 prev = wp[(i + n - 1) % n];
        Vector2 cur = wp[i];
        Vector2 next = wp[(i + 1) % n];

        Vector2 dirA{cur.x - prev.x, cur.y - prev.y};
        Vector2 dirB{next.x - cur.x, next.y - cur.y};
        float lenA = std::sqrt(dirA.x * dirA.x + dirA.y * dirA.y);
        float lenB = std::sqrt(dirB.x * dirB.x + dirB.y * dirB.y);
        if (lenA > 1e-6f) { dirA.x /= lenA; dirA.y /= lenA; }
        if (lenB > 1e-6f) { dirB.x /= lenB; dirB.y /= lenB; }

        Vector2 avg{dirA.x + dirB.x, dirA.y + dirB.y};
        float avgLen = std::sqrt(avg.x * avg.x + avg.y * avg.y);
        if (avgLen > 1e-6f) { avg.x /= avgLen; avg.y /= avgLen; }

        perp[i] = Vector2{-avg.y, avg.x};
    }
    return perp;
}

// Mesh generique "bande le long des waypoints" -- utilise pour la chaussee
// et pour les bordures (avec une largeur/decalage/couleurs differents).
Mesh BuildStripMesh(const Track& track, const std::vector<Vector2>& perp, float innerOffset, float outerOffset,
                    float yHeight, const Color& colorA, const Color& colorB, int colorPeriod) {
    const auto& wp = track.Waypoints();
    size_t n = wp.size();

    std::vector<Vector3> inner(n), outer(n);
    for (size_t i = 0; i < n; ++i) {
        inner[i] = Vector3{wp[i].x + perp[i].x * innerOffset, yHeight, wp[i].y + perp[i].y * innerOffset};
        outer[i] = Vector3{wp[i].x + perp[i].x * outerOffset, yHeight, wp[i].y + perp[i].y * outerOffset};
    }

    Mesh mesh{};
    mesh.vertexCount = static_cast<int>(n) * 4;
    mesh.triangleCount = static_cast<int>(n) * 2;

    mesh.vertices = static_cast<float*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.normals = static_cast<float*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.colors = static_cast<unsigned char*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount) * 4 * sizeof(unsigned char)));
    mesh.indices = static_cast<unsigned short*>(MemAlloc(static_cast<unsigned int>(mesh.triangleCount) * 3 * sizeof(unsigned short)));

    for (size_t i = 0; i < n; ++i) {
        size_t j = (i + 1) % n;
        Vector3 quad[4] = {inner[i], inner[j], outer[j], outer[i]};
        Color c = (static_cast<int>(i / static_cast<size_t>(colorPeriod)) % 2 == 0) ? colorA : colorB;

        for (int k = 0; k < 4; ++k) {
            size_t vIdx = i * 4 + static_cast<size_t>(k);
            mesh.vertices[vIdx * 3 + 0] = quad[k].x;
            mesh.vertices[vIdx * 3 + 1] = quad[k].y;
            mesh.vertices[vIdx * 3 + 2] = quad[k].z;
            mesh.normals[vIdx * 3 + 0] = 0.0f;
            mesh.normals[vIdx * 3 + 1] = 1.0f;
            mesh.normals[vIdx * 3 + 2] = 0.0f;
            mesh.colors[vIdx * 4 + 0] = c.r;
            mesh.colors[vIdx * 4 + 1] = c.g;
            mesh.colors[vIdx * 4 + 2] = c.b;
            mesh.colors[vIdx * 4 + 3] = c.a;
        }

        unsigned short base = static_cast<unsigned short>(i * 4);
        size_t idx = i * 6;
        mesh.indices[idx + 0] = base + 0;
        mesh.indices[idx + 1] = base + 1;
        mesh.indices[idx + 2] = base + 2;
        mesh.indices[idx + 3] = base + 0;
        mesh.indices[idx + 4] = base + 2;
        mesh.indices[idx + 5] = base + 3;
    }

    UploadMesh(&mesh, false);
    return mesh;
}

Mesh BuildCheckerGroundMesh(float totalSize, int tilesPerSide) {
    float tileSize = totalSize / static_cast<float>(tilesPerSide);
    float half = totalSize * 0.5f;

    Mesh mesh{};
    mesh.vertexCount = tilesPerSide * tilesPerSide * 4;
    mesh.triangleCount = tilesPerSide * tilesPerSide * 2;
    mesh.vertices = static_cast<float*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.normals = static_cast<float*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.colors = static_cast<unsigned char*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount) * 4 * sizeof(unsigned char)));
    mesh.indices = static_cast<unsigned short*>(MemAlloc(static_cast<unsigned int>(mesh.triangleCount) * 3 * sizeof(unsigned short)));

    Color greenA{58, 130, 58, 255};
    Color greenB{50, 118, 50, 255};

    int quadIdx = 0;
    for (int tz = 0; tz < tilesPerSide; ++tz) {
        for (int tx = 0; tx < tilesPerSide; ++tx) {
            float x0 = -half + static_cast<float>(tx) * tileSize;
            float z0 = -half + static_cast<float>(tz) * tileSize;
            Vector3 quad[4] = {
                {x0, 0.0f, z0},
                {x0, 0.0f, z0 + tileSize},
                {x0 + tileSize, 0.0f, z0 + tileSize},
                {x0 + tileSize, 0.0f, z0},
            };
            Color c = ((tx + tz) % 2 == 0) ? greenA : greenB;

            for (int k = 0; k < 4; ++k) {
                size_t vIdx = static_cast<size_t>(quadIdx) * 4 + static_cast<size_t>(k);
                mesh.vertices[vIdx * 3 + 0] = quad[k].x;
                mesh.vertices[vIdx * 3 + 1] = quad[k].y;
                mesh.vertices[vIdx * 3 + 2] = quad[k].z;
                mesh.normals[vIdx * 3 + 0] = 0.0f;
                mesh.normals[vIdx * 3 + 1] = 1.0f;
                mesh.normals[vIdx * 3 + 2] = 0.0f;
                mesh.colors[vIdx * 4 + 0] = c.r;
                mesh.colors[vIdx * 4 + 1] = c.g;
                mesh.colors[vIdx * 4 + 2] = c.b;
                mesh.colors[vIdx * 4 + 3] = c.a;
            }
            unsigned short base = static_cast<unsigned short>(quadIdx * 4);
            size_t idx = static_cast<size_t>(quadIdx) * 6;
            mesh.indices[idx + 0] = base + 0;
            mesh.indices[idx + 1] = base + 1;
            mesh.indices[idx + 2] = base + 2;
            mesh.indices[idx + 3] = base + 0;
            mesh.indices[idx + 4] = base + 2;
            mesh.indices[idx + 5] = base + 3;
            ++quadIdx;
        }
    }

    UploadMesh(&mesh, false);
    return mesh;
}

Mesh BuildFinishLineMesh(const Track& track, const std::vector<Vector2>& perp) {
    const auto& wp = track.Waypoints();
    Vector2 base = wp[0];
    Vector2 p = perp[0];
    Vector2 dir{-p.y, p.x}; // direction le long de la piste au point 0
    float halfWidth = track.Width() * 0.5f;

    constexpr int kCols = 6;
    constexpr float kLineDepth = 2.5f;

    Mesh mesh{};
    mesh.vertexCount = kCols * 4;
    mesh.triangleCount = kCols * 2;
    mesh.vertices = static_cast<float*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.normals = static_cast<float*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.colors = static_cast<unsigned char*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount) * 4 * sizeof(unsigned char)));
    mesh.indices = static_cast<unsigned short*>(MemAlloc(static_cast<unsigned int>(mesh.triangleCount) * 3 * sizeof(unsigned short)));

    for (int c = 0; c < kCols; ++c) {
        float t0 = -halfWidth + (static_cast<float>(c) / kCols) * (2.0f * halfWidth);
        float t1 = -halfWidth + (static_cast<float>(c + 1) / kCols) * (2.0f * halfWidth);

        Vector3 quad[4] = {
            {base.x + p.x * t0 - dir.x * kLineDepth * 0.5f, 0.03f, base.y + p.y * t0 - dir.y * kLineDepth * 0.5f},
            {base.x + p.x * t0 + dir.x * kLineDepth * 0.5f, 0.03f, base.y + p.y * t0 + dir.y * kLineDepth * 0.5f},
            {base.x + p.x * t1 + dir.x * kLineDepth * 0.5f, 0.03f, base.y + p.y * t1 + dir.y * kLineDepth * 0.5f},
            {base.x + p.x * t1 - dir.x * kLineDepth * 0.5f, 0.03f, base.y + p.y * t1 - dir.y * kLineDepth * 0.5f},
        };
        Color color = (c % 2 == 0) ? WHITE : BLACK;

        for (int k = 0; k < 4; ++k) {
            size_t vIdx = static_cast<size_t>(c) * 4 + static_cast<size_t>(k);
            mesh.vertices[vIdx * 3 + 0] = quad[k].x;
            mesh.vertices[vIdx * 3 + 1] = quad[k].y;
            mesh.vertices[vIdx * 3 + 2] = quad[k].z;
            mesh.normals[vIdx * 3 + 0] = 0.0f;
            mesh.normals[vIdx * 3 + 1] = 1.0f;
            mesh.normals[vIdx * 3 + 2] = 0.0f;
            mesh.colors[vIdx * 4 + 0] = color.r;
            mesh.colors[vIdx * 4 + 1] = color.g;
            mesh.colors[vIdx * 4 + 2] = color.b;
            mesh.colors[vIdx * 4 + 3] = color.a;
        }
        unsigned short base16 = static_cast<unsigned short>(c * 4);
        size_t idx = static_cast<size_t>(c) * 6;
        mesh.indices[idx + 0] = base16 + 0;
        mesh.indices[idx + 1] = base16 + 1;
        mesh.indices[idx + 2] = base16 + 2;
        mesh.indices[idx + 3] = base16 + 0;
        mesh.indices[idx + 4] = base16 + 2;
        mesh.indices[idx + 5] = base16 + 3;
    }

    UploadMesh(&mesh, false);
    return mesh;
}

uint32_t HashIndex(size_t i) {
    uint32_t h = static_cast<uint32_t>(i) * 2654435761u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}

} // namespace

TrackRenderer::TrackRenderer(const Track& track) {
    std::vector<Vector2> perp = ComputePerpendiculars(track);
    float halfWidth = track.Width() * 0.5f;

    Mesh trackMesh = BuildStripMesh(track, perp, -halfWidth, halfWidth, 0.02f, Color{58, 58, 62, 255},
                                     Color{70, 70, 74, 255}, 2);
    trackModel_ = LoadModelFromMesh(trackMesh);

    // Bordures (rumble strips) rouge/blanc a cheval sur les deux bords de la piste.
    constexpr float kCurbWidth = 1.4f;
    Mesh curbMeshOuter = BuildStripMesh(track, perp, halfWidth - kCurbWidth * 0.5f, halfWidth + kCurbWidth * 0.5f,
                                        0.025f, RED, RAYWHITE, 3);
    curbModelOuter_ = LoadModelFromMesh(curbMeshOuter);
    Mesh curbMeshInner = BuildStripMesh(track, perp, -halfWidth - kCurbWidth * 0.5f, -halfWidth + kCurbWidth * 0.5f,
                                        0.025f, RED, RAYWHITE, 3);
    curbModelInner_ = LoadModelFromMesh(curbMeshInner);

    Mesh groundMesh = BuildCheckerGroundMesh(500.0f, 40);
    groundModel_ = LoadModelFromMesh(groundMesh);

    Mesh finishMesh = BuildFinishLineMesh(track, perp);
    finishLineModel_ = LoadModelFromMesh(finishMesh);

    // Decor : arbres et immeubles places en alternance de part et d'autre de
    // la piste, avec une legere variation deterministe par index.
    const auto& wp = track.Waypoints();
    constexpr int kStride = 3;
    for (size_t i = 0; i < wp.size(); i += static_cast<size_t>(kStride)) {
        uint32_t h = HashIndex(i);
        float sideSign = (h % 2 == 0) ? 1.0f : -1.0f;
        float extraOffset = 4.0f + static_cast<float>(h % 100) * 0.05f; // 4..9 au-dela du bord
        float dist = halfWidth + extraOffset;

        Vector3 pos{
            wp[i].x + perp[i].x * dist * sideSign,
            0.0f,
            wp[i].y + perp[i].y * dist * sideSign,
        };

        bool isBuilding = (h % 5 == 0);
        PropInstance prop;
        prop.position = pos;
        prop.type = isBuilding ? 1 : 0;
        prop.heightScale = isBuilding ? (1.0f + static_cast<float>(h % 100) * 0.03f) : (0.8f + static_cast<float>(h % 100) * 0.006f);
        prop.color = isBuilding ? Color{static_cast<unsigned char>(90 + h % 60), static_cast<unsigned char>(90 + h % 60),
                                        static_cast<unsigned char>(100 + h % 60), 255}
                                : Color{34, static_cast<unsigned char>(110 + h % 50), 34, 255};
        props_.push_back(prop);
    }
}

TrackRenderer::~TrackRenderer() {
    UnloadModel(trackModel_);
    UnloadModel(curbModelOuter_);
    UnloadModel(curbModelInner_);
    UnloadModel(groundModel_);
    UnloadModel(finishLineModel_);
}

void TrackRenderer::Draw() const {
    DrawModel(groundModel_, Vector3{0.0f, -0.05f, 0.0f}, 1.0f, WHITE);
    DrawModel(trackModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    DrawModel(curbModelOuter_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    DrawModel(curbModelInner_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    DrawModel(finishLineModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);

    for (const auto& prop : props_) {
        if (prop.type == 1) {
            float w = 2.5f;
            float h = 5.0f * prop.heightScale;
            DrawCube(Vector3{prop.position.x, h * 0.5f, prop.position.z}, w, h, w, prop.color);
            DrawCubeWires(Vector3{prop.position.x, h * 0.5f, prop.position.z}, w, h, w, Fade(BLACK, 0.4f));
        } else {
            float trunkH = 1.6f * prop.heightScale;
            DrawCylinder(Vector3{prop.position.x, 0.0f, prop.position.z}, 0.25f, 0.3f, trunkH, 6, Color{92, 64, 40, 255});
            DrawCylinder(Vector3{prop.position.x, trunkH, prop.position.z}, 1.6f * prop.heightScale, 0.0f,
                         2.2f * prop.heightScale, 8, prop.color);
        }
    }
}

} // namespace racer
