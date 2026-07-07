/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track strip and dashed mesh builders
*/

#include "Render/Track/TrackMeshBuilder.hpp"
#include "rlgl.h"

namespace racer {

void TrackMeshBuilder::fillStripEndpoints(
    const Track &track, const std::vector<Vector2> &perp,
    float innerOffset, float outerOffset, float yHeight,
    std::vector<Vector3> &inner, std::vector<Vector3> &outer)
{
    const auto &wp = track.waypoints();
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

void TrackMeshBuilder::allocColoredStripMesh(Mesh &mesh, size_t n)
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

void TrackMeshBuilder::writeColoredQuadVertices(
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

void TrackMeshBuilder::writeMeshQuadIndices(
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

void TrackMeshBuilder::writeStripSegment(
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

void TrackMeshBuilder::writeDashedSegment(
    Mesh &mesh, int quadIdx, const Vector3 quad[4], Color color)
{
    size_t vBase = static_cast<size_t>(quadIdx) * 4;

    writeColoredQuadVertices(mesh, vBase, quad, color);
    writeMeshQuadIndices(
        mesh, static_cast<size_t>(quadIdx),
        static_cast<unsigned short>(quadIdx * 4));
}

void TrackMeshBuilder::writeCheckerTile(
    Mesh &mesh, int quadIdx, const Vector3 quad[4], Color c)
{
    size_t vBase = static_cast<size_t>(quadIdx) * 4;

    writeColoredQuadVertices(mesh, vBase, quad, c);
    writeMeshQuadIndices(
        mesh, static_cast<size_t>(quadIdx),
        static_cast<unsigned short>(quadIdx * 4));
}

void TrackMeshBuilder::writeFinishColumn(
    Mesh &mesh, int col, const Vector3 quad[4], Color color)
{
    size_t vBase = static_cast<size_t>(col) * 4;

    writeColoredQuadVertices(mesh, vBase, quad, color);
    writeMeshQuadIndices(
        mesh, static_cast<size_t>(col),
        static_cast<unsigned short>(col * 4));
}

Mesh TrackMeshBuilder::buildStripMesh(
    const Track &track, const std::vector<Vector2> &perp,
    float innerOffset, float outerOffset, float yHeight,
    const std::function<Color(size_t)> &colorFn)
{
    const auto &wp = track.waypoints();
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

int TrackMeshBuilder::countDashSegments(
    size_t n, int dashPeriod, int dashOn)
{
    int segmentCount = 0;

    for (size_t i = 0; i < n; ++i) {
        if (static_cast<int>(i % static_cast<size_t>(dashPeriod)) < dashOn)
            ++segmentCount;
    }
    return segmentCount;
}

void TrackMeshBuilder::allocDashedStripMesh(
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

Vector3 TrackMeshBuilder::makeDashedQuadPoint(
    const Track &track, const std::vector<Vector2> &perp,
    size_t idx, float innerOffset, float outerOffset, float yHeight,
    bool outer)
{
    const auto &wp = track.waypoints();
    float offset = outer ? outerOffset : innerOffset;

    return Vector3{
        wp[idx].x + perp[idx].x * offset, yHeight,
        wp[idx].y + perp[idx].y * offset,
    };
}

Mesh TrackMeshBuilder::buildDashedStripMesh(
    const Track &track, const std::vector<Vector2> &perp,
    float innerOffset, float outerOffset, float yHeight,
    Color color, int dashPeriod, int dashOn)
{
    size_t n = track.waypoints().size();
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

void TrackMeshBuilder::fillDashedStripLoop(
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


} // namespace racer
