#pragma once

#include <cstdint>
#include <mutex>
#include <vector>

#include "raylib.h" // Vector3 / Color : simples structs, aucun appel fenetre.

namespace racer::engine {

class World;

// Element de rendu fige : tout ce que le thread rendu doit connaitre d'une
// entite pour la dessiner, sans retoucher a l'ECS.
struct RenderItem {
    std::uint32_t meshId = 0;
    std::uint32_t materialId = 0;
    Vector3 position{0.0f, 0.0f, 0.0f};
    float heading = 0.0f;
    float roll = 0.0f;
    Color tint{255, 255, 255, 255};
};

// Photo complete d'une frame de simulation, consommable par le rendu.
struct FrameSnapshot {
    double simTime = 0.0;
    std::vector<RenderItem> items;
};

// Double buffering sim/rendu : le thread sim remplit un buffer pendant que
// le thread rendu lit l'autre. Contrat : la reference renvoyee par
// ReadLatest() reste valable jusqu'au Publish() suivant (le rendu doit avoir
// consomme le snapshot avant la publication d'apres). Deux buffers suffisent.
class SnapshotBuffer {
public:
    // Buffer d'ecriture (thread sim uniquement). Son contenu est l'avant-
    // dernier snapshot : l'ecrivain le reecrit entierement.
    FrameSnapshot& WriteBegin();

    // Bascule le buffer ecrit en "dernier publie" (swap des roles sous mutex).
    void Publish();

    // Dernier snapshot publie (snapshot vide par defaut avant le 1er Publish).
    const FrameSnapshot& ReadLatest() const;

private:
    FrameSnapshot buffers_[2];
    int writeIndex_ = 0; // buffer en cours d'ecriture ; l'autre est publie
    mutable std::mutex mutex_;
};

// Remplit snapshot.items depuis les entites Transform+RenderMesh du monde.
// Ne touche pas a simTime (a renseigner par l'appelant, cote sim).
void CaptureSnapshot(World& world, FrameSnapshot& snapshot);

} // namespace racer::engine
