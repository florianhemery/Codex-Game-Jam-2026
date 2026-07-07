#include "render/track_renderer.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
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

uint32_t HashIndex(size_t i) {
    uint32_t h = static_cast<uint32_t>(i) * 2654435761u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}

Color LerpColor(Color a, Color b, float t) {
    auto lerp = [&](unsigned char x, unsigned char y) {
        return static_cast<unsigned char>(static_cast<float>(x) + (static_cast<float>(y) - static_cast<float>(x)) * t);
    };
    return Color{lerp(a.r, b.r), lerp(a.g, b.g), lerp(a.b, b.b), lerp(a.a, b.a)};
}

Color AsphaltColor(uint32_t h, SurfaceStyle style) {
    int noise = static_cast<int>(h % 37);
    if (style == SurfaceStyle::Abimee) {
        unsigned char base = static_cast<unsigned char>(95 + noise);
        return Color{base, static_cast<unsigned char>(base - 4), static_cast<unsigned char>(base - 8), 255};
    }
    // Gris bleute volontairement peu vert pour contraster avec l'herbe.
    unsigned char r = static_cast<unsigned char>(102 + noise % 18);
    unsigned char g = static_cast<unsigned char>(98 + noise % 14);
    unsigned char b = static_cast<unsigned char>(112 + noise % 16);
    return Color{r, g, b, 255};
}

// Ruban de route segment par segment : une seule perpendiculaire par segment.
// Avec des chicanes serrees, melanger perp[i] et perp[j] croise les bords
// interieurs/exterieurs et le ruban se retourne (route invisible).
Mesh BuildStripMesh(const Track& track, float innerOffset, float outerOffset, float yHeight,
                    const std::function<Color(size_t)>& colorFn) {
    const auto& wp = track.Waypoints();
    size_t n = wp.size();

    Mesh mesh{};
    mesh.vertexCount = static_cast<int>(n) * 4;
    mesh.triangleCount = static_cast<int>(n) * 2;
    mesh.vertices = static_cast<float*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.normals = static_cast<float*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.colors = static_cast<unsigned char*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount) * 4 * sizeof(unsigned char)));
    mesh.indices = static_cast<unsigned short*>(MemAlloc(static_cast<unsigned int>(mesh.triangleCount) * 3 * sizeof(unsigned short)));

    for (size_t i = 0; i < n; ++i) {
        size_t j = (i + 1) % n;
        Vector2 a = wp[i];
        Vector2 b = wp[j];
        Vector2 ab{b.x - a.x, b.y - a.y};
        float segLen = std::sqrt(ab.x * ab.x + ab.y * ab.y);
        if (segLen < 1e-5f) continue;
        Vector2 segPerp{-ab.y / segLen, ab.x / segLen};
        float segY = yHeight + static_cast<float>(i) * 0.004f;

        Vector3 quad[4] = {
            {a.x + segPerp.x * innerOffset, segY, a.y + segPerp.y * innerOffset},
            {b.x + segPerp.x * innerOffset, segY, b.y + segPerp.y * innerOffset},
            {b.x + segPerp.x * outerOffset, segY, b.y + segPerp.y * outerOffset},
            {a.x + segPerp.x * outerOffset, segY, a.y + segPerp.y * outerOffset},
        };
        Color c = colorFn(i);

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

Mesh BuildDashedStripMesh(const Track& track, float innerOffset, float outerOffset, float yHeight, Color color,
                          int dashPeriod, int dashOn) {
    const auto& wp = track.Waypoints();
    size_t n = wp.size();

    int segmentCount = 0;
    for (size_t i = 0; i < n; ++i) {
        if (static_cast<int>(i % static_cast<size_t>(dashPeriod)) < dashOn) ++segmentCount;
    }

    Mesh mesh{};
    mesh.vertexCount = segmentCount * 4;
    mesh.triangleCount = segmentCount * 2;
    if (segmentCount == 0) return mesh;

    mesh.vertices = static_cast<float*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.normals = static_cast<float*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.colors = static_cast<unsigned char*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount) * 4 * sizeof(unsigned char)));
    mesh.indices = static_cast<unsigned short*>(MemAlloc(static_cast<unsigned int>(mesh.triangleCount) * 3 * sizeof(unsigned short)));

    int quadIdx = 0;
    for (size_t i = 0; i < n; ++i) {
        if (static_cast<int>(i % static_cast<size_t>(dashPeriod)) >= dashOn) continue;
        size_t j = (i + 1) % n;
        Vector2 a = wp[i];
        Vector2 b = wp[j];
        Vector2 ab{b.x - a.x, b.y - a.y};
        float segLen = std::sqrt(ab.x * ab.x + ab.y * ab.y);
        if (segLen < 1e-5f) continue;
        Vector2 segPerp{-ab.y / segLen, ab.x / segLen};
        float segY = yHeight + static_cast<float>(i) * 0.004f;
        Vector3 quad[4] = {
            {a.x + segPerp.x * innerOffset, segY, a.y + segPerp.y * innerOffset},
            {b.x + segPerp.x * innerOffset, segY, b.y + segPerp.y * innerOffset},
            {b.x + segPerp.x * outerOffset, segY, b.y + segPerp.y * outerOffset},
            {a.x + segPerp.x * outerOffset, segY, a.y + segPerp.y * outerOffset},
        };

        for (int k = 0; k < 4; ++k) {
            size_t vIdx = static_cast<size_t>(quadIdx) * 4 + static_cast<size_t>(k);
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

    UploadMesh(&mesh, false);
    return mesh;
}

Mesh BuildCheckerGroundMesh(float totalSize, int tilesPerSide, SurfaceStyle style) {
    float tileSize = totalSize / static_cast<float>(tilesPerSide);
    float half = totalSize * 0.5f;

    Mesh mesh{};
    mesh.vertexCount = tilesPerSide * tilesPerSide * 4;
    mesh.triangleCount = tilesPerSide * tilesPerSide * 2;
    mesh.vertices = static_cast<float*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.normals = static_cast<float*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.colors = static_cast<unsigned char*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount) * 4 * sizeof(unsigned char)));
    mesh.indices = static_cast<unsigned short*>(MemAlloc(static_cast<unsigned int>(mesh.triangleCount) * 3 * sizeof(unsigned short)));

    Color greenA = (style == SurfaceStyle::Abimee) ? Color{118, 108, 62, 255} : Color{34, 148, 38, 255};
    Color greenB = (style == SurfaceStyle::Abimee) ? Color{108, 98, 55, 255} : Color{28, 132, 32, 255};

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
    Vector2 dir{-p.y, p.x};
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

void DrawPropShadow(Vector3 pos, float radius) {
    DrawCylinder(Vector3{pos.x, 0.015f, pos.z}, radius, radius, 0.02f, 10, Fade(BLACK, 0.28f));
}

