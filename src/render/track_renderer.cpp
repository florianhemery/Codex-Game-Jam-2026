/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track mesh building and environment rendering
*/

#include "render/track_renderer.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <vector>

#include "rlgl.h"

namespace racer {

constexpr int kSkidTextureSize = 2048;

namespace {

class TrackRendererDetail {
public:
    struct MeshBuffers {
        std::vector<float> vertices;
        std::vector<float> normals;
        std::vector<unsigned char> colors;
        std::vector<unsigned short> indices;
    };

    static std::vector<Vector2> computePerpendiculars(const Track &track);
    static uint32_t hashIndex(size_t i);
    static Color lerpColor(Color a, Color b, float t);
    static Color asphaltColor(uint32_t h, SurfaceStyle style);
    static Mesh buildStripMesh(
        const Track &track, const std::vector<Vector2> &perp,
        float innerOffset, float outerOffset, float yHeight,
        const std::function<Color(size_t)> &colorFn);
    static Mesh buildDashedStripMesh(
        const Track &track, const std::vector<Vector2> &perp,
        float innerOffset, float outerOffset, float yHeight,
        Color color, int dashPeriod, int dashOn);
    static Mesh buildCheckerGroundMesh(
        float totalSize, int tilesPerSide, SurfaceStyle style);
    static Mesh buildFinishLineMesh(
        const Track &track, const std::vector<Vector2> &perp);
    static void drawPropShadow(Vector3 pos, float radius);
    static void drawMountainsRing(float radius, float baseY, int segments);
    static void appendQuad(
        MeshBuffers &mb, Vector3 p1, Vector3 p2, Vector3 p3, Vector3 p4,
        Vector3 normal, Color color);
    static void appendBox(
        MeshBuffers &mb, Vector3 center, Vector3 ax, Vector3 ay,
        Vector3 az, float hx, float hy, float hz, Color color);
    static Mesh meshFromBuffers(const MeshBuffers &mb);
    static Mesh buildSkidQuadMesh(Vector2 origin, float size, float y);
    static void applyShaderToModel(Model &model, Shader shader);
    static Vector2 worldToSkidTex(
        float worldX, float worldZ, Vector2 origin, float size);

private:
    static Vector2 computePerpendicularAt(
        const std::vector<Vector2> &wp, size_t i, size_t n);
    static unsigned char lerpChannel(unsigned char x, unsigned char y, float t);
    static void fillStripEndpoints(
        const Track &track, const std::vector<Vector2> &perp,
        float innerOffset, float outerOffset, float yHeight,
        std::vector<Vector3> &inner, std::vector<Vector3> &outer);
    static void allocColoredStripMesh(Mesh &mesh, size_t n);
    static void writeStripSegment(
        Mesh &mesh, size_t i, size_t n,
        const std::vector<Vector3> &inner, const std::vector<Vector3> &outer,
        Color c);
    static int countDashSegments(size_t n, int dashPeriod, int dashOn);
    static void allocDashedStripMesh(Mesh &mesh, int segmentCount);
    static void writeDashedSegment(
        Mesh &mesh, int quadIdx, const Vector3 quad[4], Color color);
    static void writeCheckerTile(
        Mesh &mesh, int quadIdx, const Vector3 quad[4], Color c);
    static void writeFinishColumn(
        Mesh &mesh, int col, const Vector3 quad[4], Color color);
    static void drawMountainSegment(
        int i, int segments, float radius, float baseY,
        Color farColor, Color nearColor);
    static void appendBoxFace(
        MeshBuffers &mb, Vector3 center, Vector3 fn, Vector3 u, Vector3 v,
        float hu, float hv, float offN, Color color);
    static Vector3 boxCorner(
        Vector3 fc, Vector3 u, Vector3 v, float hu, float hv,
        float su, float sv);
    static void fillSkidQuadVertices(Mesh &mesh, Vector2 origin, float size,
                                     float y);
    static void allocFinishLineMesh(Mesh &mesh, int colCount);
    static void fillFinishLineColumns(
        Mesh &mesh, Vector2 base, Vector2 p, Vector2 dir, float halfWidth);
    static void makeFinishColumnQuad(
        Vector3 quad[4], Vector2 base, Vector2 p, Vector2 dir,
        float halfWidth, float t0, float t1);
    static void normalizeVector2(Vector2 &v);
    static void writeColoredQuadVertices(
        Mesh &mesh, size_t vBase, const Vector3 quad[4], Color c);
    static void writeMeshQuadIndices(
        Mesh &mesh, size_t segIdx, unsigned short vBase);
    static void allocCheckerMesh(Mesh &mesh, int tilesPerSide);
    static void fillCheckerTiles(
        Mesh &mesh, int tilesPerSide, float tileSize, float half,
        Color greenA, Color greenB);
    static void copyMeshBufferData(Mesh &mesh, const MeshBuffers &mb);
    static void writeSkidQuadCorner(
        Mesh &mesh, int k, float x, float z, float y, float u, float v);
    static void fillDashedStripLoop(
        Mesh &mesh, const Track &track, const std::vector<Vector2> &perp,
        size_t n, float innerOffset, float outerOffset, float yHeight,
        Color color, int dashPeriod, int dashOn);
    static Vector3 makeDashedQuadPoint(
        const Track &track, const std::vector<Vector2> &perp,
        size_t idx, float innerOffset, float outerOffset, float yHeight,
        bool outer);
};

Vector2 TrackRendererDetail::computePerpendicularAt(
    const std::vector<Vector2> &wp, size_t i, size_t n)
{
    Vector2 prev = wp[(i + n - 1) % n];
    Vector2 cur = wp[i];
    Vector2 next = wp[(i + 1) % n];
    Vector2 dirA{cur.x - prev.x, cur.y - prev.y};
    Vector2 dirB{next.x - cur.x, next.y - cur.y};

    normalizeVector2(dirA);
    normalizeVector2(dirB);
    Vector2 avg{dirA.x + dirB.x, dirA.y + dirB.y};

    normalizeVector2(avg);
    return Vector2{-avg.y, avg.x};
}

void TrackRendererDetail::normalizeVector2(Vector2 &v)
{
    float len = std::sqrt(v.x * v.x + v.y * v.y);

    if (len > 1e-6f) {
        v.x /= len;
        v.y /= len;
    }
}

std::vector<Vector2> TrackRendererDetail::computePerpendiculars(
    const Track &track)
{
    const auto &wp = track.Waypoints();
    size_t n = wp.size();
    std::vector<Vector2> perp(n);

    for (size_t i = 0; i < n; ++i)
        perp[i] = computePerpendicularAt(wp, i, n);
    return perp;
}

uint32_t TrackRendererDetail::hashIndex(size_t i)
{
    uint32_t h = static_cast<uint32_t>(i) * 2654435761u;

    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}

unsigned char TrackRendererDetail::lerpChannel(
    unsigned char x, unsigned char y, float t)
{
    float xf = static_cast<float>(x);
    float yf = static_cast<float>(y);
    float out = xf + (yf - xf) * t;

    return static_cast<unsigned char>(out);
}

Color TrackRendererDetail::lerpColor(Color a, Color b, float t)
{
    return Color{
        lerpChannel(a.r, b.r, t),
        lerpChannel(a.g, b.g, t),
        lerpChannel(a.b, b.b, t),
        lerpChannel(a.a, b.a, t),
    };
}

Color TrackRendererDetail::asphaltColor(uint32_t h, SurfaceStyle style)
{
    int noise = static_cast<int>(h % 37);

    if (style == SurfaceStyle::Abimee) {
        unsigned char base = static_cast<unsigned char>(95 + noise);

        return Color{
            base,
            static_cast<unsigned char>(base - 4),
            static_cast<unsigned char>(base - 8),
            255,
        };
    }
    unsigned char base = static_cast<unsigned char>(52 + noise % 18);

    return Color{
        base, base, static_cast<unsigned char>(base + 4), 255,
    };
}

void TrackRendererDetail::fillStripEndpoints(
    const Track &track, const std::vector<Vector2> &perp,
    float innerOffset, float outerOffset, float yHeight,
    std::vector<Vector3> &inner, std::vector<Vector3> &outer)
{
    const auto &wp = track.Waypoints();
    size_t n = wp.size();

    inner.resize(n);
    outer.resize(n);
    for (size_t i = 0; i < n; ++i) {
        inner[i] = Vector3{
            wp[i].x + perp[i].x * innerOffset, yHeight,
            wp[i].y + perp[i].y * innerOffset,
        };
        outer[i] = Vector3{
            wp[i].x + perp[i].x * outerOffset, yHeight,
            wp[i].y + perp[i].y * outerOffset,
        };
    }
}

void TrackRendererDetail::allocColoredStripMesh(Mesh &mesh, size_t n)
{
    mesh.vertexCount = static_cast<int>(n) * 4;
    mesh.triangleCount = static_cast<int>(n) * 2;
    mesh.vertices = static_cast<float *>(MemAlloc(
        static_cast<unsigned int>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.normals = static_cast<float *>(MemAlloc(
        static_cast<unsigned int>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.colors = static_cast<unsigned char *>(MemAlloc(
        static_cast<unsigned int>(mesh.vertexCount) * 4
        * sizeof(unsigned char)));
    mesh.indices = static_cast<unsigned short *>(MemAlloc(
        static_cast<unsigned int>(mesh.triangleCount) * 3
        * sizeof(unsigned short)));
}

void TrackRendererDetail::writeColoredQuadVertices(
    Mesh &mesh, size_t vBase, const Vector3 quad[4], Color c)
{
    for (int k = 0; k < 4; ++k) {
        size_t vIdx = vBase + static_cast<size_t>(k);

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
}

void TrackRendererDetail::writeMeshQuadIndices(
    Mesh &mesh, size_t segIdx, unsigned short vBase)
{
    size_t idx = segIdx * 6;

    mesh.indices[idx + 0] = vBase + 0;
    mesh.indices[idx + 1] = vBase + 1;
    mesh.indices[idx + 2] = vBase + 2;
    mesh.indices[idx + 3] = vBase + 0;
    mesh.indices[idx + 4] = vBase + 2;
    mesh.indices[idx + 5] = vBase + 3;
}

void TrackRendererDetail::writeStripSegment(
    Mesh &mesh, size_t i, size_t n,
    const std::vector<Vector3> &inner, const std::vector<Vector3> &outer,
    Color c)
{
    size_t j = (i + 1) % n;
    Vector3 quad[4] = {inner[i], inner[j], outer[j], outer[i]};

    writeColoredQuadVertices(mesh, i * 4, quad, c);
    writeMeshQuadIndices(
        mesh, i, static_cast<unsigned short>(i * 4));
}

void TrackRendererDetail::writeDashedSegment(
    Mesh &mesh, int quadIdx, const Vector3 quad[4], Color color)
{
    size_t vBase = static_cast<size_t>(quadIdx) * 4;

    writeColoredQuadVertices(mesh, vBase, quad, color);
    writeMeshQuadIndices(
        mesh, static_cast<size_t>(quadIdx),
        static_cast<unsigned short>(quadIdx * 4));
}

void TrackRendererDetail::writeCheckerTile(
    Mesh &mesh, int quadIdx, const Vector3 quad[4], Color c)
{
    size_t vBase = static_cast<size_t>(quadIdx) * 4;

    writeColoredQuadVertices(mesh, vBase, quad, c);
    writeMeshQuadIndices(
        mesh, static_cast<size_t>(quadIdx),
        static_cast<unsigned short>(quadIdx * 4));
}

void TrackRendererDetail::writeFinishColumn(
    Mesh &mesh, int col, const Vector3 quad[4], Color color)
{
    size_t vBase = static_cast<size_t>(col) * 4;

    writeColoredQuadVertices(mesh, vBase, quad, color);
    writeMeshQuadIndices(
        mesh, static_cast<size_t>(col),
        static_cast<unsigned short>(col * 4));
}

Mesh TrackRendererDetail::buildStripMesh(
    const Track &track, const std::vector<Vector2> &perp,
    float innerOffset, float outerOffset, float yHeight,
    const std::function<Color(size_t)> &colorFn)
{
    const auto &wp = track.Waypoints();
    size_t n = wp.size();
    std::vector<Vector3> inner;
    std::vector<Vector3> outer;

    fillStripEndpoints(
        track, perp, innerOffset, outerOffset, yHeight, inner, outer);
    Mesh mesh{};

    allocColoredStripMesh(mesh, n);
    for (size_t i = 0; i < n; ++i)
        writeStripSegment(mesh, i, n, inner, outer, colorFn(i));
    UploadMesh(&mesh, false);
    return mesh;
}

int TrackRendererDetail::countDashSegments(
    size_t n, int dashPeriod, int dashOn)
{
    int segmentCount = 0;

    for (size_t i = 0; i < n; ++i) {
        if (static_cast<int>(i % static_cast<size_t>(dashPeriod)) < dashOn)
            ++segmentCount;
    }
    return segmentCount;
}

void TrackRendererDetail::allocDashedStripMesh(
    Mesh &mesh, int segmentCount)
{
    mesh.vertexCount = segmentCount * 4;
    mesh.triangleCount = segmentCount * 2;
    if (segmentCount == 0)
        return;
    mesh.vertices = static_cast<float *>(MemAlloc(
        static_cast<unsigned int>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.normals = static_cast<float *>(MemAlloc(
        static_cast<unsigned int>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.colors = static_cast<unsigned char *>(MemAlloc(
        static_cast<unsigned int>(mesh.vertexCount) * 4
        * sizeof(unsigned char)));
    mesh.indices = static_cast<unsigned short *>(MemAlloc(
        static_cast<unsigned int>(mesh.triangleCount) * 3
        * sizeof(unsigned short)));
}

Vector3 TrackRendererDetail::makeDashedQuadPoint(
    const Track &track, const std::vector<Vector2> &perp,
    size_t idx, float innerOffset, float outerOffset, float yHeight,
    bool outer)
{
    const auto &wp = track.Waypoints();
    float offset = outer ? outerOffset : innerOffset;

    return Vector3{
        wp[idx].x + perp[idx].x * offset, yHeight,
        wp[idx].y + perp[idx].y * offset,
    };
}

Mesh TrackRendererDetail::buildDashedStripMesh(
    const Track &track, const std::vector<Vector2> &perp,
    float innerOffset, float outerOffset, float yHeight,
    Color color, int dashPeriod, int dashOn)
{
    size_t n = track.Waypoints().size();
    int segmentCount = countDashSegments(n, dashPeriod, dashOn);
    Mesh mesh{};

    allocDashedStripMesh(mesh, segmentCount);
    if (segmentCount == 0)
        return mesh;
    fillDashedStripLoop(
        mesh, track, perp, n, innerOffset, outerOffset, yHeight,
        color, dashPeriod, dashOn);
    UploadMesh(&mesh, false);
    return mesh;
}

void TrackRendererDetail::fillDashedStripLoop(
    Mesh &mesh, const Track &track, const std::vector<Vector2> &perp,
    size_t n, float innerOffset, float outerOffset, float yHeight,
    Color color, int dashPeriod, int dashOn)
{
    int quadIdx = 0;

    for (size_t i = 0; i < n; ++i) {
        if (static_cast<int>(i % static_cast<size_t>(dashPeriod)) >= dashOn)
            continue;
        size_t j = (i + 1) % n;
        Vector3 quad[4] = {
            makeDashedQuadPoint(
                track, perp, i, innerOffset, outerOffset, yHeight, false),
            makeDashedQuadPoint(
                track, perp, j, innerOffset, outerOffset, yHeight, false),
            makeDashedQuadPoint(
                track, perp, j, innerOffset, outerOffset, yHeight, true),
            makeDashedQuadPoint(
                track, perp, i, innerOffset, outerOffset, yHeight, true),
        };

        writeDashedSegment(mesh, quadIdx, quad, color);
        ++quadIdx;
    }
}

void TrackRendererDetail::allocCheckerMesh(Mesh &mesh, int tilesPerSide)
{
    mesh.vertexCount = tilesPerSide * tilesPerSide * 4;
    mesh.triangleCount = tilesPerSide * tilesPerSide * 2;
    mesh.vertices = static_cast<float *>(MemAlloc(
        static_cast<unsigned int>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.normals = static_cast<float *>(MemAlloc(
        static_cast<unsigned int>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.colors = static_cast<unsigned char *>(MemAlloc(
        static_cast<unsigned int>(mesh.vertexCount) * 4
        * sizeof(unsigned char)));
    mesh.indices = static_cast<unsigned short *>(MemAlloc(
        static_cast<unsigned int>(mesh.triangleCount) * 3
        * sizeof(unsigned short)));
}

void TrackRendererDetail::fillCheckerTiles(
    Mesh &mesh, int tilesPerSide, float tileSize, float half,
    Color greenA, Color greenB)
{
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

            writeCheckerTile(mesh, quadIdx, quad, c);
            ++quadIdx;
        }
    }
}

Mesh TrackRendererDetail::buildCheckerGroundMesh(
    float totalSize, int tilesPerSide, SurfaceStyle style)
{
    float tileSize = totalSize / static_cast<float>(tilesPerSide);
    float half = totalSize * 0.5f;
    Mesh mesh{};
    Color greenA = (style == SurfaceStyle::Abimee)
        ? Color{118, 108, 62, 255} : Color{58, 130, 58, 255};
    Color greenB = (style == SurfaceStyle::Abimee)
        ? Color{108, 98, 55, 255} : Color{50, 118, 50, 255};

    allocCheckerMesh(mesh, tilesPerSide);
    fillCheckerTiles(mesh, tilesPerSide, tileSize, half, greenA, greenB);
    UploadMesh(&mesh, false);
    return mesh;
}

Mesh TrackRendererDetail::buildFinishLineMesh(
    const Track &track, const std::vector<Vector2> &perp)
{
    constexpr int kCols = 6;
    const auto &wp = track.Waypoints();
    Vector2 base = wp[0];
    Vector2 p = perp[0];
    Vector2 dir{-p.y, p.x};
    float halfWidth = track.Width() * 0.5f;
    Mesh mesh{};

    allocFinishLineMesh(mesh, kCols);
    fillFinishLineColumns(mesh, base, p, dir, halfWidth);
    UploadMesh(&mesh, false);
    return mesh;
}

void TrackRendererDetail::allocFinishLineMesh(Mesh &mesh, int colCount)
{
    mesh.vertexCount = colCount * 4;
    mesh.triangleCount = colCount * 2;
    mesh.vertices = static_cast<float *>(MemAlloc(
        static_cast<unsigned int>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.normals = static_cast<float *>(MemAlloc(
        static_cast<unsigned int>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.colors = static_cast<unsigned char *>(MemAlloc(
        static_cast<unsigned int>(mesh.vertexCount) * 4
        * sizeof(unsigned char)));
    mesh.indices = static_cast<unsigned short *>(MemAlloc(
        static_cast<unsigned int>(mesh.triangleCount) * 3
        * sizeof(unsigned short)));
}

void TrackRendererDetail::makeFinishColumnQuad(
    Vector3 quad[4], Vector2 base, Vector2 p, Vector2 dir,
    float halfWidth, float t0, float t1)
{
    constexpr float kLineDepth = 2.5f;

    quad[0] = Vector3{
        base.x + p.x * t0 - dir.x * kLineDepth * 0.5f, 0.03f,
        base.y + p.y * t0 - dir.y * kLineDepth * 0.5f,
    };
    quad[1] = Vector3{
        base.x + p.x * t0 + dir.x * kLineDepth * 0.5f, 0.03f,
        base.y + p.y * t0 + dir.y * kLineDepth * 0.5f,
    };
    quad[2] = Vector3{
        base.x + p.x * t1 + dir.x * kLineDepth * 0.5f, 0.03f,
        base.y + p.y * t1 + dir.y * kLineDepth * 0.5f,
    };
    quad[3] = Vector3{
        base.x + p.x * t1 - dir.x * kLineDepth * 0.5f, 0.03f,
        base.y + p.y * t1 - dir.y * kLineDepth * 0.5f,
    };
}

void TrackRendererDetail::fillFinishLineColumns(
    Mesh &mesh, Vector2 base, Vector2 p, Vector2 dir, float halfWidth)
{
    constexpr int kCols = 6;

    for (int c = 0; c < kCols; ++c) {
        float t0 = -halfWidth
            + (static_cast<float>(c) / kCols) * (2.0f * halfWidth);
        float t1 = -halfWidth
            + (static_cast<float>(c + 1) / kCols) * (2.0f * halfWidth);
        Vector3 quad[4];

        makeFinishColumnQuad(quad, base, p, dir, halfWidth, t0, t1);
        writeFinishColumn(mesh, c, quad, (c % 2 == 0) ? WHITE : BLACK);
    }
}

void TrackRendererDetail::drawPropShadow(Vector3 pos, float radius)
{
    DrawCylinder(
        Vector3{pos.x, 0.015f, pos.z}, radius, radius, 0.02f, 10,
        Fade(BLACK, 0.28f));
}

void TrackRendererDetail::drawMountainSegment(
    int i, int segments, float radius, float baseY,
    Color farColor, Color nearColor)
{
    float a0 = (static_cast<float>(i) / segments) * 2.0f * PI;
    float a1 = (static_cast<float>(i + 1) / segments) * 2.0f * PI;
    float h0 = 8.0f + 12.0f * std::fabs(std::sin(a0 * 3.7f));
    float h1 = 8.0f + 12.0f * std::fabs(std::sin(a1 * 3.7f));
    Vector3 p0{std::cos(a0) * radius, baseY, std::sin(a0) * radius};
    Vector3 p1{std::cos(a1) * radius, baseY, std::sin(a1) * radius};
    float midA = (a0 + a1) * 0.5f;
    Vector3 peak0{
        std::cos(midA) * (radius - 15.0f), baseY + (h0 + h1) * 0.5f,
        std::sin(midA) * (radius - 15.0f),
    };

    DrawTriangle3D(p0, peak0, p1, (i % 2 == 0) ? farColor : nearColor);
}

void TrackRendererDetail::drawMountainsRing(
    float radius, float baseY, int segments)
{
    Color farColor{72, 118, 158, 255};
    Color nearColor{58, 98, 132, 255};

    for (int i = 0; i < segments; ++i)
        drawMountainSegment(i, segments, radius, baseY, farColor, nearColor);
}

void TrackRendererDetail::appendQuad(
    MeshBuffers &mb, Vector3 p1, Vector3 p2, Vector3 p3, Vector3 p4,
    Vector3 normal, Color color)
{
    auto base = static_cast<unsigned short>(mb.vertices.size() / 3);
    const Vector3 pts[4] = {p1, p2, p3, p4};

    for (const Vector3 &pt : pts) {
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

    for (unsigned short off : quadIdx)
        mb.indices.push_back(static_cast<unsigned short>(base + off));
}

Vector3 TrackRendererDetail::boxCorner(
    Vector3 fc, Vector3 u, Vector3 v, float hu, float hv,
    float su, float sv)
{
    return Vector3{
        fc.x + u.x * su * hu + v.x * sv * hv,
        fc.y + u.y * su * hu + v.y * sv * hv,
        fc.z + u.z * su * hu + v.z * sv * hv,
    };
}

void TrackRendererDetail::appendBoxFace(
    MeshBuffers &mb, Vector3 center, Vector3 fn, Vector3 u, Vector3 v,
    float hu, float hv, float offN, Color color)
{
    Vector3 fc{
        center.x + fn.x * offN, center.y + fn.y * offN,
        center.z + fn.z * offN,
    };

    appendQuad(
        mb,
        boxCorner(fc, u, v, hu, hv, -1.0f, -1.0f),
        boxCorner(fc, u, v, hu, hv, 1.0f, -1.0f),
        boxCorner(fc, u, v, hu, hv, 1.0f, 1.0f),
        boxCorner(fc, u, v, hu, hv, -1.0f, 1.0f),
        fn, color);
}

void TrackRendererDetail::appendBox(
    MeshBuffers &mb, Vector3 center, Vector3 ax, Vector3 ay, Vector3 az,
    float hx, float hy, float hz, Color color)
{
    Vector3 nax{-ax.x, -ax.y, -ax.z};
    Vector3 nay{-ay.x, -ay.y, -ay.z};
    Vector3 naz{-az.x, -az.y, -az.z};

    appendBoxFace(mb, center, ax, ay, az, hy, hz, hx, color);
    appendBoxFace(mb, center, nax, az, ay, hz, hy, hx, color);
    appendBoxFace(mb, center, ay, az, ax, hz, hx, hy, color);
    appendBoxFace(mb, center, nay, ax, az, hx, hz, hy, color);
    appendBoxFace(mb, center, az, ax, ay, hx, hy, hz, color);
    appendBoxFace(mb, center, naz, ay, ax, hy, hx, hz, color);
}

void TrackRendererDetail::copyMeshBufferData(
    Mesh &mesh, const MeshBuffers &mb)
{
    const size_t vBytes = mb.vertices.size() * sizeof(float);
    const size_t nBytes = mb.normals.size() * sizeof(float);
    const size_t cBytes = mb.colors.size() * sizeof(unsigned char);
    const size_t iBytes = mb.indices.size() * sizeof(unsigned short);

    mesh.vertices = static_cast<float *>(MemAlloc(
        static_cast<unsigned int>(vBytes)));
    mesh.normals = static_cast<float *>(MemAlloc(
        static_cast<unsigned int>(nBytes)));
    mesh.colors = static_cast<unsigned char *>(MemAlloc(
        static_cast<unsigned int>(cBytes)));
    mesh.indices = static_cast<unsigned short *>(MemAlloc(
        static_cast<unsigned int>(iBytes)));
    std::memcpy(mesh.vertices, mb.vertices.data(), vBytes);
    std::memcpy(mesh.normals, mb.normals.data(), nBytes);
    std::memcpy(mesh.colors, mb.colors.data(), cBytes);
    std::memcpy(mesh.indices, mb.indices.data(), iBytes);
}

Mesh TrackRendererDetail::meshFromBuffers(const MeshBuffers &mb)
{
    Mesh mesh{};

    mesh.vertexCount = static_cast<int>(mb.vertices.size() / 3);
    mesh.triangleCount = static_cast<int>(mb.indices.size() / 3);
    if (mesh.vertexCount == 0 || mesh.triangleCount == 0)
        return mesh;
    copyMeshBufferData(mesh, mb);
    UploadMesh(&mesh, false);
    return mesh;
}

void TrackRendererDetail::writeSkidQuadCorner(
    Mesh &mesh, int k, float x, float z, float y, float u, float v)
{
    mesh.vertices[k * 3 + 0] = x;
    mesh.vertices[k * 3 + 1] = y;
    mesh.vertices[k * 3 + 2] = z;
    mesh.normals[k * 3 + 0] = 0.0f;
    mesh.normals[k * 3 + 1] = 1.0f;
    mesh.normals[k * 3 + 2] = 0.0f;
    mesh.texcoords[k * 2 + 0] = u;
    mesh.texcoords[k * 2 + 1] = v;
}

void TrackRendererDetail::fillSkidQuadVertices(
    Mesh &mesh, Vector2 origin, float size, float y)
{
    const float xs[4] = {
        origin.x, origin.x, origin.x + size, origin.x + size,
    };
    const float zs[4] = {
        origin.y, origin.y + size, origin.y + size, origin.y,
    };
    const float us[4] = {0.0f, 0.0f, 1.0f, 1.0f};
    const float vs[4] = {0.0f, 1.0f, 1.0f, 0.0f};

    for (int k = 0; k < 4; ++k)
        writeSkidQuadCorner(mesh, k, xs[k], zs[k], y, us[k], vs[k]);
    const unsigned short idx[6] = {0, 1, 2, 0, 2, 3};

    for (int k = 0; k < 6; ++k)
        mesh.indices[k] = idx[k];
}

Mesh TrackRendererDetail::buildSkidQuadMesh(
    Vector2 origin, float size, float y)
{
    Mesh mesh{};

    mesh.vertexCount = 4;
    mesh.triangleCount = 2;
    mesh.vertices = static_cast<float *>(MemAlloc(
        static_cast<unsigned int>(4 * 3 * sizeof(float))));
    mesh.normals = static_cast<float *>(MemAlloc(
        static_cast<unsigned int>(4 * 3 * sizeof(float))));
    mesh.texcoords = static_cast<float *>(MemAlloc(
        static_cast<unsigned int>(4 * 2 * sizeof(float))));
    mesh.indices = static_cast<unsigned short *>(MemAlloc(
        static_cast<unsigned int>(6 * sizeof(unsigned short))));
    fillSkidQuadVertices(mesh, origin, size, y);
    UploadMesh(&mesh, false);
    return mesh;
}

void TrackRendererDetail::applyShaderToModel(Model &model, Shader shader)
{
    for (int i = 0; i < model.materialCount; ++i)
        model.materials[i].shader = shader;
}

Vector2 TrackRendererDetail::worldToSkidTex(
    float worldX, float worldZ, Vector2 origin, float size)
{
    float u = (worldX - origin.x) / size;
    float v = 1.0f - (worldZ - origin.y) / size;

    return Vector2{
        u * static_cast<float>(kSkidTextureSize),
        v * static_cast<float>(kSkidTextureSize),
    };
}

} // namespace

struct TrackRendererBuild {
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
        TrackRenderer::CloudInstance &cloud, int cloudIndex);
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
        TrackRenderer::GrandstandInstance &gs, size_t mid);
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
        TrackRenderer::GrandstandInstance &gs, size_t mid,
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
        TrackRendererDetail::MeshBuffers &barrierBuf, Vector3 pos,
        Vector3 along, Vector3 out, SurfaceStyle style);
    static void addSponsorAtWaypoint(
        TrackRendererDetail::MeshBuffers &sponsorBuf,
        const Track &track, const std::vector<Vector2> &perp,
        float halfWidth, size_t i);
    static void setupLampTop(
        TrackRenderer::LampInstance &lamp, const Vector2 &perp,
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
        TrackRendererDetail::MeshBuffers &barrierBuf, const Track &track,
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
        TrackRenderer::PropInstance &prop, uint32_t h);
    static void fillTreeProp(
        TrackRenderer::PropInstance &prop, uint32_t h, bool isDeadTree);
    static TrackRenderer::GrandstandInstance makeGrandstandInstance(
        const Track &track, float halfWidth, size_t mid,
        Vector2 along, Vector2 outward, float alongLen);
    static void scanWaypointBounds(
        const Track &track, float &minX, float &maxX,
        float &minZ, float &maxZ);
    static void addAbimeeCrackAt(
        TrackRenderer &renderer, const TrackRenderer::PotholeInstance &hole,
        const std::vector<Vector2> &perp, size_t i, uint32_t h);
    static void appendSponsorPanel(
        TrackRendererDetail::MeshBuffers &sponsorBuf, Vector3 base,
        Vector3 face, Vector3 right, Color panelColor);
    static Vector3 barrierCornerPos(
        const Track &track, const std::vector<Vector2> &perp,
        float halfWidth, size_t i, float side);
    static Color makeSpectatorShirtColor(uint32_t sh);
    static Vector3 makeSpectatorPosition(
        const TrackRenderer::GrandstandInstance &gs,
        float rowH, float alongT, float outwardD);
    static bool computeStraightAlong(
        const Track &track, size_t runStart, size_t runEnd,
        Vector2 &along, size_t &mid, float &alongLen);
    static float straightRunDirectionDot(
        const Track &track, size_t i, size_t n);
    static bool isGrandstandRunEnd(
        const Track &track, size_t i, size_t n);
    static void buildSceneMeshes(
        TrackRenderer::LampInstance &lamp, const Vector2 &perp,
        float sideSign, bool broken);
    static void buildSceneMeshes(
        TrackRenderer &renderer, const Track &track,
        const std::vector<Vector2> &perp, float halfWidth, size_t n);
    static void buildSceneDecor(
        TrackRenderer &renderer, const Track &track,
        const std::vector<Vector2> &perp, float halfWidth, size_t n);
};

struct TrackRendererDraw {
    static void drawClouds(
        const TrackRenderer &renderer, float timeSeconds);
    static void drawOneCloud(
        const TrackRenderer::CloudInstance &cloud, float timeSeconds);
    static void drawLamps(const TrackRenderer &renderer);
    static void drawOneLamp(const TrackRenderer::LampInstance &lamp);
    static void drawArch(const TrackRenderer &renderer);
    static void drawArchSpan(const TrackRenderer &renderer);
    static void drawRoadDamage(const TrackRenderer &renderer);
    static void drawAbimeeDebris(const TrackRenderer &renderer);
    static void drawStartGantry(
        const TrackRenderer &renderer, float timeSeconds);
    static void drawGantryBeam(
        Vector3 leftBase, Vector3 rightBase, Vector3 p, float hw,
        float pillarH);
    static void drawGantryCheckerCells(
        Vector3 beamMid, Vector3 p, float hw, float pillarH);
    static void drawGrandstandRows(
        const TrackRenderer::GrandstandInstance &gs);
    static void drawGrandstandRoof(
        const TrackRenderer::GrandstandInstance &gs);
    static void drawOneSkidMark(
        const TrackRenderer::SkidMarkCmd &cmd, Vector2 origin, float worldSize);
    static void drawGantryLights(
        float timeSeconds, Vector3 leftBase, float pillarH, Vector3 d);
    static void drawGrandstandStructure(const TrackRenderer &renderer);
    static void drawOneGrandstand(
        const TrackRenderer::GrandstandInstance &gs);
    static void drawSpectators(
        const TrackRenderer &renderer, float timeSeconds);
    static void drawOneSpectator(
        const TrackRenderer::SpectatorInstance &spec, float timeSeconds);
    static void drawProps(const TrackRenderer &renderer);
    static void drawOneProp(const TrackRenderer::PropInstance &prop);
    static void drawBuildingProp(const TrackRenderer::PropInstance &prop);
    static void drawDeadTreeProp(const TrackRenderer::PropInstance &prop);
    static void drawLiveTreeProp(const TrackRenderer::PropInstance &prop);
    static void drawTireStacks(const TrackRenderer &renderer);
    static void drawPennants(
        const TrackRenderer &renderer, float timeSeconds);
    static void drawOnePennant(
        const TrackRenderer::PennantInstance &pen, float timeSeconds);
    static void drawNpcs(
        const TrackRenderer &renderer, float timeSeconds);
    static void drawPotholes(const TrackRenderer &renderer);
    static void drawCracks(const TrackRenderer &renderer);
    static void drawNpcBody(
        const TrackRenderer::NpcInstance &npc, float armWave);
    static void drawNpcFlag(
        const TrackRenderer::NpcInstance &npc, float armWave);
    static void drawOneNpc(
        const TrackRenderer::NpcInstance &npc, float timeSeconds);
};

void TrackRendererBuild::buildTrackMeshes(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth)
{
    SurfaceStyle style = renderer.surfaceStyle_;
    Mesh trackMesh = TrackRendererDetail::buildStripMesh(
        track, perp, -halfWidth, halfWidth, 0.02f,
        [style](size_t i) {
            return TrackRendererDetail::asphaltColor(
                TrackRendererDetail::hashIndex(i), style);
        });
    renderer.trackModel_ = LoadModelFromMesh(trackMesh);
    Mesh rubberMesh = TrackRendererDetail::buildStripMesh(
        track, perp, -0.35f, 0.35f, 0.021f,
        [](size_t) { return Color{28, 28, 32, 255}; });
    renderer.rubberLineModel_ = LoadModelFromMesh(rubberMesh);
    Mesh centerDash = TrackRendererDetail::buildDashedStripMesh(
        track, perp, -0.12f, 0.12f, 0.035f, WHITE, 4, 2);
    renderer.centerDashModel_ = LoadModelFromMesh(centerDash);
}

void TrackRendererBuild::buildEdgeLineMeshes(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth)
{
    constexpr float kEdgeWidth = 0.18f;
    Mesh edgeOuter = TrackRendererDetail::buildStripMesh(
        track, perp, halfWidth - kEdgeWidth, halfWidth - 0.02f, 0.034f,
        [](size_t) { return WHITE; });
    renderer.edgeLineOuterModel_ = LoadModelFromMesh(edgeOuter);
    Mesh edgeInner = TrackRendererDetail::buildStripMesh(
        track, perp, -halfWidth + 0.02f, -halfWidth + kEdgeWidth, 0.034f,
        [](size_t) { return WHITE; });
    renderer.edgeLineInnerModel_ = LoadModelFromMesh(edgeInner);
}

void TrackRendererBuild::buildCurbMeshes(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth)
{
    constexpr float kCurbWidth = 1.4f;
    Color curbA = (renderer.surfaceStyle_ == SurfaceStyle::Abimee)
        ? Color{180, 170, 150, 255} : RED;
    Color curbB = (renderer.surfaceStyle_ == SurfaceStyle::Abimee)
        ? Color{160, 150, 130, 255} : RAYWHITE;
    Mesh curbMeshOuter = TrackRendererDetail::buildStripMesh(
        track, perp, halfWidth - kCurbWidth * 0.5f,
        halfWidth + kCurbWidth * 0.5f, 0.025f,
        [curbA, curbB](size_t i) {
            return (static_cast<int>(i / 3) % 2 == 0) ? curbA : curbB;
        });
    renderer.curbModelOuter_ = LoadModelFromMesh(curbMeshOuter);
    Mesh curbMeshInner = TrackRendererDetail::buildStripMesh(
        track, perp, -halfWidth - kCurbWidth * 0.5f,
        -halfWidth + kCurbWidth * 0.5f, 0.025f,
        [curbA, curbB](size_t i) {
            return (static_cast<int>(i / 3) % 2 == 0) ? curbA : curbB;
        });
    renderer.curbModelInner_ = LoadModelFromMesh(curbMeshInner);
}

void TrackRendererBuild::buildGroundAndFinishMeshes(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp)
{
    Mesh groundMesh = TrackRendererDetail::buildCheckerGroundMesh(
        500.0f, 40, renderer.surfaceStyle_);
    renderer.groundModel_ = LoadModelFromMesh(groundMesh);
    Mesh finishMesh = TrackRendererDetail::buildFinishLineMesh(track, perp);
    renderer.finishLineModel_ = LoadModelFromMesh(finishMesh);
}

void TrackRendererBuild::buildGroundAndFinish(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp)
{
    float halfWidth = renderer.trackHalfWidth_;

    buildEdgeLineMeshes(renderer, track, perp, halfWidth);
    buildCurbMeshes(renderer, track, perp, halfWidth);
    buildGroundAndFinishMeshes(renderer, track, perp);
}

void TrackRendererBuild::initStartGantry(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp)
{
    const auto &wp = track.Waypoints();

    renderer.startGantryBase_ = Vector3{wp[0].x, 0.0f, wp[0].y};
    renderer.startGantryPerp_ = Vector3{perp[0].x, 0.0f, perp[0].y};
    renderer.startGantryAlong_ = Vector3{-perp[0].y, 0.0f, perp[0].x};
}

void TrackRendererBuild::buildCloudPuffs(
    TrackRenderer::CloudInstance &cloud, int cloudIndex)
{
    uint32_t h = TrackRendererDetail::hashIndex(
        static_cast<size_t>(cloudIndex) + 9000);
    int puffCount = 3 + static_cast<int>(h % 3);

    for (int p = 0; p < puffCount; ++p) {
        uint32_t ph = TrackRendererDetail::hashIndex(
            static_cast<size_t>(cloudIndex * 10 + p));
        cloud.puffOffsets.push_back(Vector3{
            static_cast<float>(ph % 100) * 0.08f - 4.0f,
            static_cast<float>((ph >> 4) % 50) * 0.02f,
            static_cast<float>((ph >> 8) % 100) * 0.08f - 4.0f,
        });
        cloud.puffScales.push_back(
            1.8f + static_cast<float>(ph % 40) * 0.04f);
    }
}

void TrackRendererBuild::buildCloudRing(TrackRenderer &renderer)
{
    for (int c = 0; c < 14; ++c) {
        uint32_t h = TrackRendererDetail::hashIndex(
            static_cast<size_t>(c) + 9000);
        float angle = static_cast<float>(c) / 14.0f * 2.0f * PI;
        float dist = 120.0f + static_cast<float>(h % 80);
        TrackRenderer::CloudInstance cloud;

        cloud.basePosition = Vector3{
            std::cos(angle) * dist, 42.0f + static_cast<float>(h % 20),
            std::sin(angle) * dist,
        };
        cloud.driftSpeed = 0.4f + static_cast<float>(h % 50) * 0.01f;
        cloud.scale = 2.5f + static_cast<float>(h % 30) * 0.05f;
        buildCloudPuffs(cloud, c);
        renderer.clouds_.push_back(cloud);
    }
}

Color TrackRendererBuild::makeSpectatorShirtColor(uint32_t sh)
{
    return Color{
        static_cast<unsigned char>(80 + sh % 175),
        static_cast<unsigned char>(60 + (sh >> 4) % 175),
        static_cast<unsigned char>(50 + (sh >> 8) % 175),
        255,
    };
}

Vector3 TrackRendererBuild::makeSpectatorPosition(
    const TrackRenderer::GrandstandInstance &gs,
    float rowH, float alongT, float outwardD)
{
    return Vector3{
        gs.origin.x + gs.along.x * alongT + gs.outward.x * outwardD,
        rowH,
        gs.origin.z + gs.along.z * alongT + gs.outward.z * outwardD,
    };
}

void TrackRendererBuild::addGrandstandSpectator(
    TrackRenderer::GrandstandInstance &gs, size_t mid,
    int row, int col)
{
    constexpr int kCols = 14;
    uint32_t sh = TrackRendererDetail::hashIndex(
        static_cast<size_t>(row * kCols + col) + mid * 17);
    float alongT = (static_cast<float>(col) / (kCols - 1) - 0.5f) * gs.length;
    float outwardD = static_cast<float>(row) * 1.8f + 1.5f;
    float rowH = static_cast<float>(row) * 1.1f + 0.5f;
    TrackRenderer::SpectatorInstance spec;

    spec.position = makeSpectatorPosition(gs, rowH, alongT, outwardD);
    spec.shirtColor = makeSpectatorShirtColor(sh);
    spec.jumpPhase = static_cast<float>(sh % 628) * 0.01f;
    spec.jumpSpeed = 2.5f + static_cast<float>(sh % 30) * 0.05f;
    gs.spectators.push_back(spec);
}

void TrackRendererBuild::fillGrandstandSpectators(
    TrackRenderer::GrandstandInstance &gs, size_t mid)
{
    constexpr int kRows = 4;
    constexpr int kCols = 14;

    for (int row = 0; row < kRows; ++row) {
        for (int col = 0; col < kCols; ++col)
            addGrandstandSpectator(gs, mid, row, col);
    }
}

void TrackRendererBuild::fillBuildingProp(
    TrackRenderer::PropInstance &prop, uint32_t h)
{
    prop.type = 1;
    prop.heightScale = 1.0f + static_cast<float>(h % 100) * 0.03f;
    prop.color = Color{
        static_cast<unsigned char>(90 + h % 60),
        static_cast<unsigned char>(90 + h % 60),
        static_cast<unsigned char>(100 + h % 60),
        255,
    };
}

void TrackRendererBuild::fillTreeProp(
    TrackRenderer::PropInstance &prop, uint32_t h, bool isDeadTree)
{
    prop.type = isDeadTree ? 2 : 0;
    prop.heightScale = isDeadTree
        ? (0.7f + static_cast<float>(h % 50) * 0.004f)
        : (0.8f + static_cast<float>(h % 100) * 0.006f);
    prop.color = isDeadTree
        ? Color{92, 72, 48, 255}
        : Color{34, static_cast<unsigned char>(110 + h % 50), 34, 255};
}

void TrackRendererBuild::addWaypointProp(
    TrackRenderer &renderer, Vector3 pos, uint32_t h, SurfaceStyle style)
{
    bool isBuilding = (h % 5 == 0) && style != SurfaceStyle::Abimee;
    bool isDeadTree = style == SurfaceStyle::Abimee && (h % 4 != 0);
    TrackRenderer::PropInstance prop;

    prop.position = pos;
    if (isBuilding)
        fillBuildingProp(prop, h);
    else
        fillTreeProp(prop, h, isDeadTree);
    renderer.props_.push_back(prop);
}

void TrackRendererBuild::addWaypointNpc(
    TrackRenderer &renderer, Vector3 pos,
    const Vector2 &perp, float sideSign, uint32_t h)
{
    TrackRenderer::NpcInstance npc;

    npc.position = Vector3{
        pos.x + perp.x * 1.5f, 0.0f, pos.z + perp.y * 1.5f,
    };
    npc.heading = std::atan2(perp.x * sideSign, perp.y * sideSign);
    npc.shirtColor = Color{
        static_cast<unsigned char>(100 + h % 120),
        static_cast<unsigned char>(80 + (h >> 3) % 120),
        static_cast<unsigned char>(70 + (h >> 6) % 120),
        255,
    };
    npc.flagColor = Color{
        static_cast<unsigned char>(150 + h % 100),
        static_cast<unsigned char>(40 + (h >> 4) % 80),
        static_cast<unsigned char>(40 + (h >> 7) % 80),
        255,
    };
    npc.animPhase = static_cast<float>(h % 628) * 0.01f;
    renderer.npcs_.push_back(npc);
}

void TrackRendererBuild::addWaypointTireStack(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth,
    float sideSign, size_t i, uint32_t h)
{
    const auto &wp = track.Waypoints();
    TrackRenderer::TireStackInstance stack;

    stack.position = Vector3{
        wp[i].x + perp[i].x * (halfWidth + 2.2f) * sideSign, 0.0f,
        wp[i].y + perp[i].y * (halfWidth + 2.2f) * sideSign,
    };
    stack.tiers = 2 + static_cast<int>(h % 3);
    renderer.tireStacks_.push_back(stack);
}

void TrackRendererBuild::addWaypointPennant(
    TrackRenderer &renderer, Vector3 pos, uint32_t h)
{
    TrackRenderer::PennantInstance pen;

    pen.base = Vector3{pos.x, 0.0f, pos.z};
    pen.top = Vector3{pos.x, 5.5f, pos.z};
    pen.color = Color{
        static_cast<unsigned char>(180 + h % 60),
        static_cast<unsigned char>(50 + (h >> 5) % 150),
        static_cast<unsigned char>(50 + (h >> 9) % 150),
        255,
    };
    pen.phase = static_cast<float>(h % 628) * 0.01f;
    renderer.pennants_.push_back(pen);
}

void TrackRendererBuild::populateWaypointDecor(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth, size_t i)
{
    const auto &wp = track.Waypoints();
    uint32_t h = TrackRendererDetail::hashIndex(i);
    float sideSign = (h % 2 == 0) ? 1.0f : -1.0f;
    float extraOffset = 4.0f + static_cast<float>(h % 100) * 0.05f;
    float dist = halfWidth + extraOffset;
    Vector3 pos{
        wp[i].x + perp[i].x * dist * sideSign, 0.0f,
        wp[i].y + perp[i].y * dist * sideSign,
    };

    addWaypointProp(renderer, pos, h, renderer.surfaceStyle_);
    if (h % 7 == 0)
        addWaypointNpc(renderer, pos, perp[i], sideSign, h);
    if (h % 11 == 0)
        addWaypointTireStack(
            renderer, track, perp, halfWidth, sideSign, i, h);
    if (h % 13 == 0)
        addWaypointPennant(renderer, pos, h);
}

TrackRenderer::GrandstandInstance TrackRendererBuild::makeGrandstandInstance(
    const Track &track, float halfWidth, size_t mid,
    Vector2 along, Vector2 outward, float alongLen)
{
    const auto &wp = track.Waypoints();
    TrackRenderer::GrandstandInstance gs;

    gs.origin = Vector3{
        wp[mid].x + outward.x * (halfWidth + 10.0f), 0.0f,
        wp[mid].y + outward.y * (halfWidth + 10.0f),
    };
    gs.along = Vector3{along.x, 0.0f, along.y};
    gs.outward = Vector3{outward.x, 0.0f, outward.y};
    gs.length = alongLen * 0.7f;
    return gs;
}

bool TrackRendererBuild::computeStraightAlong(
    const Track &track, size_t runStart, size_t runEnd,
    Vector2 &along, size_t &mid, float &alongLen)
{
    const auto &wp = track.Waypoints();

    if (runEnd <= runStart + 8)
        return false;
    mid = (runStart + runEnd) / 2;
    along = Vector2{
        wp[runEnd].x - wp[runStart].x, wp[runEnd].y - wp[runStart].y,
    };
    alongLen = std::sqrt(along.x * along.x + along.y * along.y);
    if (alongLen <= 30.0f)
        return false;
    along.x /= alongLen;
    along.y /= alongLen;
    return true;
}

void TrackRendererBuild::tryAddGrandstand(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth,
    size_t runStart, size_t runEnd)
{
    const auto &wp = track.Waypoints();
    Vector2 along{};
    size_t mid = 0;
    float alongLen = 0.0f;

    (void)perp;
    if (!computeStraightAlong(track, runStart, runEnd, along, mid, alongLen))
        return;
    Vector2 outward{-along.y, along.x};
    float side = (wp[mid].x * outward.x + wp[mid].y * outward.y) > 0.0f
        ? 1.0f : -1.0f;

    outward.x *= side;
    outward.y *= side;
    TrackRenderer::GrandstandInstance gs = makeGrandstandInstance(
        track, halfWidth, mid, along, outward, alongLen);

    fillGrandstandSpectators(gs, mid);
    renderer.grandstands_.push_back(gs);
}

float TrackRendererBuild::straightRunDirectionDot(
    const Track &track, size_t i, size_t n)
{
    const auto &wp = track.Waypoints();
    Vector2 d0{wp[i].x - wp[i - 1].x, wp[i].y - wp[i - 1].y};
    Vector2 d1{wp[i + 1].x - wp[i].x, wp[i + 1].y - wp[i].y};
    float l0 = std::sqrt(d0.x * d0.x + d0.y * d0.y);
    float l1 = std::sqrt(d1.x * d1.x + d1.y * d1.y);

    if (l0 <= 1e-4f || l1 <= 1e-4f)
        return -2.0f;
    d0.x /= l0;
    d0.y /= l0;
    d1.x /= l1;
    d1.y /= l1;
    return d0.x * d1.x + d0.y * d1.y;
}

bool TrackRendererBuild::shouldEndStraightRun(
    const Track &track, size_t i, size_t n, bool &endRun)
{
    constexpr float kStraightDotThreshold = 0.995f;

    if (i + 1 >= n)
        return true;
    float dot = straightRunDirectionDot(track, i, n);

    if (dot < -1.5f) {
        endRun = true;
        return true;
    }
    if (dot >= kStraightDotThreshold)
        return false;
    endRun = true;
    return true;
}

bool TrackRendererBuild::isGrandstandRunEnd(
    const Track &track, size_t i, size_t n)
{
    if (i == n)
        return true;
    bool dummy = false;

    if (shouldEndStraightRun(track, i, n, dummy))
        return true;
    return false;
}

void TrackRendererBuild::buildGrandstands(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth)
{
    size_t n = track.Waypoints().size();
    size_t runStart = 0;

    for (size_t i = 1; i <= n; ++i) {
        if (!isGrandstandRunEnd(track, i, n))
            continue;
        size_t runEnd = (i == n) ? n - 1 : i;

        tryAddGrandstand(renderer, track, perp, halfWidth, runStart, runEnd);
        runStart = i;
    }
}

void TrackRendererBuild::addAbimeeCrackAt(
    TrackRenderer &renderer, const TrackRenderer::PotholeInstance &hole,
    const std::vector<Vector2> &perp, size_t i, uint32_t h)
{
    TrackRenderer::CrackInstance crack;

    crack.center = hole.position;
    crack.tangent = Vector3{perp[i].y, 0.0f, -perp[i].x};
    crack.length = 1.2f + static_cast<float>(h % 30) * 0.05f;
    renderer.cracks_.push_back(crack);
}

void TrackRendererBuild::addAbimeePotholeAt(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, size_t i, uint32_t h)
{
    const auto &wp = track.Waypoints();
    float t = static_cast<float>(h % 1000) / 1000.0f;
    size_t j = (i + 1) % wp.size();
    Vector2 p2{
        wp[i].x + (wp[j].x - wp[i].x) * t,
        wp[i].y + (wp[j].y - wp[i].y) * t,
    };
    float lateral = (static_cast<float>((h >> 10) % 100) / 100.0f - 0.5f)
        * track.Width() * 0.7f;
    TrackRenderer::PotholeInstance hole;

    hole.position = Vector3{
        p2.x + perp[i].x * lateral, 0.025f,
        p2.y + perp[i].y * lateral,
    };
    hole.radius = 0.35f + static_cast<float>(h % 40) * 0.015f;
    renderer.potholes_.push_back(hole);
    if (h % 5 == 0)
        addAbimeeCrackAt(renderer, hole, perp, i, h);
}

void TrackRendererBuild::buildAbimeeDamage(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp)
{
    const auto &wp = track.Waypoints();

    if (renderer.surfaceStyle_ != SurfaceStyle::Abimee)
        return;
    for (size_t i = 0; i < wp.size(); i += 2) {
        uint32_t h = TrackRendererDetail::hashIndex(i + 50000);

        if (h % 3 != 0)
            continue;
        addAbimeePotholeAt(renderer, track, perp, i, h);
    }
}

void TrackRendererBuild::scanWaypointBounds(
    const Track &track, float &minX, float &maxX,
    float &minZ, float &maxZ)
{
    const auto &wp = track.Waypoints();

    minX = wp[0].x;
    maxX = wp[0].x;
    minZ = wp[0].y;
    maxZ = wp[0].y;
    for (const auto &p : wp) {
        minX = std::min(minX, p.x);
        maxX = std::max(maxX, p.x);
        minZ = std::min(minZ, p.y);
        maxZ = std::max(maxZ, p.y);
    }
}

void TrackRendererBuild::computeSkidBounds(
    TrackRenderer &renderer, const Track &track, float halfWidth)
{
    float minX = 0.0f;
    float maxX = 0.0f;
    float minZ = 0.0f;
    float maxZ = 0.0f;

    scanWaypointBounds(track, minX, maxX, minZ, maxZ);
    constexpr float kSkidMargin = 8.0f;

    minX -= halfWidth + kSkidMargin;
    maxX += halfWidth + kSkidMargin;
    minZ -= halfWidth + kSkidMargin;
    maxZ += halfWidth + kSkidMargin;
    renderer.skidWorldSize_ = std::max(maxX - minX, maxZ - minZ);
    renderer.skidWorldOrigin_ = Vector2{
        (minX + maxX - renderer.skidWorldSize_) * 0.5f,
        (minZ + maxZ - renderer.skidWorldSize_) * 0.5f,
    };
}

void TrackRendererBuild::setupSkidTextureModel(TrackRenderer &renderer)
{
    renderer.skidTexture_ = LoadRenderTexture(
        kSkidTextureSize, kSkidTextureSize);
    BeginTextureMode(renderer.skidTexture_);
    ClearBackground(BLANK);
    EndTextureMode();
    Mesh skidQuad = TrackRendererDetail::buildSkidQuadMesh(
        renderer.skidWorldOrigin_, renderer.skidWorldSize_, 0.045f);
    renderer.skidOverlayModel_ = LoadModelFromMesh(skidQuad);
    if (renderer.skidOverlayModel_.materialCount > 0) {
        renderer.skidOverlayModel_.materials[0]
            .maps[MATERIAL_MAP_DIFFUSE].texture = renderer.skidTexture_.texture;
    }
}

void TrackRendererBuild::initSkidOverlay(
    TrackRenderer &renderer, const Track &track, float halfWidth)
{
    computeSkidBounds(renderer, track, halfWidth);
    setupSkidTextureModel(renderer);
}

bool TrackRendererBuild::isSharpCorner(
    const Track &track, size_t i, size_t n, Vector2 &d0, Vector2 &d1)
{
    const auto &wp = track.Waypoints();
    size_t prev = (i + n - 1) % n;
    size_t next = (i + 1) % n;
    constexpr float kCurveDot = 0.88f;

    d0 = Vector2{wp[i].x - wp[prev].x, wp[i].y - wp[prev].y};
    d1 = Vector2{wp[next].x - wp[i].x, wp[next].y - wp[i].y};
    float l0 = std::sqrt(d0.x * d0.x + d0.y * d0.y);
    float l1 = std::sqrt(d1.x * d1.x + d1.y * d1.y);

    if (l0 < 1e-4f || l1 < 1e-4f)
        return false;
    d0.x /= l0;
    d0.y /= l0;
    d1.x /= l1;
    d1.y /= l1;
    return d0.x * d1.x + d0.y * d1.y < kCurveDot;
}

void TrackRendererBuild::appendBarrierRails(
    TrackRendererDetail::MeshBuffers &barrierBuf, Vector3 pos,
    Vector3 along, Vector3 out, SurfaceStyle style)
{
    Color postColor = (style == SurfaceStyle::Abimee)
        ? Color{120, 80, 55, 255} : Color{150, 150, 158, 255};
    Color railColor = (style == SurfaceStyle::Abimee)
        ? Color{140, 100, 70, 255} : Color{210, 210, 218, 255};

    TrackRendererDetail::appendBox(
        barrierBuf, pos, along, Vector3{0.0f, 1.0f, 0.0f}, out,
        0.08f, 0.35f, 0.08f, postColor);
    TrackRendererDetail::appendBox(
        barrierBuf,
        Vector3{pos.x + along.x * 0.4f, 0.55f, pos.z + along.z * 0.4f},
        along, Vector3{0.0f, 1.0f, 0.0f}, out, 0.55f, 0.06f, 0.05f, railColor);
    TrackRendererDetail::appendBox(
        barrierBuf,
        Vector3{pos.x - along.x * 0.4f, 0.75f, pos.z - along.z * 0.4f},
        along, Vector3{0.0f, 1.0f, 0.0f}, out, 0.55f, 0.06f, 0.05f, railColor);
}

Vector3 TrackRendererBuild::barrierCornerPos(
    const Track &track, const std::vector<Vector2> &perp,
    float halfWidth, size_t i, float side)
{
    const auto &wp = track.Waypoints();

    return Vector3{
        wp[i].x + perp[i].x * (halfWidth + 2.5f) * side, 0.0f,
        wp[i].y + perp[i].y * (halfWidth + 2.5f) * side,
    };
}

void TrackRendererBuild::addBarrierAtCorner(
    TrackRendererDetail::MeshBuffers &barrierBuf, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth,
    SurfaceStyle style, size_t i, size_t n)
{
    Vector2 d0{};
    Vector2 d1{};

    if (!isSharpCorner(track, i, n, d0, d1))
        return;
    const auto &wp = track.Waypoints();
    float side = (perp[i].x * wp[i].x + perp[i].y * wp[i].y) > 0.0f
        ? 1.0f : -1.0f;
    Vector3 pos = barrierCornerPos(track, perp, halfWidth, i, side);
    Vector3 along{d0.x + d1.x, 0.0f, d0.y + d1.y};
    float alen = std::sqrt(along.x * along.x + along.z * along.z);

    if (alen > 1e-4f) {
        along.x /= alen;
        along.z /= alen;
    }
    Vector3 out{perp[i].x * side, 0.0f, perp[i].y * side};

    appendBarrierRails(barrierBuf, pos, along, out, style);
}

void TrackRendererBuild::buildBarrierMeshes(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth, size_t n)
{
    TrackRendererDetail::MeshBuffers barrierBuf;

    for (size_t i = 0; i < n; ++i) {
        addBarrierAtCorner(
            barrierBuf, track, perp, halfWidth, renderer.surfaceStyle_, i, n);
    }
    if (!barrierBuf.vertices.empty()) {
        renderer.barrierModel_ = LoadModelFromMesh(
            TrackRendererDetail::meshFromBuffers(barrierBuf));
        renderer.hasBarriers_ = true;
    }
}

void TrackRendererBuild::appendSponsorPanel(
    TrackRendererDetail::MeshBuffers &sponsorBuf, Vector3 base,
    Vector3 face, Vector3 right, Color panelColor)
{
    TrackRendererDetail::appendBox(
        sponsorBuf,
        Vector3{base.x + face.x * 0.12f, 1.6f, base.z + face.z * 0.12f},
        right, Vector3{0.0f, 1.0f, 0.0f}, face, 1.1f, 0.75f, 0.05f,
        panelColor);
    TrackRendererDetail::appendBox(
        sponsorBuf, Vector3{base.x, 0.8f, base.z}, right,
        Vector3{0.0f, 1.0f, 0.0f}, face, 0.06f, 1.6f, 0.06f, DARKGRAY);
}

void TrackRendererBuild::addSponsorAtWaypoint(
    TrackRendererDetail::MeshBuffers &sponsorBuf,
    const Track &track, const std::vector<Vector2> &perp,
    float halfWidth, size_t i)
{
    const auto &wp = track.Waypoints();
    uint32_t h = TrackRendererDetail::hashIndex(i + 30000);
    float sideSign = (h % 2 == 0) ? 1.0f : -1.0f;
    float dist = halfWidth + 6.0f;
    Vector3 base{
        wp[i].x + perp[i].x * dist * sideSign, 0.0f,
        wp[i].y + perp[i].y * dist * sideSign,
    };
    Color panel{
        static_cast<unsigned char>(80 + h % 120),
        static_cast<unsigned char>(60 + (h >> 4) % 120),
        static_cast<unsigned char>(50 + (h >> 8) % 120),
        255,
    };
    Vector3 face{-perp[i].x * sideSign, 0.0f, -perp[i].y * sideSign};
    Vector3 right{face.z, 0.0f, -face.x};
    Color panelColor = (h % 2 == 0) ? panel : Fade(WHITE, 0.35f);

    appendSponsorPanel(sponsorBuf, base, face, right, panelColor);
}

void TrackRendererBuild::buildSponsorMeshes(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth)
{
    const auto &wp = track.Waypoints();
    TrackRendererDetail::MeshBuffers sponsorBuf;

    for (size_t i = 0; i < wp.size(); i += 12)
        addSponsorAtWaypoint(sponsorBuf, track, perp, halfWidth, i);
    if (!sponsorBuf.vertices.empty()) {
        renderer.sponsorModel_ = LoadModelFromMesh(
            TrackRendererDetail::meshFromBuffers(sponsorBuf));
        renderer.hasSponsors_ = true;
    }
}

void TrackRendererBuild::setupLampTop(
    TrackRenderer::LampInstance &lamp, const Vector2 &perp,
    float sideSign, bool broken)
{
    float lean = broken ? 0.2f : 0.06f;

    lamp.top = Vector3{
        lamp.base.x + perp.x * sideSign * lean, 6.5f,
        lamp.base.z + perp.y * sideSign * lean,
    };
    lamp.lit = !broken;
    lamp.headColor = lamp.lit
        ? Color{255, 240, 200, 255} : Color{70, 70, 80, 255};
}

void TrackRendererBuild::addLampAtWaypoint(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth, size_t i)
{
    const auto &wp = track.Waypoints();
    uint32_t h = TrackRendererDetail::hashIndex(i + 40000);
    float sideSign = (static_cast<int>(i / 10) % 2 == 0) ? 1.0f : -1.0f;
    float dist = halfWidth + 5.0f;
    TrackRenderer::LampInstance lamp;

    lamp.base = Vector3{
        wp[i].x + perp[i].x * dist * sideSign, 0.0f,
        wp[i].y + perp[i].y * dist * sideSign,
    };
    bool broken = renderer.surfaceStyle_ == SurfaceStyle::Abimee
        && (h % 3 == 0);

    setupLampTop(lamp, perp[i], sideSign, broken);
    renderer.lamps_.push_back(lamp);
}

void TrackRendererBuild::buildLampRing(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth)
{
    const auto &wp = track.Waypoints();

    for (size_t i = 0; i < wp.size(); i += 10)
        addLampAtWaypoint(renderer, track, perp, halfWidth, i);
}

void TrackRendererBuild::initInflatableArch(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth, size_t n)
{
    const auto &wp = track.Waypoints();
    size_t mid = n / 2;
    float hw = halfWidth + 0.5f;

    renderer.arch_.leftBase = Vector3{
        wp[mid].x - perp[mid].x * hw, 0.0f,
        wp[mid].y - perp[mid].y * hw,
    };
    renderer.arch_.rightBase = Vector3{
        wp[mid].x + perp[mid].x * hw, 0.0f,
        wp[mid].y + perp[mid].y * hw,
    };
    renderer.arch_.colorA = Color{220, 40, 60, 255};
    renderer.arch_.colorB = Color{40, 80, 220, 255};
    renderer.hasArch_ = true;
}

void TrackRendererDraw::drawOneCloud(
    const TrackRenderer::CloudInstance &cloud, float timeSeconds)
{
    float drift = timeSeconds * cloud.driftSpeed;
    Vector3 center{
        cloud.basePosition.x + std::sin(drift * 0.15f) * 8.0f,
        cloud.basePosition.y + std::sin(drift * 0.08f) * 0.6f,
        cloud.basePosition.z + std::cos(drift * 0.12f) * 8.0f,
    };

    for (size_t i = 0; i < cloud.puffOffsets.size(); ++i) {
        Vector3 off = cloud.puffOffsets[i];
        float s = cloud.puffScales[i] * cloud.scale;

        DrawSphere(
            Vector3{center.x + off.x, center.y + off.y, center.z + off.z},
            s, Color{255, 255, 255, 200});
    }
}

void TrackRendererDraw::drawClouds(
    const TrackRenderer &renderer, float timeSeconds)
{
    for (const auto &cloud : renderer.clouds_)
        drawOneCloud(cloud, timeSeconds);
}

void TrackRendererDraw::drawOneLamp(
    const TrackRenderer::LampInstance &lamp)
{
    DrawCylinderEx(
        lamp.base, lamp.top, 0.08f, 0.06f, 6, Color{90, 90, 95, 255});
    if (lamp.lit) {
        DrawSphere(lamp.top, 0.22f, lamp.headColor);
        DrawCylinder(
            Vector3{lamp.top.x, 0.02f, lamp.top.z}, 1.0f, 1.0f, 0.02f, 10,
            Fade(lamp.headColor, 0.14f));
    } else {
        DrawSphere(lamp.top, 0.15f, lamp.headColor);
    }
}

void TrackRendererDraw::drawLamps(const TrackRenderer &renderer)
{
    for (const auto &lamp : renderer.lamps_)
        drawOneLamp(lamp);
}

void TrackRendererDraw::drawArchSpan(const TrackRenderer &renderer)
{
    constexpr float pillarH = 5.5f;
    Vector3 leftTop{
        renderer.arch_.leftBase.x, pillarH, renderer.arch_.leftBase.z,
    };
    Vector3 rightTop{
        renderer.arch_.rightBase.x, pillarH, renderer.arch_.rightBase.z,
    };

    for (int s = 0; s < 8; ++s) {
        float t = static_cast<float>(s) / 7.0f;
        float archY = pillarH + 1.8f * std::sin(t * PI);
        Vector3 p{
            leftTop.x + (rightTop.x - leftTop.x) * t,
            archY,
            leftTop.z + (rightTop.z - leftTop.z) * t,
        };
        Color c = (s % 2 == 0) ? renderer.arch_.colorA : renderer.arch_.colorB;

        DrawCube(p, 1.4f, 0.5f, 0.5f, c);
    }
}

void TrackRendererDraw::drawArch(const TrackRenderer &renderer)
{
    if (!renderer.hasArch_)
        return;
    constexpr float pillarH = 5.5f;

    DrawCylinder(
        renderer.arch_.leftBase, 0.35f, 0.35f, pillarH, 8,
        renderer.arch_.colorA);
    DrawCylinder(
        renderer.arch_.rightBase, 0.35f, 0.35f, pillarH, 8,
        renderer.arch_.colorB);
    drawArchSpan(renderer);
}

void TrackRendererDraw::drawPotholes(const TrackRenderer &renderer)
{
    for (const auto &hole : renderer.potholes_) {
        DrawCylinder(
            hole.position, hole.radius, hole.radius * 0.85f, 0.04f, 12,
            Color{35, 32, 30, 255});
        DrawCylinder(
            hole.position, hole.radius * 0.6f, hole.radius * 0.5f, 0.05f,
            10, Color{22, 20, 18, 255});
    }
}

void TrackRendererDraw::drawCracks(const TrackRenderer &renderer)
{
    for (const auto &crack : renderer.cracks_) {
        Vector3 a{
            crack.center.x - crack.tangent.x * crack.length * 0.5f,
            crack.center.y + 0.01f,
            crack.center.z - crack.tangent.z * crack.length * 0.5f,
        };
        Vector3 b{
            crack.center.x + crack.tangent.x * crack.length * 0.5f,
            crack.center.y + 0.01f,
            crack.center.z + crack.tangent.z * crack.length * 0.5f,
        };

        DrawCylinderEx(a, b, 0.06f, 0.06f, 4, Color{48, 44, 40, 255});
    }
}

void TrackRendererDraw::drawRoadDamage(const TrackRenderer &renderer)
{
    drawPotholes(renderer);
    drawCracks(renderer);
}

void TrackRendererDraw::drawAbimeeDebris(const TrackRenderer &renderer)
{
    if (renderer.surfaceStyle_ != SurfaceStyle::Abimee)
        return;
    for (size_t i = 0; i < renderer.potholes_.size(); i += 2) {
        const auto &h = renderer.potholes_[i];

        DrawCube(
            Vector3{h.position.x + 0.8f, 0.08f, h.position.z},
            0.5f, 0.16f, 0.5f, Color{90, 110, 45, 255});
    }
}

void TrackRendererDraw::drawGantryBeam(
    Vector3 leftBase, Vector3 rightBase, Vector3 p, float hw,
    float pillarH)
{
    Color pillarColor{180, 180, 190, 255};

    DrawCube(
        Vector3{leftBase.x, pillarH * 0.5f, leftBase.z},
        0.5f, pillarH, 0.5f, pillarColor);
    DrawCube(
        Vector3{rightBase.x, pillarH * 0.5f, rightBase.z},
        0.5f, pillarH, 0.5f, pillarColor);
    Vector3 beamMid{
        (leftBase.x + rightBase.x) * 0.5f, pillarH,
        (leftBase.z + rightBase.z) * 0.5f,
    };

    DrawCube(beamMid, hw * 2.0f + 1.0f, 0.4f, 0.5f, DARKGRAY);
    DrawCube(
        Vector3{beamMid.x, pillarH + 0.6f, beamMid.z},
        hw * 1.6f, 0.5f, 0.15f, WHITE);
    drawGantryCheckerCells(beamMid, p, hw, pillarH);
}

void TrackRendererDraw::drawGantryCheckerCells(
    Vector3 beamMid, Vector3 p, float hw, float pillarH)
{
    for (int c = 0; c < 6; ++c) {
        Color cell = (c % 2 == 0) ? BLACK : WHITE;
        float t = -0.5f + (static_cast<float>(c) + 0.5f) / 6.0f;

        DrawCube(
            Vector3{
                beamMid.x + p.x * t * hw * 1.4f, pillarH + 0.6f,
                beamMid.z + p.z * t * hw * 1.4f,
            },
            hw * 0.22f, 0.48f, 0.12f, cell);
    }
}

void TrackRendererDraw::drawGantryLights(
    float timeSeconds, Vector3 leftBase, float pillarH, Vector3 d)
{
    float lightPhase = std::fmod(timeSeconds * 2.0f, 3.0f);
    Color lightColors[3] = {RED, YELLOW, GREEN};

    for (int l = 0; l < 3; ++l) {
        bool on = lightPhase >= static_cast<float>(l)
            && lightPhase < static_cast<float>(l + 1);
        Color lc = on ? lightColors[l] : Fade(lightColors[l], 0.25f);

        DrawSphere(
            Vector3{
                leftBase.x + d.x * 0.8f,
                pillarH - 1.5f - static_cast<float>(l) * 0.7f,
                leftBase.z + d.z * 0.8f,
            },
            0.18f, lc);
    }
}

void TrackRendererDraw::drawStartGantry(
    const TrackRenderer &renderer, float timeSeconds)
{
    Vector3 p = renderer.startGantryPerp_;
    Vector3 d = renderer.startGantryAlong_;
    float hw = renderer.trackHalfWidth_ + 1.5f;
    Vector3 leftBase{
        renderer.startGantryBase_.x - p.x * hw, 0.0f,
        renderer.startGantryBase_.z - p.z * hw,
    };
    Vector3 rightBase{
        renderer.startGantryBase_.x + p.x * hw, 0.0f,
        renderer.startGantryBase_.z + p.z * hw,
    };
    constexpr float pillarH = 7.5f;

    drawGantryBeam(leftBase, rightBase, p, hw, pillarH);
    drawGantryLights(timeSeconds, leftBase, pillarH, d);
}

void TrackRendererDraw::drawGrandstandRows(
    const TrackRenderer::GrandstandInstance &gs)
{
    constexpr int kRows = 4;
    float stepDepth = 1.8f;
    float stepWidth = gs.length;
    Color seatColor{110, 110, 118, 255};

    for (int row = 0; row < kRows; ++row) {
        Vector3 seatPos{
            gs.origin.x + gs.outward.x
                * (static_cast<float>(row) * stepDepth + 0.9f),
            static_cast<float>(row) * 1.1f + 0.55f,
            gs.origin.z + gs.outward.z
                * (static_cast<float>(row) * stepDepth + 0.9f),
        };

        DrawCube(seatPos, stepWidth, 1.1f, stepDepth, seatColor);
        DrawCubeWires(
            seatPos, stepWidth, 1.1f, stepDepth, Fade(BLACK, 0.3f));
    }
}

void TrackRendererDraw::drawGrandstandRoof(
    const TrackRenderer::GrandstandInstance &gs)
{
    constexpr int kRows = 4;
    float stepDepth = 1.8f;
    float stepWidth = gs.length;
    Vector3 roofPos{
        gs.origin.x + gs.outward.x
            * (static_cast<float>(kRows) * stepDepth + 0.5f),
        static_cast<float>(kRows) * 1.1f + 1.2f,
        gs.origin.z + gs.outward.z
            * (static_cast<float>(kRows) * stepDepth + 0.5f),
    };

    DrawCube(
        roofPos, stepWidth + 1.0f, 0.25f, stepDepth + 1.5f,
        Color{180, 50, 50, 255});
}

void TrackRendererDraw::drawOneGrandstand(
    const TrackRenderer::GrandstandInstance &gs)
{
    drawGrandstandRows(gs);
    drawGrandstandRoof(gs);
}

void TrackRendererDraw::drawGrandstandStructure(
    const TrackRenderer &renderer)
{
    for (const auto &gs : renderer.grandstands_)
        drawOneGrandstand(gs);
}

void TrackRendererDraw::drawOneSpectator(
    const TrackRenderer::SpectatorInstance &spec, float timeSeconds)
{
    float jump = std::max(
        0.0f, std::sin(timeSeconds * spec.jumpSpeed + spec.jumpPhase)) * 0.35f;

    if (std::sin(timeSeconds * spec.jumpSpeed + spec.jumpPhase) < 0.85f)
        jump = 0.0f;
    Vector3 pos{spec.position.x, spec.position.y + jump, spec.position.z};

    DrawCube(
        Vector3{pos.x, pos.y + 0.35f, pos.z},
        0.35f, 0.7f, 0.25f, spec.shirtColor);
    DrawSphere(
        Vector3{pos.x, pos.y + 0.85f, pos.z}, 0.18f,
        Color{240, 200, 170, 255});
}

void TrackRendererDraw::drawSpectators(
    const TrackRenderer &renderer, float timeSeconds)
{
    for (const auto &gs : renderer.grandstands_) {
        for (const auto &spec : gs.spectators)
            drawOneSpectator(spec, timeSeconds);
    }
}

void TrackRendererDraw::drawBuildingProp(
    const TrackRenderer::PropInstance &prop)
{
    float w = 2.5f;
    float h = 5.0f * prop.heightScale;

    DrawCube(
        Vector3{prop.position.x, h * 0.5f, prop.position.z},
        w, h, w, prop.color);
    DrawCubeWires(
        Vector3{prop.position.x, h * 0.5f, prop.position.z},
        w, h, w, Fade(BLACK, 0.4f));
    for (int win = 0; win < 3; ++win) {
        DrawCube(
            Vector3{
                prop.position.x + 0.8f,
                1.5f + static_cast<float>(win) * 1.4f,
                prop.position.z + 1.26f,
            },
            0.4f, 0.5f, 0.05f, Color{200, 220, 255, 200});
    }
}

void TrackRendererDraw::drawDeadTreeProp(
    const TrackRenderer::PropInstance &prop)
{
    float trunkH = 1.2f * prop.heightScale;

    DrawCylinder(
        Vector3{prop.position.x, 0.0f, prop.position.z},
        0.2f, 0.25f, trunkH, 5, Color{72, 52, 36, 255});
    DrawSphere(
        Vector3{prop.position.x, trunkH + 0.3f, prop.position.z},
        0.5f * prop.heightScale, prop.color);
    DrawSphere(
        Vector3{
            prop.position.x + 0.4f, trunkH,
            prop.position.z - 0.2f,
        },
        0.35f * prop.heightScale, prop.color);
}

void TrackRendererDraw::drawLiveTreeProp(
    const TrackRenderer::PropInstance &prop)
{
    float trunkH = 1.6f * prop.heightScale;

    DrawCylinder(
        Vector3{prop.position.x, 0.0f, prop.position.z},
        0.25f, 0.3f, trunkH, 6, Color{92, 64, 40, 255});
    DrawCylinder(
        Vector3{prop.position.x, trunkH, prop.position.z},
        1.6f * prop.heightScale, 0.0f, 2.2f * prop.heightScale, 8,
        prop.color);
}

void TrackRendererDraw::drawOneProp(
    const TrackRenderer::PropInstance &prop)
{
    float shadowR = (prop.type == 1) ? 1.8f : 1.2f;

    TrackRendererDetail::drawPropShadow(prop.position, shadowR);
    if (prop.type == 1)
        drawBuildingProp(prop);
    else if (prop.type == 2)
        drawDeadTreeProp(prop);
    else
        drawLiveTreeProp(prop);
}

void TrackRendererDraw::drawProps(const TrackRenderer &renderer)
{
    for (const auto &prop : renderer.props_)
        drawOneProp(prop);
}

void TrackRendererDraw::drawTireStacks(const TrackRenderer &renderer)
{
    for (const auto &stack : renderer.tireStacks_) {
        TrackRendererDetail::drawPropShadow(stack.position, 0.9f);
        for (int t = 0; t < stack.tiers; ++t) {
            Color tc = (t % 2 == 0)
                ? Color{30, 30, 32, 255} : Color{220, 220, 220, 255};

            DrawCylinder(
                Vector3{
                    stack.position.x,
                    static_cast<float>(t) * 0.35f + 0.18f,
                    stack.position.z,
                },
                0.55f, 0.55f, 0.35f, 10, tc);
        }
    }
}

void TrackRendererDraw::drawOnePennant(
    const TrackRenderer::PennantInstance &pen, float timeSeconds)
{
    float wave = std::sin(timeSeconds * 3.0f + pen.phase) * 0.3f;

    DrawCylinderEx(
        pen.base, pen.top, 0.06f, 0.04f, 4, Color{120, 120, 130, 255});
    Vector3 flagEnd{
        pen.top.x + 1.2f + wave,
        pen.top.y + std::sin(timeSeconds * 4.0f + pen.phase) * 0.15f,
        pen.top.z,
    };

    DrawCylinderEx(pen.top, flagEnd, 0.02f, 0.02f, 3, pen.color);
    DrawTriangle3D(
        pen.top, flagEnd,
        Vector3{pen.top.x, pen.top.y - 0.5f, pen.top.z}, pen.color);
}

void TrackRendererDraw::drawPennants(
    const TrackRenderer &renderer, float timeSeconds)
{
    for (const auto &pen : renderer.pennants_)
        drawOnePennant(pen, timeSeconds);
}

void TrackRendererDraw::drawNpcBody(
    const TrackRenderer::NpcInstance &npc, float armWave)
{
    DrawCylinder(
        Vector3{0.0f, 0.9f, 0.0f}, 0.22f, 0.25f, 1.1f, 6, npc.shirtColor);
    DrawSphere(Vector3{0.0f, 1.65f, 0.0f}, 0.22f, Color{240, 200, 170, 255});
    DrawCube(
        Vector3{0.45f, 1.3f + armWave * 0.15f, 0.0f},
        0.12f, 0.5f, 0.12f, npc.shirtColor);
    DrawCube(
        Vector3{-0.35f, 1.1f, 0.0f}, 0.12f, 0.4f, 0.12f, npc.shirtColor);
    DrawCylinder(
        Vector3{0.7f, 1.5f, 0.0f}, 0.03f, 0.03f, 1.2f, 4,
        Color{100, 80, 60, 255});
}

void TrackRendererDraw::drawNpcFlag(
    const TrackRenderer::NpcInstance &npc, float armWave)
{
    rlPushMatrix();
    rlRotatef(armWave * 25.0f, 0.0f, 0.0f, 1.0f);
    DrawCube(
        Vector3{0.7f, 2.2f, 0.0f}, 0.5f, 0.35f, 0.05f, npc.flagColor);
    rlPopMatrix();
}

void TrackRendererDraw::drawOneNpc(
    const TrackRenderer::NpcInstance &npc, float timeSeconds)
{
    TrackRendererDetail::drawPropShadow(npc.position, 0.45f);
    float armWave = std::sin(timeSeconds * 4.0f + npc.animPhase);

    rlPushMatrix();
    rlTranslatef(npc.position.x, 0.0f, npc.position.z);
    rlRotatef(npc.heading * RAD2DEG, 0.0f, 1.0f, 0.0f);
    drawNpcBody(npc, armWave);
    drawNpcFlag(npc, armWave);
    rlPopMatrix();
}

void TrackRendererDraw::drawNpcs(
    const TrackRenderer &renderer, float timeSeconds)
{
    for (const auto &npc : renderer.npcs_)
        drawOneNpc(npc, timeSeconds);
}

void DrawSkyGradient(int screenWidth, int screenHeight)
{
    Color horizon{135, 196, 235, 255};
    Color zenith{42, 92, 168, 255};

    ClearBackground(horizon);
    int bands = 32;
    int bandH = (screenHeight + bands - 1) / bands;

    for (int i = 0; i < bands; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(bands - 1);
        Color c = TrackRendererDetail::lerpColor(horizon, zenith, t * t);

        DrawRectangle(0, i * bandH, screenWidth, bandH + 1, c);
    }
}

void TrackRendererBuild::buildSceneMeshes(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth, size_t n)
{
    buildTrackMeshes(renderer, track, perp, halfWidth);
    buildGroundAndFinish(renderer, track, perp);
    initStartGantry(renderer, track, perp);
    initSkidOverlay(renderer, track, halfWidth);
    buildBarrierMeshes(renderer, track, perp, halfWidth, n);
    buildSponsorMeshes(renderer, track, perp, halfWidth);
}

void TrackRendererBuild::buildSceneDecor(
    TrackRenderer &renderer, const Track &track,
    const std::vector<Vector2> &perp, float halfWidth, size_t n)
{
    buildCloudRing(renderer);
    buildGrandstands(renderer, track, perp, halfWidth);
    constexpr int kStride = 3;
    const auto &wp = track.Waypoints();

    for (size_t i = 0; i < wp.size(); i += static_cast<size_t>(kStride))
        populateWaypointDecor(renderer, track, perp, halfWidth, i);
    buildAbimeeDamage(renderer, track, perp);
    buildLampRing(renderer, track, perp, halfWidth);
    initInflatableArch(renderer, track, perp, halfWidth, n);
}

TrackRenderer::TrackRenderer(const Track &track, const TrackDef &def)
    : surfaceStyle_(def.surfaceStyle)
{
    std::vector<Vector2> perp =
        TrackRendererDetail::computePerpendiculars(track);
    float halfWidth = track.Width() * 0.5f;

    trackHalfWidth_ = halfWidth;
    size_t n = track.Waypoints().size();

    TrackRendererBuild::buildSceneMeshes(
        *this, track, perp, halfWidth, n);
    TrackRendererBuild::buildSceneDecor(
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

void TrackRenderer::DrawOpaqueGeometry() const
{
    DrawModel(groundModel_, Vector3{0.0f, -0.05f, 0.0f}, 1.0f, WHITE);
    DrawModel(trackModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    DrawModel(rubberLineModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    DrawModel(centerDashModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    DrawModel(edgeLineOuterModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    DrawModel(edgeLineInnerModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    DrawModel(curbModelOuter_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    DrawModel(curbModelInner_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    if (hasBarriers_)
        DrawModel(barrierModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    if (hasSponsors_)
        DrawModel(sponsorModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    BeginBlendMode(BLEND_ALPHA);
    DrawModel(skidOverlayModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    EndBlendMode();
    DrawModel(finishLineModel_, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
}

void TrackRenderer::Draw(float timeSeconds) const
{
    TrackRendererDetail::drawMountainsRing(220.0f, 2.0f, 48);
    TrackRendererDraw::drawClouds(*this, timeSeconds);
    DrawOpaqueGeometry();
    TrackRendererDraw::drawLamps(*this);
    TrackRendererDraw::drawArch(*this);
    TrackRendererDraw::drawRoadDamage(*this);
    TrackRendererDraw::drawAbimeeDebris(*this);
    TrackRendererDraw::drawStartGantry(*this, timeSeconds);
    TrackRendererDraw::drawGrandstandStructure(*this);
    TrackRendererDraw::drawSpectators(*this, timeSeconds);
    TrackRendererDraw::drawProps(*this);
    TrackRendererDraw::drawTireStacks(*this);
    TrackRendererDraw::drawPennants(*this, timeSeconds);
    TrackRendererDraw::drawNpcs(*this, timeSeconds);
}

void TrackRenderer::ApplyShader(Shader shader)
{
    TrackRendererDetail::applyShaderToModel(trackModel_, shader);
    TrackRendererDetail::applyShaderToModel(rubberLineModel_, shader);
    TrackRendererDetail::applyShaderToModel(centerDashModel_, shader);
    TrackRendererDetail::applyShaderToModel(edgeLineOuterModel_, shader);
    TrackRendererDetail::applyShaderToModel(edgeLineInnerModel_, shader);
    TrackRendererDetail::applyShaderToModel(curbModelOuter_, shader);
    TrackRendererDetail::applyShaderToModel(curbModelInner_, shader);
    TrackRendererDetail::applyShaderToModel(groundModel_, shader);
    TrackRendererDetail::applyShaderToModel(finishLineModel_, shader);
    TrackRendererDetail::applyShaderToModel(skidOverlayModel_, shader);
    if (hasBarriers_)
        TrackRendererDetail::applyShaderToModel(barrierModel_, shader);
    if (hasSponsors_)
        TrackRendererDetail::applyShaderToModel(sponsorModel_, shader);
}

void TrackRenderer::QueueSkidMark(
    Vector3 pos, Vector3 dir, float width, float strength)
{
    float len = std::sqrt(dir.x * dir.x + dir.z * dir.z);

    if (len < 1e-4f)
        return;
    SkidMarkCmd cmd;

    cmd.position = pos;
    cmd.direction = Vector3{dir.x / len, 0.0f, dir.z / len};
    cmd.width = width;
    cmd.strength = std::clamp(strength, 0.0f, 1.0f);
    skidQueue_.push_back(cmd);
}

void TrackRendererDraw::drawOneSkidMark(
    const TrackRenderer::SkidMarkCmd &cmd, Vector2 origin, float worldSize)
{
    Vector2 center = TrackRendererDetail::worldToSkidTex(
        cmd.position.x, cmd.position.z, origin, worldSize);
    float lenPx = 1.2f / worldSize * static_cast<float>(kSkidTextureSize);
    float wPx = cmd.width / worldSize * static_cast<float>(kSkidTextureSize);
    float angle = std::atan2(cmd.direction.x, cmd.direction.z) * RAD2DEG;
    Rectangle rect{
        center.x - lenPx * 0.5f, center.y - wPx * 0.5f, lenPx, wPx,
    };

    DrawRectanglePro(
        rect, Vector2{lenPx * 0.5f, wPx * 0.5f}, angle,
        Color{
            0, 0, 0,
            static_cast<unsigned char>(cmd.strength * 76.0f),
        });
}

void TrackRenderer::FlushSkidMarks()
{
    if (skidQueue_.empty() || skidTexture_.id == 0)
        return;
    BeginTextureMode(skidTexture_);

    for (const auto &cmd : skidQueue_)
        TrackRendererDraw::drawOneSkidMark(
            cmd, skidWorldOrigin_, skidWorldSize_);
    EndTextureMode();
    skidQueue_.clear();
}

} // namespace racer
