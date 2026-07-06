#include "client/render/chunk_mesher.h"

#include <array>
#include <vector>

#include "client/render/block_colors.h"
#include "common/world/block.h"

namespace client {

namespace {

using common::world::BlockId;
using common::world::CHUNK_SIZE_X;
using common::world::CHUNK_SIZE_Y;
using common::world::CHUNK_SIZE_Z;

int Dims(int axis) {
    switch (axis) {
        case 0: return CHUNK_SIZE_X;
        case 1: return CHUNK_SIZE_Y;
        default: return CHUNK_SIZE_Z;
    }
}

uint8_t BlockAt(const common::world::Chunk& chunk, int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE_X || y < 0 || y >= CHUNK_SIZE_Y || z < 0 || z >= CHUNK_SIZE_Z) {
        return 0; // hors chunk : air (pas de culling inter-chunk au jour 3 non plus)
    }
    return chunk.blocks[common::world::BlockIndex(x, y, z)];
}

struct MaskCell {
    uint8_t blockId = 0;
    bool backFace = false;

    bool SameAs(const MaskCell& other) const {
        return blockId != 0 && blockId == other.blockId && backFace == other.backFace;
    }
};

struct MeshBuilder {
    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<Color> colors;
    int faceCount = 0;

    void AddQuad(Vector3 origin, Vector3 du, Vector3 dv, Vector3 normal, Color color, bool backFace) {
        Vector3 a = origin;
        Vector3 b = {origin.x + du.x, origin.y + du.y, origin.z + du.z};
        Vector3 c = {origin.x + du.x + dv.x, origin.y + du.y + dv.y, origin.z + du.z + dv.z};
        Vector3 d = {origin.x + dv.x, origin.y + dv.y, origin.z + dv.z};

        if (!backFace) {
            positions.push_back(a);
            positions.push_back(b);
            positions.push_back(c);
            positions.push_back(d);
        } else {
            // ordre inverse pour rester coherent avec la normale opposee
            positions.push_back(a);
            positions.push_back(d);
            positions.push_back(c);
            positions.push_back(b);
        }
        for (int i = 0; i < 4; ++i) {
            normals.push_back(normal);
            colors.push_back(color);
        }
        ++faceCount;
    }
};

} // namespace

Mesh BuildGreedyChunkMesh(const common::world::Chunk& chunk) {
    MeshBuilder builder;

    for (int d = 0; d < 3; ++d) {
        int u = (d + 1) % 3;
        int v = (d + 2) % 3;

        int dimsD = Dims(d);
        int dimsU = Dims(u);
        int dimsV = Dims(v);

        std::array<int, 3> x{0, 0, 0};
        std::array<int, 3> q{0, 0, 0};
        q[d] = 1;

        std::vector<MaskCell> mask(static_cast<size_t>(dimsU) * dimsV);

        for (x[d] = -1; x[d] < dimsD;) {
            int n = 0;
            for (x[v] = 0; x[v] < dimsV; ++x[v]) {
                for (x[u] = 0; x[u] < dimsU; ++x[u], ++n) {
                    uint8_t blockA = (x[d] >= 0) ? BlockAt(chunk, x[0], x[1], x[2]) : 0;
                    uint8_t blockB = (x[d] < dimsD - 1)
                                         ? BlockAt(chunk, x[0] + q[0], x[1] + q[1], x[2] + q[2])
                                         : 0;

                    bool solidA = blockA != 0;
                    bool solidB = blockB != 0;

                    if (solidA == solidB) {
                        mask[n] = MaskCell{};
                    } else if (solidA) {
                        mask[n] = MaskCell{blockA, false};
                    } else {
                        mask[n] = MaskCell{blockB, true};
                    }
                }
            }

            ++x[d];

            n = 0;
            for (int j = 0; j < dimsV; ++j) {
                for (int i = 0; i < dimsU;) {
                    if (mask[n].blockId == 0) {
                        ++i;
                        ++n;
                        continue;
                    }

                    MaskCell current = mask[n];

                    int w = 1;
                    while (i + w < dimsU && mask[n + w].SameAs(current)) ++w;

                    int h = 1;
                    bool done = false;
                    while (j + h < dimsV) {
                        for (int k = 0; k < w; ++k) {
                            if (!mask[n + k + h * dimsU].SameAs(current)) {
                                done = true;
                                break;
                            }
                        }
                        if (done) break;
                        ++h;
                    }

                    std::array<int, 3> origin = x;
                    origin[u] = i;
                    origin[v] = j;

                    std::array<int, 3> du{0, 0, 0};
                    du[u] = w;
                    std::array<int, 3> dv{0, 0, 0};
                    dv[v] = h;

                    Vector3 originF{static_cast<float>(origin[0]), static_cast<float>(origin[1]), static_cast<float>(origin[2])};
                    Vector3 duF{static_cast<float>(du[0]), static_cast<float>(du[1]), static_cast<float>(du[2])};
                    Vector3 dvF{static_cast<float>(dv[0]), static_cast<float>(dv[1]), static_cast<float>(dv[2])};

                    Vector3 normal{0, 0, 0};
                    float dir = current.backFace ? -1.0f : 1.0f;
                    if (d == 0) normal.x = dir;
                    else if (d == 1) normal.y = dir;
                    else normal.z = dir;

                    Color color = ColorForBlock(static_cast<BlockId>(current.blockId));
                    builder.AddQuad(originF, duF, dvF, normal, color, current.backFace);

                    for (int l = 0; l < h; ++l) {
                        for (int k = 0; k < w; ++k) {
                            mask[n + k + l * dimsU] = MaskCell{};
                        }
                    }

                    i += w;
                    n += w;
                }
            }
        }
    }

    Mesh mesh{};
    mesh.vertexCount = static_cast<int>(builder.positions.size());
    mesh.triangleCount = builder.faceCount * 2;

    mesh.vertices = static_cast<float*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.normals = static_cast<float*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.colors = static_cast<unsigned char*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount) * 4 * sizeof(unsigned char)));
    mesh.indices = static_cast<unsigned short*>(MemAlloc(static_cast<unsigned int>(builder.faceCount) * 6 * sizeof(unsigned short)));

    for (int i = 0; i < mesh.vertexCount; ++i) {
        mesh.vertices[i * 3 + 0] = builder.positions[i].x;
        mesh.vertices[i * 3 + 1] = builder.positions[i].y;
        mesh.vertices[i * 3 + 2] = builder.positions[i].z;

        mesh.normals[i * 3 + 0] = builder.normals[i].x;
        mesh.normals[i * 3 + 1] = builder.normals[i].y;
        mesh.normals[i * 3 + 2] = builder.normals[i].z;

        mesh.colors[i * 4 + 0] = builder.colors[i].r;
        mesh.colors[i * 4 + 1] = builder.colors[i].g;
        mesh.colors[i * 4 + 2] = builder.colors[i].b;
        mesh.colors[i * 4 + 3] = builder.colors[i].a;
    }

    for (int f = 0; f < builder.faceCount; ++f) {
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