void DrawMountainsRing(float radius, float baseY, int segments) {
    Color farColor{72, 118, 158, 255};
    Color nearColor{58, 98, 132, 255};
    for (int i = 0; i < segments; ++i) {
        float a0 = (static_cast<float>(i) / segments) * 2.0f * PI;
        float a1 = (static_cast<float>(i + 1) / segments) * 2.0f * PI;
        float h0 = 8.0f + 12.0f * std::fabs(std::sin(a0 * 3.7f));
        float h1 = 8.0f + 12.0f * std::fabs(std::sin(a1 * 3.7f));
        Vector3 p0{std::cos(a0) * radius, baseY, std::sin(a0) * radius};
        Vector3 p1{std::cos(a1) * radius, baseY, std::sin(a1) * radius};
        Vector3 peak0{std::cos((a0 + a1) * 0.5f) * (radius - 15.0f), baseY + (h0 + h1) * 0.5f,
                      std::sin((a0 + a1) * 0.5f) * (radius - 15.0f)};
        DrawTriangle3D(p0, peak0, p1, (i % 2 == 0) ? farColor : nearColor);
    }
}

// ---- Phase 4 : outils de construction de meshes fusionnes ------------------

constexpr int kSkidTextureSize = 2048;

struct MeshBuffers {
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<unsigned char> colors;
    std::vector<unsigned short> indices;
};

// Quad p1..p4 en sens anti-horaire vu depuis le cote `normal` (deux triangles).
void AppendQuad(MeshBuffers& mb, Vector3 p1, Vector3 p2, Vector3 p3, Vector3 p4, Vector3 normal, Color color) {
    auto base = static_cast<unsigned short>(mb.vertices.size() / 3);
    const Vector3 pts[4] = {p1, p2, p3, p4};
    for (const Vector3& pt : pts) {
        mb.vertices.push_back(pt.x);
        mb.vertices.push_back(pt.y);
        mb.vertices.push_back(pt.z);
        mb.normals.push_back(normal.x);
        mb.normals.push_back(normal.y);
        mb.normals.push_back(normal.z);
        mb.colors.push_back(color.r);
        mb.colors.push_back(color.g);
        mb.colors.push_back(color.b);
        mb.colors.push_back(color.a);
    }
    const unsigned short quadIdx[6] = {0, 1, 2, 0, 2, 3};
    for (unsigned short off : quadIdx) mb.indices.push_back(static_cast<unsigned short>(base + off));
}

// Boite orientee par le triedre direct (ax, ay, az), demi-dimensions (hx, hy, hz).
void AppendBox(MeshBuffers& mb, Vector3 center, Vector3 ax, Vector3 ay, Vector3 az, float hx, float hy, float hz,
               Color color) {
    auto addFace = [&](Vector3 fn, Vector3 u, Vector3 v, float hu, float hv, float offN) {
        Vector3 fc{center.x + fn.x * offN, center.y + fn.y * offN, center.z + fn.z * offN};
        auto corner = [&](float su, float sv) {
            return Vector3{fc.x + u.x * su * hu + v.x * sv * hv, fc.y + u.y * su * hu + v.y * sv * hv,
                           fc.z + u.z * su * hu + v.z * sv * hv};
        };
        AppendQuad(mb, corner(-1.0f, -1.0f), corner(1.0f, -1.0f), corner(1.0f, 1.0f), corner(-1.0f, 1.0f), fn, color);
    };
    Vector3 nax{-ax.x, -ax.y, -ax.z};
    Vector3 nay{-ay.x, -ay.y, -ay.z};
    Vector3 naz{-az.x, -az.y, -az.z};
    addFace(ax, ay, az, hy, hz, hx);
    addFace(nax, az, ay, hz, hy, hx);
    addFace(ay, az, ax, hz, hx, hy);
    addFace(nay, ax, az, hx, hz, hy);
    addFace(az, ax, ay, hx, hy, hz);
    addFace(naz, ay, ax, hy, hx, hz);
}

Mesh MeshFromBuffers(const MeshBuffers& mb) {
    Mesh mesh{};
    mesh.vertexCount = static_cast<int>(mb.vertices.size() / 3);
    mesh.triangleCount = static_cast<int>(mb.indices.size() / 3);
    if (mesh.vertexCount == 0 || mesh.triangleCount == 0) return mesh;

    const size_t vBytes = mb.vertices.size() * sizeof(float);
    const size_t nBytes = mb.normals.size() * sizeof(float);
    const size_t cBytes = mb.colors.size() * sizeof(unsigned char);
    const size_t iBytes = mb.indices.size() * sizeof(unsigned short);
    mesh.vertices = static_cast<float*>(MemAlloc(static_cast<unsigned int>(vBytes)));
    mesh.normals = static_cast<float*>(MemAlloc(static_cast<unsigned int>(nBytes)));
    mesh.colors = static_cast<unsigned char*>(MemAlloc(static_cast<unsigned int>(cBytes)));
    mesh.indices = static_cast<unsigned short*>(MemAlloc(static_cast<unsigned int>(iBytes)));
    std::memcpy(mesh.vertices, mb.vertices.data(), vBytes);
    std::memcpy(mesh.normals, mb.normals.data(), nBytes);
    std::memcpy(mesh.colors, mb.colors.data(), cBytes);
    std::memcpy(mesh.indices, mb.indices.data(), iBytes);

    UploadMesh(&mesh, false);
    return mesh;
}

// Quad horizontal couvrant la zone carree des traces, UV 0..1 (2 triangles).
Mesh BuildSkidQuadMesh(Vector2 origin, float size, float y) {
    Mesh mesh{};
    mesh.vertexCount = 4;
    mesh.triangleCount = 2;
    mesh.vertices = static_cast<float*>(MemAlloc(static_cast<unsigned int>(4 * 3 * sizeof(float))));
    mesh.normals = static_cast<float*>(MemAlloc(static_cast<unsigned int>(4 * 3 * sizeof(float))));
    mesh.texcoords = static_cast<float*>(MemAlloc(static_cast<unsigned int>(4 * 2 * sizeof(float))));
    mesh.indices = static_cast<unsigned short*>(MemAlloc(static_cast<unsigned int>(6 * sizeof(unsigned short))));

    const float xs[4] = {origin.x, origin.x, origin.x + size, origin.x + size};
    const float zs[4] = {origin.y, origin.y + size, origin.y + size, origin.y};
    const float us[4] = {0.0f, 0.0f, 1.0f, 1.0f};
    const float vs[4] = {0.0f, 1.0f, 1.0f, 0.0f};
    for (int k = 0; k < 4; ++k) {
        mesh.vertices[k * 3 + 0] = xs[k];
        mesh.vertices[k * 3 + 1] = y;
        mesh.vertices[k * 3 + 2] = zs[k];
        mesh.normals[k * 3 + 0] = 0.0f;
        mesh.normals[k * 3 + 1] = 1.0f;
        mesh.normals[k * 3 + 2] = 0.0f;
        mesh.texcoords[k * 2 + 0] = us[k];
        mesh.texcoords[k * 2 + 1] = vs[k];
    }
    const unsigned short idx[6] = {0, 1, 2, 0, 2, 3};
    for (int k = 0; k < 6; ++k) mesh.indices[k] = idx[k];

    UploadMesh(&mesh, false);
    return mesh;
}

