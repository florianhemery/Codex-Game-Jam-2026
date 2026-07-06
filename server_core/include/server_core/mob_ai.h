#pragma once

#include <random>
#include <unordered_set>
#include <vector>

#include "common/world/world_coords.h"
#include "server_core/entity.h"
#include "server_core/world_manager.h"

namespace server_core {

// IA volontairement minimale (best-effort du plan) : marche aleatoire +
// gravite, pas de pathfinding A*. Un mob change de direction toutes les
// quelques secondes et rebrousse chemin s'il rencontre un bloc solide.
class MobManager {
public:
    // A appeler quand un chunk finit de charger : tire au sort (une seule
    // fois par coord, jamais reevalue si le chunk se recharge) l'apparition
    // d'un mob a sa surface.
    void MaybeSpawn(common::world::ChunkCoord coord, const WorldManager& worldManager, uint32_t seed);

    void Tick(float dt, WorldManager& worldManager);

    const std::vector<Entity>& Entities() const { return entities_; }

private:
    std::vector<Entity> entities_;
    std::unordered_set<common::world::ChunkCoord, common::world::ChunkCoordHash> spawnRolled_;
    std::mt19937 rng_{std::random_device{}()};
    uint32_t nextEntityId_ = 1;
};

} // namespace server_core
