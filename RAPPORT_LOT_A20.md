# Rapport Lot A20 - VFX (vfx.h / vfx.cpp)

**Fichiers traites :** `src/render/vfx.h`, `src/render/vfx.cpp`  
**Date :** 2026-07-07  
**Commit attendu :** `style: conformite coding style vfx`

## Regles appliquees localement

| Regle | Action |
|-------|--------|
| G1 | Header Epitech (`racer`) ajoute aux deux fichiers |
| H2 | `#pragma once` remplace par garde `VFX_H_` |
| F1 | `Update`, `Draw`, `DrawParticlesOfType` decoupes en sous-fonctions (<= 20 lignes) |
| F2 | 61 lignes > 80 colonnes reformatees (0 restante) |
| F3 | `EmitNitroFlame` conserve 4 arguments ; `emitQuad` / `drawParticlesOfType` a 5 args (helpers internes, voir note) |
| F5 | Tous les commentaires dans les corps de fonctions supprimes |
| G8 (partiel) | Helpers `namespace {}` migres vers `VfxInternals` (methodes `static`) |
| G2 | Ligne vide entre implementations de fonctions |
| L1-L6 | Indentation 4 espaces, accolades ouvrantes ligne suivante, une instruction par ligne |
| V1 | References `const Camera3D &camera` |
| V2 | `VfxSystem` reste une `class` |
| N4/N5 (local) | `PType` valeurs enum en `UPPER_CASE` (`DRIFT_SMOKE`, etc.) |
| K1 | Attributs `Impl` suffixes `_` (`count_`, `texPuff_`, `rainIntensity_`, ...) |
| A1 | Parametre `const Camera3D &camera` sur `Draw` |
| E6 | `alloc()` et helpers avec un seul `return` |
| C1-C5 | `else if` et profondeur de branchement inchangees fonctionnellement |
| S1-S3 | STL (`std::array`, `std::unique_ptr`) conserve |
| D1-D6 | Pimpl `Impl` preserve, encapsulation publique intacte |

## Violations corrigees

- Absence de header G1 (2 fichiers)
- `#pragma once` au lieu de garde H2
- Commentaires inline dans le header et ~40 commentaires dans `vfx.cpp` (F5)
- Fonctions > 20 lignes : `Update` (~100), `DrawParticlesOfType` (~50), `Draw` (~30), emitters
- 61 lignes depassant 80 colonnes
- Helpers libres dans namespace anonyme (G8 partiel)

## Note F3

`VfxInternals::emitQuad` et `drawParticlesOfType` gardent 5 parametres (texture + geometrie/triplet pool). Decoupage en struct locale possible en lot S4 si le checker l'exige.

## Items transverses (lots sequentiels)

| Item | Regle | Detail | Lot cible |
|------|-------|--------|-----------|
| Extension `.h` | O2 | `vfx.h` -> `vfx.hpp` puis PascalCase | S1 / S2 |
| Nom de fichier | N2 | `vfx.*` -> `VfxSystem.hpp` / `VfxSystem.cpp` | S2 |
| Chemin dossier | N2/O1 | `src/render/` -> `src/Render/` | S2 |
| API publique PascalCase | N3 | `Update`, `Draw`, `EmitDriftSmoke`, `ActiveCount`, etc. -> camelCase | S3 |
| Include guard | H2 | `VFX_H_` -> `VFXSYSTEM_HPP_` apres renommage | S1 |
| G8 strict | G8 | `VfxInternals` pourrait etre integre a `VfxSystem` ou fichier dedie | S4 |
| Appels raylib/rlgl | A7 | Appels C non encapsules (inevitables en l'etat) | info |

## Verification build

Cibles impactees : `vfx_demo`, `racer` (via `vfx.cpp`).

```text
cmake --build build --target vfx_demo
```

Resultat : **SUCCES** (`vfx_demo.exe` lie sans erreur sur `vfx.cpp`).