void ApplyShaderToModel(Model& model, Shader shader) {
    for (int i = 0; i < model.materialCount; ++i) {
        model.materials[i].shader = shader;
    }
}

Vector2 WorldToSkidTex(float worldX, float worldZ, Vector2 origin, float size) {
    float u = (worldX - origin.x) / size;
    float v = 1.0f - (worldZ - origin.y) / size;
    return Vector2{u * static_cast<float>(kSkidTextureSize), v * static_cast<float>(kSkidTextureSize)};
}

// DrawCube est aligne sur les axes monde : sans rotation, un gradin "le long de la
// piste" devient un mur de 70+ u sur l'axe X qui barre la route.
void DrawOrientedBox(Vector3 center, Vector3 along, float sizeAlong, float sizeUp, float sizeOut, Color color) {
    float heading = std::atan2(along.x, along.z) * RAD2DEG;
    rlPushMatrix();
    rlTranslatef(center.x, center.y, center.z);
    rlRotatef(heading, 0.0f, 1.0f, 0.0f);
    DrawCube(Vector3{0.0f, 0.0f, 0.0f}, sizeOut, sizeUp, sizeAlong, color);
    rlPopMatrix();
}

void DrawOrientedBoxWires(Vector3 center, Vector3 along, float sizeAlong, float sizeUp, float sizeOut, Color color) {
    float heading = std::atan2(along.x, along.z) * RAD2DEG;
    rlPushMatrix();
    rlTranslatef(center.x, center.y, center.z);
    rlRotatef(heading, 0.0f, 1.0f, 0.0f);
    DrawCubeWires(Vector3{0.0f, 0.0f, 0.0f}, sizeOut, sizeUp, sizeAlong, color);
    rlPopMatrix();
}

} // namespace

void DrawSkyGradient(int screenWidth, int screenHeight) {
    Color horizon{135, 196, 235, 255};
    Color zenith{42, 92, 168, 255};
    ClearBackground(horizon);
    int bands = 32;
    int bandH = (screenHeight + bands - 1) / bands;
    for (int i = 0; i < bands; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(bands - 1);
        Color c = LerpColor(horizon, zenith, t * t);
        DrawRectangle(0, i * bandH, screenWidth, bandH + 1, c);
    }
}

