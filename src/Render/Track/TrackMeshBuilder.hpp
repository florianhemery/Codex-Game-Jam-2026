/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track mesh geometry helpers
*/

#ifndef TRACK_MESH_BUILDER_HPP_
#define TRACK_MESH_BUILDER_HPP_

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

#include "raylib.h"
#include "Track/Track.hpp"

namespace racer {

class TrackMeshBuilder {
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

} // namespace racer

#endif /* !TRACK_MESH_BUILDER_HPP_ */
