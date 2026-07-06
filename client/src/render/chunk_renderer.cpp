#include "client/render/chunk_renderer.h"

#include "client/render/chunk_mesher.h"
#include "rlgl.h"

namespace client {

ChunkRenderer::~ChunkRenderer() {
    for (auto& [coord, model] : models_) {
        (void)coord;
        UnloadModel(model);
    }
}

void ChunkRenderer::UpsertChunk(const common::world::Chunk& chunk) {
    auto it = models_.find(chunk.coord);
    if (it != models_.end()) {
        UnloadModel(it->second);
        models_.erase(it);
    }

    Mesh mesh = BuildGreedyChunkMesh(chunk);
    Model model = LoadModelFromMesh(mesh);
    models_.emplace(chunk.coord, model);
}

void ChunkRenderer::RemoveChunk(common::world::ChunkCoord coord) {
    auto it = models_.find(coord);
    if (it == models_.end()) return;
    UnloadModel(it->second);
    models_.erase(it);
}

void ChunkRenderer::DrawAll(Color tint) const {
    // Le mesher ne garantit pas un ordre de sommets coherent avec le
    // backface culling par defaut ; on le desactive plutot que de risquer
    // des faces invisibles pendant l'iteration rapide du jam.
    rlDisableBackfaceCulling();
    for (const auto& [coord, model] : models_) {
        Vector3 origin{
            static_cast<float>(coord.x * common::world::CHUNK_SIZE_X),
            0.0f,
            static_cast<float>(coord.z * common::world::CHUNK_SIZE_Z),
        };
        DrawModel(model, origin, 1.0f, tint);
    }
    rlEnableBackfaceCulling();
}

} // namespace client