TrackRenderer::TrackRenderer(const Track& track, const TrackDef& def) : surfaceStyle_(def.surfaceStyle) {
    std::vector<Vector2> perp = ComputePerpendiculars(track);
    float halfWidth = track.Width() * 0.5f;
    trackHalfWidth_ = halfWidth;

    const auto& wp = track.Waypoints();

    constexpr float kRoadY = 0.10f;
    constexpr float kMarkY = kRoadY + 0.012f;
    constexpr float kCurbY = kRoadY - 0.015f;
    constexpr float kShoulderY = kRoadY - 0.025f;

    Mesh trackMesh = BuildStripMesh(track, -halfWidth, halfWidth, kRoadY, [&](size_t i) {
        return AsphaltColor(HashIndex(i), surfaceStyle_);
    });
    trackModel_ = LoadModelFromMesh(trackMesh);

    Mesh rubberMesh = BuildStripMesh(track, -0.35f, 0.35f, kRoadY + 0.001f, [&](size_t) {
        return Color{42, 42, 48, 255};
    });
    rubberLineModel_ = LoadModelFromMesh(rubberMesh);

    Mesh centerDash = BuildDashedStripMesh(track, -0.12f, 0.12f, kMarkY, WHITE, 4, 2);
    centerDashModel_ = LoadModelFromMesh(centerDash);

    constexpr float kEdgeWidth = 0.30f;
    Mesh edgeOuter = BuildStripMesh(track, halfWidth - kEdgeWidth, halfWidth - 0.02f, kMarkY,
                                    [&](size_t) { return Color{245, 245, 250, 255}; });
    edgeLineOuterModel_ = LoadModelFromMesh(edgeOuter);
    Mesh edgeInner = BuildStripMesh(track, -halfWidth + 0.02f, -halfWidth + kEdgeWidth, kMarkY,
                                    [&](size_t) { return Color{245, 245, 250, 255}; });
    edgeLineInnerModel_ = LoadModelFromMesh(edgeInner);

    constexpr float kCurbWidth = 1.4f;
    Color curbA = (surfaceStyle_ == SurfaceStyle::Abimee) ? Color{180, 170, 150, 255} : RED;
    Color curbB = (surfaceStyle_ == SurfaceStyle::Abimee) ? Color{160, 150, 130, 255} : RAYWHITE;
    Mesh curbMeshOuter = BuildStripMesh(track, halfWidth - kCurbWidth * 0.5f, halfWidth + kCurbWidth * 0.5f, kCurbY,
                                        [&](size_t i) {
                                            return (static_cast<int>(i / 3) % 2 == 0) ? curbA : curbB;
                                        });
    curbModelOuter_ = LoadModelFromMesh(curbMeshOuter);
    Mesh curbMeshInner = BuildStripMesh(track, -halfWidth - kCurbWidth * 0.5f, -halfWidth + kCurbWidth * 0.5f, kCurbY,
                                        [&](size_t i) {
                                            return (static_cast<int>(i / 3) % 2 == 0) ? curbA : curbB;
                                        });
    curbModelInner_ = LoadModelFromMesh(curbMeshInner);

    constexpr float kShoulderWidth = 2.2f;
    Color shoulderA = (surfaceStyle_ == SurfaceStyle::Abimee) ? Color{105, 92, 68, 255} : Color{128, 112, 82, 255};
    Color shoulderB = (surfaceStyle_ == SurfaceStyle::Abimee) ? Color{95, 84, 62, 255} : Color{115, 100, 74, 255};
    Mesh shoulderOuter = BuildStripMesh(track, halfWidth + kCurbWidth * 0.5f, halfWidth + kCurbWidth * 0.5f + kShoulderWidth,
                                        kShoulderY, [&](size_t i) {
                                            return (static_cast<int>(i / 4) % 2 == 0) ? shoulderA : shoulderB;
                                        });
    shoulderOuterModel_ = LoadModelFromMesh(shoulderOuter);
    Mesh shoulderInner = BuildStripMesh(track, -halfWidth - kCurbWidth * 0.5f - kShoulderWidth, -halfWidth - kCurbWidth * 0.5f,
                                        kShoulderY, [&](size_t i) {
                                            return (static_cast<int>(i / 4) % 2 == 0) ? shoulderA : shoulderB;
                                        });
    shoulderInnerModel_ = LoadModelFromMesh(shoulderInner);

    Mesh groundMesh = BuildCheckerGroundMesh(500.0f, 40, surfaceStyle_);
    groundModel_ = LoadModelFromMesh(groundMesh);

    Mesh finishMesh = BuildFinishLineMesh(track, perp);
    finishLineModel_ = LoadModelFromMesh(finishMesh);

    startGantryBase_ = Vector3{wp[0].x, 0.0f, wp[0].y};
    startGantryPerp_ = Vector3{perp[0].x, 0.0f, perp[0].y};
    startGantryAlong_ = Vector3{-perp[0].y, 0.0f, perp[0].x};

    // Nuages en anneau autour du circuit.
    for (int c = 0; c < 14; ++c) {
        uint32_t h = HashIndex(static_cast<size_t>(c) + 9000);
        float angle = static_cast<float>(c) / 14.0f * 2.0f * PI;
        float dist = 120.0f + static_cast<float>(h % 80);
        CloudInstance cloud;
        cloud.basePosition = Vector3{std::cos(angle) * dist, 42.0f + static_cast<float>(h % 20),
                                     std::sin(angle) * dist};
        cloud.driftSpeed = 0.4f + static_cast<float>(h % 50) * 0.01f;
        cloud.scale = 2.5f + static_cast<float>(h % 30) * 0.05f;
        int puffCount = 3 + static_cast<int>(h % 3);
        for (int p = 0; p < puffCount; ++p) {
            uint32_t ph = HashIndex(static_cast<size_t>(c * 10 + p));
            cloud.puffOffsets.push_back(
                Vector3{static_cast<float>(ph % 100) * 0.08f - 4.0f, static_cast<float>((ph >> 4) % 50) * 0.02f,
                        static_cast<float>((ph >> 8) % 100) * 0.08f - 4.0f});
            cloud.puffScales.push_back(1.8f + static_cast<float>(ph % 40) * 0.04f);
        }
        clouds_.push_back(cloud);
    }

    // Detection des lignes droites pour gradins.
    size_t n = wp.size();
    constexpr float kStraightDotThreshold = 0.995f;
    size_t runStart = 0;
    for (size_t i = 1; i <= n; ++i) {
        bool endRun = (i == n);
        if (!endRun && i + 1 < n) {
            Vector2 d0{wp[i].x - wp[i - 1].x, wp[i].y - wp[i - 1].y};
            Vector2 d1{wp[i + 1].x - wp[i].x, wp[i + 1].y - wp[i].y};
            float l0 = std::sqrt(d0.x * d0.x + d0.y * d0.y);
            float l1 = std::sqrt(d1.x * d1.x + d1.y * d1.y);
            if (l0 > 1e-4f && l1 > 1e-4f) {
                d0.x /= l0; d0.y /= l0;
                d1.x /= l1; d1.y /= l1;
                float dot = d0.x * d1.x + d0.y * d1.y;
                if (dot >= kStraightDotThreshold) continue;
            }
            endRun = true;
        }
        if (endRun) {
            size_t runEnd = (i == n) ? n - 1 : i;
            if (runEnd > runStart + 8) {
                size_t mid = (runStart + runEnd) / 2;
                Vector2 along{wp[runEnd].x - wp[runStart].x, wp[runEnd].y - wp[runStart].y};
                float alongLen = std::sqrt(along.x * along.x + along.y * along.y);
                if (alongLen > 30.0f) {
                    along.x /= alongLen;
                    along.y /= alongLen;
                    Vector2 outward{-along.y, along.x};
                    float side = (wp[mid].x * outward.x + wp[mid].y * outward.y) > 0.0f ? 1.0f : -1.0f;
                    outward.x *= side;
                    outward.y *= side;

                    GrandstandInstance gs;
                    gs.origin = Vector3{wp[mid].x + outward.x * (halfWidth + 10.0f), 0.0f,
                                        wp[mid].y + outward.y * (halfWidth + 10.0f)};
                    gs.along = Vector3{along.x, 0.0f, along.y};
                    gs.outward = Vector3{outward.x, 0.0f, outward.y};
                    gs.length = alongLen * 0.7f;

                    constexpr int kRows = 4;
                    constexpr int kCols = 14;
                    for (int row = 0; row < kRows; ++row) {
                        for (int col = 0; col < kCols; ++col) {
                            uint32_t sh = HashIndex(static_cast<size_t>(row * kCols + col) + mid * 17);
                            float alongT = (static_cast<float>(col) / (kCols - 1) - 0.5f) * gs.length;
                            float outwardD = static_cast<float>(row) * 1.8f + 1.5f;
                            float rowH = static_cast<float>(row) * 1.1f + 0.5f;
                            SpectatorInstance spec;
                            spec.position = Vector3{
                                gs.origin.x + gs.along.x * alongT + gs.outward.x * outwardD,
                                rowH,
                                gs.origin.z + gs.along.z * alongT + gs.outward.z * outwardD,
                            };
                            spec.shirtColor = Color{static_cast<unsigned char>(80 + sh % 175),
                                                    static_cast<unsigned char>(60 + (sh >> 4) % 175),
                                                    static_cast<unsigned char>(50 + (sh >> 8) % 175), 255};
                            spec.jumpPhase = static_cast<float>(sh % 628) * 0.01f;
                            spec.jumpSpeed = 2.5f + static_cast<float>(sh % 30) * 0.05f;
                            gs.spectators.push_back(spec);
                        }
                    }
                    grandstands_.push_back(gs);
                }
            }
            runStart = i;
        }
    }

    // Decor, PNJ, pneus, fanions.
    constexpr int kStride = 3;
    for (size_t i = 0; i < wp.size(); i += static_cast<size_t>(kStride)) {
        uint32_t h = HashIndex(i);
        float sideSign = (h % 2 == 0) ? 1.0f : -1.0f;
        float extraOffset = 4.0f + static_cast<float>(h % 100) * 0.05f;
        float dist = halfWidth + extraOffset;

        Vector3 pos{wp[i].x + perp[i].x * dist * sideSign, 0.0f, wp[i].y + perp[i].y * dist * sideSign};

        bool isBuilding = (h % 5 == 0) && surfaceStyle_ != SurfaceStyle::Abimee;
        bool isDeadTree = surfaceStyle_ == SurfaceStyle::Abimee && (h % 4 != 0);

        if (isBuilding) {
            PropInstance prop;
            prop.position = pos;
            prop.type = 1;
            prop.heightScale = 1.0f + static_cast<float>(h % 100) * 0.03f;
            prop.color = Color{static_cast<unsigned char>(90 + h % 60), static_cast<unsigned char>(90 + h % 60),
                               static_cast<unsigned char>(100 + h % 60), 255};
            props_.push_back(prop);
        } else {
            PropInstance prop;
            prop.position = pos;
            prop.type = isDeadTree ? 2 : 0;
            prop.heightScale = isDeadTree ? (0.7f + static_cast<float>(h % 50) * 0.004f)
                                          : (0.8f + static_cast<float>(h % 100) * 0.006f);
            prop.color = isDeadTree ? Color{92, 72, 48, 255}
                                    : Color{34, static_cast<unsigned char>(110 + h % 50), 34, 255};
            props_.push_back(prop);
        }

        if (h % 7 == 0) {
            NpcInstance npc;
            npc.position = Vector3{pos.x + perp[i].x * 1.5f, 0.0f, pos.z + perp[i].y * 1.5f};
            npc.heading = std::atan2(perp[i].x * sideSign, perp[i].y * sideSign);
            npc.shirtColor = Color{static_cast<unsigned char>(100 + h % 120), static_cast<unsigned char>(80 + (h >> 3) % 120),
                                   static_cast<unsigned char>(70 + (h >> 6) % 120), 255};
            npc.flagColor = Color{static_cast<unsigned char>(150 + h % 100), static_cast<unsigned char>(40 + (h >> 4) % 80),
                                  static_cast<unsigned char>(40 + (h >> 7) % 80), 255};
            npc.animPhase = static_cast<float>(h % 628) * 0.01f;
            npcs_.push_back(npc);
        }

        if (h % 11 == 0) {
            TireStackInstance stack;
            stack.position = Vector3{wp[i].x + perp[i].x * (halfWidth + 2.2f) * sideSign, 0.0f,
                                     wp[i].y + perp[i].y * (halfWidth + 2.2f) * sideSign};
            stack.tiers = 2 + static_cast<int>(h % 3);
            tireStacks_.push_back(stack);
        }

        if (h % 13 == 0) {
            PennantInstance pen;
            pen.base = Vector3{pos.x, 0.0f, pos.z};
            pen.top = Vector3{pos.x, 5.5f, pos.z};
            pen.color = Color{static_cast<unsigned char>(180 + h % 60), static_cast<unsigned char>(50 + (h >> 5) % 150),
                              static_cast<unsigned char>(50 + (h >> 9) % 150), 255};
            pen.phase = static_cast<float>(h % 628) * 0.01f;
            pennants_.push_back(pen);
        }
    }

    if (surfaceStyle_ == SurfaceStyle::Abimee) {
        for (size_t i = 0; i < wp.size(); i += 2) {
            uint32_t h = HashIndex(i + 50000);
            if (h % 3 != 0) continue;
            float t = static_cast<float>(h % 1000) / 1000.0f;
            size_t j = (i + 1) % wp.size();
            Vector2 p2{wp[i].x + (wp[j].x - wp[i].x) * t, wp[i].y + (wp[j].y - wp[i].y) * t};
            float lateral = (static_cast<float>((h >> 10) % 100) / 100.0f - 0.5f) * track.Width() * 0.7f;
            PotholeInstance hole;
            hole.position = Vector3{p2.x + perp[i].x * lateral, 0.025f, p2.y + perp[i].y * lateral};
            hole.radius = 0.35f + static_cast<float>(h % 40) * 0.015f;
            potholes_.push_back(hole);

            if (h % 5 == 0) {
                CrackInstance crack;
                crack.center = hole.position;
                crack.tangent = Vector3{perp[i].y, 0.0f, -perp[i].x};
                crack.length = 1.2f + static_cast<float>(h % 30) * 0.05f;
                cracks_.push_back(crack);
            }
        }
    }

    // ---- Traces de pneus persistantes (render-to-texture) -------------------
    float minX = wp[0].x, maxX = wp[0].x, minZ = wp[0].y, maxZ = wp[0].y;
    for (const auto& p : wp) {
        minX = std::min(minX, p.x);
        maxX = std::max(maxX, p.x);
        minZ = std::min(minZ, p.y);
        maxZ = std::max(maxZ, p.y);
    }
    constexpr float kSkidMargin = 8.0f;
    minX -= halfWidth + kSkidMargin;
    maxX += halfWidth + kSkidMargin;
    minZ -= halfWidth + kSkidMargin;
    maxZ += halfWidth + kSkidMargin;
    skidWorldSize_ = std::max(maxX - minX, maxZ - minZ);
    skidWorldOrigin_ = Vector2{(minX + maxX - skidWorldSize_) * 0.5f, (minZ + maxZ - skidWorldSize_) * 0.5f};

    skidTexture_ = LoadRenderTexture(kSkidTextureSize, kSkidTextureSize);
    BeginTextureMode(skidTexture_);
    ClearBackground(BLANK);
    EndTextureMode();

    Mesh skidQuad = BuildSkidQuadMesh(skidWorldOrigin_, skidWorldSize_, 0.115f);
    skidOverlayModel_ = LoadModelFromMesh(skidQuad);
    if (skidOverlayModel_.materialCount > 0) {
        skidOverlayModel_.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = skidTexture_.texture;
    }

    // Barrieres armco dans les virages serres (mesh fusionne).
    {
        MeshBuffers barrierBuf;
        Color postColor = (surfaceStyle_ == SurfaceStyle::Abimee) ? Color{120, 80, 55, 255} : Color{150, 150, 158, 255};
        Color railColor = (surfaceStyle_ == SurfaceStyle::Abimee) ? Color{140, 100, 70, 255} : Color{210, 210, 218, 255};
        constexpr float kCurveDot = 0.88f;
        for (size_t i = 0; i < n; ++i) {
            size_t prev = (i + n - 1) % n;
            size_t next = (i + 1) % n;
            Vector2 d0{wp[i].x - wp[prev].x, wp[i].y - wp[prev].y};
            Vector2 d1{wp[next].x - wp[i].x, wp[next].y - wp[i].y};
            float l0 = std::sqrt(d0.x * d0.x + d0.y * d0.y);
            float l1 = std::sqrt(d1.x * d1.x + d1.y * d1.y);
            if (l0 < 1e-4f || l1 < 1e-4f) continue;
            d0.x /= l0; d0.y /= l0;
            d1.x /= l1; d1.y /= l1;
            if (d0.x * d1.x + d0.y * d1.y >= kCurveDot) continue;

            float side = (perp[i].x * wp[i].x + perp[i].y * wp[i].y) > 0.0f ? 1.0f : -1.0f;
            Vector3 pos{wp[i].x + perp[i].x * (halfWidth + 2.5f) * side, 0.0f,
                        wp[i].y + perp[i].y * (halfWidth + 2.5f) * side};
            Vector3 along{d0.x + d1.x, 0.0f, d0.y + d1.y};
            float alen = std::sqrt(along.x * along.x + along.z * along.z);
            if (alen > 1e-4f) { along.x /= alen; along.z /= alen; }
            Vector3 out{perp[i].x * side, 0.0f, perp[i].y * side};

            AppendBox(barrierBuf, pos, along, Vector3{0.0f, 1.0f, 0.0f}, out, 0.08f, 0.35f, 0.08f, postColor);
            AppendBox(barrierBuf, Vector3{pos.x + along.x * 0.4f, 0.55f, pos.z + along.z * 0.4f}, along,
                      Vector3{0.0f, 1.0f, 0.0f}, out, 0.55f, 0.06f, 0.05f, railColor);
            AppendBox(barrierBuf, Vector3{pos.x - along.x * 0.4f, 0.75f, pos.z - along.z * 0.4f}, along,
                      Vector3{0.0f, 1.0f, 0.0f}, out, 0.55f, 0.06f, 0.05f, railColor);
        }
        if (!barrierBuf.vertices.empty()) {
            barrierModel_ = LoadModelFromMesh(MeshFromBuffers(barrierBuf));
            hasBarriers_ = true;
        }
    }

    // Panneaux sponsors (mesh fusionne).
    {
        MeshBuffers sponsorBuf;
        for (size_t i = 0; i < wp.size(); i += 12) {
            uint32_t h = HashIndex(i + 30000);
            float sideSign = (h % 2 == 0) ? 1.0f : -1.0f;
            float dist = halfWidth + 6.0f;
            Vector3 base{wp[i].x + perp[i].x * dist * sideSign, 0.0f, wp[i].y + perp[i].y * dist * sideSign};
            Color panel = Color{static_cast<unsigned char>(80 + h % 120),
                                static_cast<unsigned char>(60 + (h >> 4) % 120),
                                static_cast<unsigned char>(50 + (h >> 8) % 120), 255};
            Vector3 face{-perp[i].x * sideSign, 0.0f, -perp[i].y * sideSign};
            Vector3 right{face.z, 0.0f, -face.x};
            AppendBox(sponsorBuf, Vector3{base.x + face.x * 0.12f, 1.6f, base.z + face.z * 0.12f}, right,
                      Vector3{0.0f, 1.0f, 0.0f}, face, 1.1f, 0.75f, 0.05f, (h % 2 == 0) ? panel : Fade(WHITE, 0.35f));
            AppendBox(sponsorBuf, Vector3{base.x, 0.8f, base.z}, right, Vector3{0.0f, 1.0f, 0.0f}, face, 0.06f, 1.6f,
                      0.06f, DARKGRAY);
        }
        if (!sponsorBuf.vertices.empty()) {
            sponsorModel_ = LoadModelFromMesh(MeshFromBuffers(sponsorBuf));
            hasSponsors_ = true;
        }
    }

    // Lampadaires.
    for (size_t i = 0; i < wp.size(); i += 10) {
        uint32_t h = HashIndex(i + 40000);
        float sideSign = (static_cast<int>(i / 10) % 2 == 0) ? 1.0f : -1.0f;
        float dist = halfWidth + 5.0f;
        LampInstance lamp;
        lamp.base = Vector3{wp[i].x + perp[i].x * dist * sideSign, 0.0f, wp[i].y + perp[i].y * dist * sideSign};
        bool broken = surfaceStyle_ == SurfaceStyle::Abimee && (h % 3 == 0);
        float lean = broken ? 0.2f : 0.06f;
        lamp.top = Vector3{lamp.base.x + perp[i].x * sideSign * lean, 6.5f, lamp.base.z + perp[i].y * sideSign * lean};
        lamp.lit = !broken;
        lamp.headColor = lamp.lit ? Color{255, 240, 200, 255} : Color{70, 70, 80, 255};
        lamps_.push_back(lamp);
    }

    // Arche gonflable au milieu du circuit.
    {
        size_t mid = n / 2;
        float hw = halfWidth + 0.5f;
        arch_.leftBase = Vector3{wp[mid].x - perp[mid].x * hw, 0.0f, wp[mid].y - perp[mid].y * hw};
        arch_.rightBase = Vector3{wp[mid].x + perp[mid].x * hw, 0.0f, wp[mid].y + perp[mid].y * hw};
        arch_.colorA = Color{220, 40, 60, 255};
        arch_.colorB = Color{40, 80, 220, 255};
        hasArch_ = true;
    }
}

