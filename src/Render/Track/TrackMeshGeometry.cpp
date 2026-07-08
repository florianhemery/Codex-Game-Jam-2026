/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track mesh buffer and box geometry
*/

#include "Render/Track/TrackMeshBuilder.hpp"
#include "rlgl.h"

namespace racer {

namespace {
constexpr int kSkidTextureSize = 2048;
} // namespace

void TrackMeshBuilder::appendQuad(
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

Vector3 TrackMeshBuilder::boxCorner(
    Vector3 fc, Vector3 u, Vector3 v, float hu, float hv,
    float su, float sv)
{
    return Vector3{
        fc.x + u.x * su * hu + v.x * sv * hv,
        fc.y + u.y * su * hu + v.y * sv * hv,
        fc.z + u.z * su * hu + v.z * sv * hv,
    };
}

void TrackMeshBuilder::appendBoxFace(
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

void TrackMeshBuilder::appendBox(
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

void TrackMeshBuilder::copyMeshBufferData(
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

Mesh TrackMeshBuilder::meshFromBuffers(const MeshBuffers &mb)
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

void TrackMeshBuilder::writeSkidQuadCorner(
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

void TrackMeshBuilder::fillSkidQuadVertices(
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

Mesh TrackMeshBuilder::buildSkidQuadMesh(
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

void TrackMeshBuilder::applyShaderToModel(Model &model, Shader shader)
{
    for (int i = 0; i < model.materialCount; ++i)
        model.materials[i].shader = shader;
}

Vector2 TrackMeshBuilder::worldToSkidTex(
    float worldX, float worldZ, Vector2 origin, float size)
{
    float u = (worldX - origin.x) / size;
    float v = 1.0f - (worldZ - origin.y) / size;

    return Vector2{
        u * static_cast<float>(kSkidTextureSize),
        v * static_cast<float>(kSkidTextureSize),
    };
}

} // namespace racer

