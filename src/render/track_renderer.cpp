#include "render/track_renderer.h"

#include <cmath>
#include <vector>

namespace racer {

namespace {

Mesh BuildTrackStripMesh(const Track& track) {
    const auto& wp = track.Waypoints();
    size_t n = wp.size();
    float halfWidth = track.Width() * 0.5f;

    std::vector<Vector3> left(n), right(n);

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

        Vector2 perp{-avg.y, avg.x};
        left[i] = Vector3{cur.x + perp.x * halfWidth, 0.02f, cur.y + perp.y * halfWidth};
        right[i] = Vector3{cur.x - perp.x * halfWidth, 0.02f, cur.y - perp.y * halfWidth};
    }

    Mesh mesh{};
    mesh.vertexCount = static_cast<int>(n) * 4; // 4 sommets par segment (pas de partage, couleurs simples)
    mesh.triangleCount = static_cast<int>(n) * 2;

    mesh.vertices = static_cast<float*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.normals = static_cast<float*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.colors = static_cast<unsigned char*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount) * 4 * sizeof(unsigned char)));
    mesh.indices = static_cast<unsigned short*>(MemAlloc(static_cast<unsigned int>(mesh.triangleCount) * 3 * sizeof(unsigned short)));

    Color asphalt{58, 58, 62, 255};
    Color asphaltAlt{70, 70, 74, 255}; // alterne legerement pour distinguer les segments a l'oeil

    for (size_t i = 0; i < n; ++i) {
        size_t j = (i + 1) % n;
        Vector3 quad[4] = {left[i], left[j], right[j], right[i]};
        Color c = (i % 2 == 0) ? asphalt : asphaltAlt;

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

} // namespace

TrackRenderer::TrackRenderer(const Track& track) {
    Mesh trackMesh = BuildTrackStripMesh(track);
    trackModel_ = LoadModelFromMesh(trackMesh);

    Mesh groundMesh = GenMeshPlane(400.0f, 400.0f, 2, 2);
    groundModel_ = LoadModelFromMesh(groundMesh);
    groundModel_.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = Color{58, 130, 58, 255};
}

TrackRenderer::~TrackRenderer() {
    UnloadModel(trackModel_);
    UnloadModel(groundModel_);
}

void TrackRenderer::Draw() const {
    DrawModel(groundModel_, Vector3{0.0f, -0.05f, 0.0f}, 1.0f, WHITE);
    DrawModel(trackModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
}

} // namespace racer