TrackRenderer::~TrackRenderer() {
    UnloadModel(trackModel_);
    UnloadModel(rubberLineModel_);
    UnloadModel(centerDashModel_);
    UnloadModel(edgeLineOuterModel_);
    UnloadModel(edgeLineInnerModel_);
    UnloadModel(curbModelOuter_);
    UnloadModel(curbModelInner_);
    UnloadModel(shoulderOuterModel_);
    UnloadModel(shoulderInnerModel_);
    UnloadModel(groundModel_);
    UnloadModel(finishLineModel_);
    if (skidTexture_.id != 0) UnloadRenderTexture(skidTexture_);
    UnloadModel(skidOverlayModel_);
    if (hasBarriers_) UnloadModel(barrierModel_);
    if (hasSponsors_) UnloadModel(sponsorModel_);
}

void TrackRenderer::DrawOpaqueGeometry() const {
    DrawModel(groundModel_, Vector3{0.0f, -0.08f, 0.0f}, 1.0f, WHITE);
    DrawModel(shoulderOuterModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    DrawModel(shoulderInnerModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    DrawModel(trackModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    DrawModel(rubberLineModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    DrawModel(centerDashModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    DrawModel(edgeLineOuterModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    DrawModel(edgeLineInnerModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    DrawModel(curbModelOuter_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    DrawModel(curbModelInner_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    if (hasBarriers_) DrawModel(barrierModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    if (hasSponsors_) DrawModel(sponsorModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    rlDisableDepthMask();
    BeginBlendMode(BLEND_ALPHA);
    DrawModel(skidOverlayModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    EndBlendMode();
    rlEnableDepthMask();
    DrawModel(finishLineModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
}

void TrackRenderer::Draw(float timeSeconds) const {
    DrawMountainsRing(220.0f, 2.0f, 48);

    for (const auto& cloud : clouds_) {
        float drift = timeSeconds * cloud.driftSpeed;
        Vector3 center{
            cloud.basePosition.x + std::sin(drift * 0.15f) * 8.0f,
            cloud.basePosition.y + std::sin(drift * 0.08f) * 0.6f,
            cloud.basePosition.z + std::cos(drift * 0.12f) * 8.0f,
        };
        for (size_t i = 0; i < cloud.puffOffsets.size(); ++i) {
            Vector3 off = cloud.puffOffsets[i];
            float s = cloud.puffScales[i] * cloud.scale;
            DrawSphere(Vector3{center.x + off.x, center.y + off.y, center.z + off.z}, s,
                       Color{255, 255, 255, 200});
        }
    }

    DrawOpaqueGeometry();

    for (const auto& lamp : lamps_) {
        DrawCylinderEx(lamp.base, lamp.top, 0.08f, 0.06f, 6, Color{90, 90, 95, 255});
        if (lamp.lit) {
            DrawSphere(lamp.top, 0.22f, lamp.headColor);
            DrawCylinder(Vector3{lamp.top.x, 0.02f, lamp.top.z}, 1.0f, 1.0f, 0.02f, 10, Fade(lamp.headColor, 0.14f));
        } else {
            DrawSphere(lamp.top, 0.15f, lamp.headColor);
        }
    }

    if (hasArch_) {
        constexpr float pillarH = 5.5f;
        DrawCylinder(arch_.leftBase, 0.35f, 0.35f, pillarH, 8, arch_.colorA);
        DrawCylinder(arch_.rightBase, 0.35f, 0.35f, pillarH, 8, arch_.colorB);
        Vector3 leftTop{arch_.leftBase.x, pillarH, arch_.leftBase.z};
        Vector3 rightTop{arch_.rightBase.x, pillarH, arch_.rightBase.z};
        for (int s = 0; s < 8; ++s) {
            float t = static_cast<float>(s) / 7.0f;
            float archY = pillarH + 1.8f * std::sin(t * PI);
            Vector3 p{
                leftTop.x + (rightTop.x - leftTop.x) * t,
                archY,
                leftTop.z + (rightTop.z - leftTop.z) * t,
            };
            Color c = (s % 2 == 0) ? arch_.colorA : arch_.colorB;
            DrawCube(p, 1.4f, 0.5f, 0.5f, c);
        }
    }

    for (const auto& hole : potholes_) {
        DrawCylinder(hole.position, hole.radius, hole.radius * 0.85f, 0.04f, 12, Color{35, 32, 30, 255});
        DrawCylinder(hole.position, hole.radius * 0.6f, hole.radius * 0.5f, 0.05f, 10, Color{22, 20, 18, 255});
    }
    for (const auto& crack : cracks_) {
        Vector3 a{crack.center.x - crack.tangent.x * crack.length * 0.5f, crack.center.y + 0.01f,
                  crack.center.z - crack.tangent.z * crack.length * 0.5f};
        Vector3 b{crack.center.x + crack.tangent.x * crack.length * 0.5f, crack.center.y + 0.01f,
                  crack.center.z + crack.tangent.z * crack.length * 0.5f};
        DrawCylinderEx(a, b, 0.06f, 0.06f, 4, Color{48, 44, 40, 255});
    }

    if (surfaceStyle_ == SurfaceStyle::Abimee) {
        for (size_t i = 0; i < potholes_.size(); i += 2) {
            const auto& h = potholes_[i];
            DrawCube(Vector3{h.position.x + 0.8f, 0.08f, h.position.z}, 0.5f, 0.16f, 0.5f, Color{90, 110, 45, 255});
        }
    }

    // Portique depart/arrivee.
    {
        Vector3 p = startGantryPerp_;
        Vector3 d = startGantryAlong_;
        float hw = trackHalfWidth_ + 1.5f;
        Vector3 leftBase{startGantryBase_.x - p.x * hw, 0.0f, startGantryBase_.z - p.z * hw};
        Vector3 rightBase{startGantryBase_.x + p.x * hw, 0.0f, startGantryBase_.z + p.z * hw};
        constexpr float pillarH = 7.5f;
        Color pillarColor{180, 180, 190, 255};
        DrawCube(Vector3{leftBase.x, pillarH * 0.5f, leftBase.z}, 0.5f, pillarH, 0.5f, pillarColor);
        DrawCube(Vector3{rightBase.x, pillarH * 0.5f, rightBase.z}, 0.5f, pillarH, 0.5f, pillarColor);
        Vector3 beamMid{
            (leftBase.x + rightBase.x) * 0.5f,
            pillarH,
            (leftBase.z + rightBase.z) * 0.5f,
        };
        DrawOrientedBox(beamMid, startGantryPerp_, hw * 2.0f + 1.0f, 0.4f, 0.5f, DARKGRAY);
        DrawOrientedBox(Vector3{beamMid.x, pillarH + 0.6f, beamMid.z}, startGantryPerp_, hw * 1.6f, 0.5f, 0.15f, WHITE);
        for (int c = 0; c < 6; ++c) {
            Color cell = (c % 2 == 0) ? BLACK : WHITE;
            float t = -0.5f + (static_cast<float>(c) + 0.5f) / 6.0f;
            Vector3 cellPos{
                beamMid.x + startGantryPerp_.x * t * hw * 1.4f,
                pillarH + 0.6f,
                beamMid.z + startGantryPerp_.z * t * hw * 1.4f,
            };
            DrawOrientedBox(cellPos, startGantryPerp_, hw * 0.22f, 0.48f, 0.12f, cell);
        }
        float lightPhase = std::fmod(timeSeconds * 2.0f, 3.0f);
        Color lightColors[3] = {RED, YELLOW, GREEN};
        for (int l = 0; l < 3; ++l) {
            bool on = lightPhase >= static_cast<float>(l) && lightPhase < static_cast<float>(l + 1);
            Color lc = on ? lightColors[l] : Fade(lightColors[l], 0.25f);
            DrawSphere(Vector3{leftBase.x + d.x * 0.8f, pillarH - 1.5f - static_cast<float>(l) * 0.7f,
                               leftBase.z + d.z * 0.8f},
                       0.18f, lc);
        }
    }

    // Gradins (structure statique, orientes le long de la ligne droite).
    for (const auto& gs : grandstands_) {
        constexpr int kRows = 4;
        float stepDepth = 1.8f;
        float stepWidth = gs.length;
        for (int row = 0; row < kRows; ++row) {
            Vector3 seatPos{
                gs.origin.x + gs.outward.x * (static_cast<float>(row) * stepDepth + 0.9f),
                static_cast<float>(row) * 1.1f + 0.55f,
                gs.origin.z + gs.outward.z * (static_cast<float>(row) * stepDepth + 0.9f),
            };
            DrawOrientedBox(seatPos, gs.along, stepWidth, 1.1f, stepDepth, Color{110, 110, 118, 255});
            DrawOrientedBoxWires(seatPos, gs.along, stepWidth, 1.1f, stepDepth, Fade(BLACK, 0.3f));
        }
        Vector3 roofPos{
            gs.origin.x + gs.outward.x * (static_cast<float>(kRows) * stepDepth + 0.5f),
            static_cast<float>(kRows) * 1.1f + 1.2f,
            gs.origin.z + gs.outward.z * (static_cast<float>(kRows) * stepDepth + 0.5f),
        };
        DrawOrientedBox(roofPos, gs.along, stepWidth + 1.0f, 0.25f, stepDepth + 1.5f, Color{180, 50, 50, 255});
    }

    // Foule animee.
    for (const auto& gs : grandstands_) {
        for (const auto& spec : gs.spectators) {
            float jump = std::max(0.0f, std::sin(timeSeconds * spec.jumpSpeed + spec.jumpPhase)) * 0.35f;
            if (std::sin(timeSeconds * spec.jumpSpeed + spec.jumpPhase) < 0.85f) jump = 0.0f;
            Vector3 pos{spec.position.x, spec.position.y + jump, spec.position.z};
            DrawCube(Vector3{pos.x, pos.y + 0.35f, pos.z}, 0.35f, 0.7f, 0.25f, spec.shirtColor);
            DrawSphere(Vector3{pos.x, pos.y + 0.85f, pos.z}, 0.18f, Color{240, 200, 170, 255});
        }
    }

    for (const auto& prop : props_) {
        float shadowR = (prop.type == 1) ? 1.8f : 1.2f;
        DrawPropShadow(prop.position, shadowR);
        if (prop.type == 1) {
            float w = 2.5f;
            float h = 5.0f * prop.heightScale;
            DrawCube(Vector3{prop.position.x, h * 0.5f, prop.position.z}, w, h, w, prop.color);
            DrawCubeWires(Vector3{prop.position.x, h * 0.5f, prop.position.z}, w, h, w, Fade(BLACK, 0.4f));
            for (int win = 0; win < 3; ++win) {
                DrawCube(Vector3{prop.position.x + 0.8f, 1.5f + static_cast<float>(win) * 1.4f, prop.position.z + 1.26f},
                         0.4f, 0.5f, 0.05f, Color{200, 220, 255, 200});
            }
        } else if (prop.type == 2) {
            float trunkH = 1.2f * prop.heightScale;
            DrawCylinder(Vector3{prop.position.x, 0.0f, prop.position.z}, 0.2f, 0.25f, trunkH, 5, Color{72, 52, 36, 255});
            DrawSphere(Vector3{prop.position.x, trunkH + 0.3f, prop.position.z}, 0.5f * prop.heightScale, prop.color);
            DrawSphere(Vector3{prop.position.x + 0.4f, trunkH, prop.position.z - 0.2f}, 0.35f * prop.heightScale, prop.color);
        } else {
            float trunkH = 1.6f * prop.heightScale;
            DrawCylinder(Vector3{prop.position.x, 0.0f, prop.position.z}, 0.25f, 0.3f, trunkH, 6, Color{92, 64, 40, 255});
            DrawCylinder(Vector3{prop.position.x, trunkH, prop.position.z}, 1.6f * prop.heightScale, 0.0f,
                         2.2f * prop.heightScale, 8, prop.color);
        }
    }

    for (const auto& stack : tireStacks_) {
        DrawPropShadow(stack.position, 0.9f);
        for (int t = 0; t < stack.tiers; ++t) {
            Color tc = (t % 2 == 0) ? Color{30, 30, 32, 255} : Color{220, 220, 220, 255};
            DrawCylinder(Vector3{stack.position.x, static_cast<float>(t) * 0.35f + 0.18f, stack.position.z}, 0.55f, 0.55f,
                         0.35f, 10, tc);
        }
    }

    for (const auto& pen : pennants_) {
        float wave = std::sin(timeSeconds * 3.0f + pen.phase) * 0.3f;
        DrawCylinderEx(pen.base, pen.top, 0.06f, 0.04f, 4, Color{120, 120, 130, 255});
        Vector3 flagEnd{pen.top.x + 1.2f + wave, pen.top.y + std::sin(timeSeconds * 4.0f + pen.phase) * 0.15f,
                        pen.top.z};
        DrawCylinderEx(pen.top, flagEnd, 0.02f, 0.02f, 3, pen.color);
        DrawTriangle3D(pen.top, flagEnd, Vector3{pen.top.x, pen.top.y - 0.5f, pen.top.z}, pen.color);
    }

    for (const auto& npc : npcs_) {
        DrawPropShadow(npc.position, 0.45f);
        float armWave = std::sin(timeSeconds * 4.0f + npc.animPhase);
        rlPushMatrix();
        rlTranslatef(npc.position.x, 0.0f, npc.position.z);
        rlRotatef(npc.heading * RAD2DEG, 0.0f, 1.0f, 0.0f);
        DrawCylinder(Vector3{0.0f, 0.9f, 0.0f}, 0.22f, 0.25f, 1.1f, 6, npc.shirtColor);
        DrawSphere(Vector3{0.0f, 1.65f, 0.0f}, 0.22f, Color{240, 200, 170, 255});
        DrawCube(Vector3{0.45f, 1.3f + armWave * 0.15f, 0.0f}, 0.12f, 0.5f, 0.12f, npc.shirtColor);
        DrawCube(Vector3{-0.35f, 1.1f, 0.0f}, 0.12f, 0.4f, 0.12f, npc.shirtColor);
        DrawCylinder(Vector3{0.7f, 1.5f, 0.0f}, 0.03f, 0.03f, 1.2f, 4, Color{100, 80, 60, 255});
        rlPushMatrix();
        rlRotatef(armWave * 25.0f, 0.0f, 0.0f, 1.0f);
        DrawCube(Vector3{0.7f, 2.2f, 0.0f}, 0.5f, 0.35f, 0.05f, npc.flagColor);
        rlPopMatrix();
        rlPopMatrix();
    }
}

void TrackRenderer::ApplyShader(Shader shader) {
    ApplyShaderToModel(trackModel_, shader);
    ApplyShaderToModel(rubberLineModel_, shader);
    ApplyShaderToModel(centerDashModel_, shader);
    ApplyShaderToModel(edgeLineOuterModel_, shader);
    ApplyShaderToModel(edgeLineInnerModel_, shader);
    ApplyShaderToModel(curbModelOuter_, shader);
    ApplyShaderToModel(curbModelInner_, shader);
    ApplyShaderToModel(shoulderOuterModel_, shader);
    ApplyShaderToModel(shoulderInnerModel_, shader);
    ApplyShaderToModel(groundModel_, shader);
    ApplyShaderToModel(finishLineModel_, shader);
    ApplyShaderToModel(skidOverlayModel_, shader);
    if (hasBarriers_) ApplyShaderToModel(barrierModel_, shader);
    if (hasSponsors_) ApplyShaderToModel(sponsorModel_, shader);
}

void TrackRenderer::QueueSkidMark(Vector3 pos, Vector3 dir, float width, float strength) {
    float len = std::sqrt(dir.x * dir.x + dir.z * dir.z);
    if (len < 1e-4f) return;
    SkidMarkCmd cmd;
    cmd.position = pos;
    cmd.direction = Vector3{dir.x / len, 0.0f, dir.z / len};
    cmd.width = width;
    cmd.strength = std::clamp(strength, 0.0f, 1.0f);
    skidQueue_.push_back(cmd);
}

void TrackRenderer::FlushSkidMarks() {
    if (skidQueue_.empty() || skidTexture_.id == 0) return;

    BeginTextureMode(skidTexture_);
    for (const auto& cmd : skidQueue_) {
        Vector2 center =
            WorldToSkidTex(cmd.position.x, cmd.position.z, skidWorldOrigin_, skidWorldSize_);
        float lenPx = 1.2f / skidWorldSize_ * static_cast<float>(kSkidTextureSize);
        float wPx = cmd.width / skidWorldSize_ * static_cast<float>(kSkidTextureSize);
        float angle = std::atan2(cmd.direction.x, cmd.direction.z) * RAD2DEG;
        Rectangle rect{center.x - lenPx * 0.5f, center.y - wPx * 0.5f, lenPx, wPx};
        DrawRectanglePro(rect, Vector2{lenPx * 0.5f, wPx * 0.5f}, angle,
                         Color{0, 0, 0, static_cast<unsigned char>(cmd.strength * 76.0f)});
    }
    EndTextureMode();
    skidQueue_.clear();
}

} // namespace racer
