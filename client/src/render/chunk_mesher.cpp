#include "client/render/chunk_mesher.h"

#include <vector>

#include "client/render/block_colors.h"
#include "common/world/block.h"

namespace client {

namespace {

using common::world::BlockId;
using common::world::CHUNK_SIZE_X;
using common::world::CHUNK_SIZE_Y;
using common::world::CHUNK_SIZE_Z;

struct FaceDef {
    int dx, dy, dz;
    Vector3 normal;
    Vector3 corners[4];
};

const FaceDef kFaces[6] = {
    {1, 0, 0, {1, 0, 0}, {{1, 0, 0}, {1, 1, 0}, {1, 1, 1}, {1, 0, 1}}},
    {-1, 0, 0, {-1, 0, 0}, {{0, 0, 1}, {0, 1, 1}, {0, 1, 0}, {0, 0, 0}}},
    {0, 1, 0, {0, 1, 0}, {{0, 1, 0}, {0, 1, 1}, {1, 1, 1}, {1, 1, 0}}},
    {0, -1, 0, {0, -1, 0}, {{0, 0, 1}, {0, 0, 0}, {1, 0, 0}, {1, 0, 1}}},
    {0, 0, 1, {0, 0, 1}, {{1, 0, 1}, {1, 1, 1}, {0, 1, 1}, {0, 0, 1}}},
    {0, 0, -1, {0, 0, -1}, {{0, 0, 0}, {0, 1, 0}, {1, 1, 0}, {1, 0, 0}}},
};

bool IsSolid(const common::world::Chunk& chunk, int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE_X || y < 0 || y >= CHUNK_SIZE_Y || z < 0 || z >= CHUNK_SIZE_Z) {
        return false; // hors chunk : traite comme de l'air au jour 2 (pas de culling inter-chunk)
    }
    return static_cast<BlockId>(chunk.blocks[common::world::BlockIndex(x, y, z)]) != BlockId::Air;
}

// Nombre de faces au-dela duquel on arrete d'en ajouter : garde-fou pour
// l'index uint16 de raylib (max 65535 sommets, 4 par face -> ~16000 faces).
constexpr int kMaxFaces = 16000;

} // namespace

Mesh BuildNaiveChunkMesh(const common::world::Chunk& chunk) {
    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<Color> colors;

    int faceCount = 0;

    for (int x = 0; x < CHUNK_SIZE_X && faceCount < kMaxFaces; ++x) {
        for (int y = 0; y < CHUNK_SIZE_Y && faceCount < kMaxFaces; ++y) {
            for (int z = 0; z < CHUNK_SIZE_Z && faceCount < kMaxFaces; ++z) {
                auto id = static_cast<BlockId>(chunk.blocks[common::world::BlockIndex(x, y, z)]);
                if (id == BlockId::Air) continue;

                Color color = ColorForBlock(id);

                for (const FaceDef& face : kFaces) {
                    if (faceCount >= kMaxFaces) break;
                    if (IsSolid(chunk, x + face.dx, y + face.dy, z + face.dz)) continue;

                    for (const Vector3& corner : face.corners) {
                        positions.push_back({
                            static_cast<float>(x) + corner.x,
                            static_cast<float>(y) + corner.y,
                            static_cast<float>(z) + corner.z,
                        });
                        normals.push_back(face.normal);
                        colors.push_back(color);
                    }
                    ++faceCount;
                }
            }
        }
    }

    Mesh mesh{};
    mesh.vertexCount = static_cast<int>(positions.size());
    mesh.triangleCount = faceCount * 2;

    mesh.vertices = static_cast<float*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.normals = static_cast<float*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.colors = static_cast<unsigned char*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount) * 4 * sizeof(unsigned char)));
    mesh.indices = static_cast<unsigned short*>(MemAlloc(static_cast<unsigned int>(faceCount) * 6 * sizeof(unsigned short)));

    for (int i = 0; i < mesh.vertexCount; ++i) {
        mesh.vertices[i * 3 + 0] = positions[i].x;
        mesh.vertices[i * 3 + 1] = positions[i].y;
        mesh.vertices[i * 3 + 2] = positions[i].z;

        mesh.normals[i * 3 + 0] = normals[i].x;
        mesh.normals[i * 3 + 1] = normals[i].y;
        mesh.normals[i * 3 + 2] = normals[i].z;

        mesh.colors[i * 4 + 0] = colors[i].r;
        mesh.colors[i * 4 + 1] = colors[i].g;
        mesh.colors[i * 4 + 2] = colors[i].b;
        mesh.colors[i * 4 + 3] = colors[i].a;
    }

    for (int f = 0; f < faceCount; ++f) {
        unsigned short base = static_cast<unsigned short>(f * 4);
        int idx = f * 6;
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

} // namespace client
