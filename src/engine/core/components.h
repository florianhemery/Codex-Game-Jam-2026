#pragma once

#include <cstdint>
#include <string>

#include "raylib.h" // Vector3 / Color : simples structs, aucun appel fenetre.

namespace racer::engine {

// Position et orientation d'une entite dans le monde.
struct TransformComponent {
    Vector3 position{0.0f, 0.0f, 0.0f};
    float heading = 0.0f; // radians, 0 = +Z (convention du jeu actuel)
    float roll = 0.0f;
    float pitch = 0.0f;
};

// Etat cinematique arcade (voir vehicle/car.h pour le modele d'origine).
struct KinematicsComponent {
    float speed = 0.0f;           // scalaire signe le long de velocityHeading
    float velocityHeading = 0.0f; // direction reelle du deplacement
    bool isDrifting = false;
};

// Reference de rendu : ids opaques resolus par le module rendu.
// Volontairement AUCUNE dependance vers engine/rhi.
struct RenderMeshComponent {
    std::uint32_t meshId = 0;
    std::uint32_t materialId = 0;
    Color tint{255, 255, 255, 255};
};

// Progression de course (reprend les champs de RacerEntry).
struct LapProgressComponent {
    int lap = 0;
    int lastSegment = 0;
    bool passedMidpoint = false; // garde-fou anti faux-tour
    bool finished = false;
    float finishTime = 0.0f;
};

// Nom affichable (pilote, objet nomme...).
struct NameComponent {
    std::string name;
};

// Tags vides : discriminent joueur et IA sans payload.
struct PlayerTag {};
struct AiTag {};

} // namespace racer::engine
