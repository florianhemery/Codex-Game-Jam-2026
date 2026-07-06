#pragma once

#include <cstdint>
#include <deque>
#include <vector>

#include "server_core/world_manager.h"

namespace server_core {

struct BlockChangeEvent {
    int worldX, worldY, worldZ;
    uint8_t newBlockId;
};

// Sable/gravier tombent si le bloc en dessous devient de l'air. File
// "dirty" plafonnee par tick pour eviter qu'une grosse colonne qui s'effondre
// d'un coup ne bloque un tick (cf. plan jour 5).
class BlockPhysics {
public:
    // A appeler apres tout changement de bloc (casser/placer/chute) : le
    // bloc au-dessus peut devenir instable.
    void NotifyChanged(int worldX, int worldY, int worldZ);

    // Traite jusqu'a un plafond d'entrees en attente, applique les chutes
    // via worldManager et remplit outEvents pour la diffusion BlockUpdate.
    void Tick(WorldManager& worldManager, std::vector<BlockChangeEvent>& outEvents);

private:
    struct Pos {
        int x, y, z;
    };
    std::deque<Pos> pending_;
};

} // namespace server_core
