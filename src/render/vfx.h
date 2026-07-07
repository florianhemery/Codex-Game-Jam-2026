#pragma once

#include <memory>

#include "raylib.h"

namespace racer {

// Systeme de particules du jeu : fumee de drift, poussiere hors piste,
// flammes de nitro, etincelles, confettis d'arrivee et pluie.
// - Pool fixe (4096), aucune allocation par frame.
// - Textures procedurales generees au constructeur (aucun fichier).
// - Draw s'appelle DANS BeginMode3D, apres la scene opaque (deux passes de
//   blend : alpha puis additif, ecriture de profondeur coupee).
class VfxSystem {
public:
    VfxSystem();               // genere les textures procedurales (pas de fichiers)
    ~VfxSystem();
    VfxSystem(const VfxSystem&) = delete;
    VfxSystem& operator=(const VfxSystem&) = delete;

    void Update(float dt, Vector3 focus);    // focus = position du joueur (ancre la pluie)
    void Draw(const Camera3D& camera) const; // appele DANS BeginMode3D, apres la scene opaque

    void EmitDriftSmoke(Vector3 pos, Vector3 carVel);
    void EmitOffroadDust(Vector3 pos, Vector3 carVel);
    void EmitNitroFlame(Vector3 pos, Vector3 backDir, Vector3 carVel); // backDir = -forward normalise
    void EmitSparks(Vector3 pos, Vector3 dir);
    void EmitConfetti(Vector3 pos);
    void SetRain(bool enabled);              // transition lissee (intensite 0..1 interne)
    int ActiveCount() const;
    void Clear();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace racer
