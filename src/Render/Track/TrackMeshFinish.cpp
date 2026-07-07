/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track finish line and mountain ring meshes
*/

#include "Render/Track/TrackMeshBuilder.hpp"
#include "rlgl.h"

namespace racer {

void TrackMeshBuilder::allocCheckerMesh(Mesh &mesh, int tilesPerSide)
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

void TrackMeshBuilder::fillCheckerTiles(
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

Mesh TrackMeshBuilder::buildCheckerGroundMesh(
    float totalSize, int tilesPerSide, SurfaceStyle style)
{
    float tileSize = totalSize / static_cast<float>(tilesPerSide);
    float half = totalSize * 0.5f;
    Mesh mesh{};
    Color greenA = (style == SurfaceStyle::ABIMEE)
        ? Color{118, 108, 62, 255} : Color{58, 130, 58, 255};
    Color greenB = (style == SurfaceStyle::ABIMEE)
        ? Color{108, 98, 55, 255} : Color{50, 118, 50, 255};

    allocCheckerMesh(mesh, tilesPerSide);
    fillCheckerTiles(mesh, tilesPerSide, tileSize, half, greenA, greenB);
    UploadMesh(&mesh, false);
    return mesh;
}

Mesh TrackMeshBuilder::buildFinishLineMesh(
    const Track &track, const std::vector<Vector2> &perp)
{
    constexpr int kCols = 6;
    const auto &wp = track.waypoints();
    Vector2 base = wp[0];
    Vector2 p = perp[0];
    Vector2 dir{-p.y, p.x};
    float halfWidth = track.width() * 0.5f;
    Mesh mesh{};

    allocFinishLineMesh(mesh, kCols);
    fillFinishLineColumns(mesh, base, p, dir, halfWidth);
    UploadMesh(&mesh, false);
    return mesh;
}

void TrackMeshBuilder::allocFinishLineMesh(Mesh &mesh, int colCount)
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

void TrackMeshBuilder::makeFinishColumnQuad(
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

void TrackMeshBuilder::fillFinishLineColumns(
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

void TrackMeshBuilder::drawPropShadow(Vector3 pos, float radius)
{
    DrawCylinder(
        Vector3{pos.x, 0.015f, pos.z}, radius, radius, 0.02f, 10,
        Fade(BLACK, 0.28f));
}

void TrackMeshBuilder::drawMountainSegment(
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

void TrackMeshBuilder::drawMountainsRing(
    float radius, float baseY, int segments)
{
    Color farColor{72, 118, 158, 255};
    Color nearColor{58, 98, 132, 255};

    for (int i = 0; i < segments; ++i)
        drawMountainSegment(i, segments, radius, baseY, farColor, nearColor);
}


} // namespace racer
