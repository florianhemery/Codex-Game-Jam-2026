# RAPPORT_LOT_A19 - HUD (hud.h / hud.cpp)

**Lot :** A19  
**Date :** 2026-07-07  
**Fichiers modifies :** `src/render/hud.h`, `src/render/hud.cpp`

## Regles appliquees localement

| Regle | Action |
|-------|--------|
| **G1** | Header Epitech (`racer`) ajoute aux deux fichiers |
| **H2** | `#pragma once` remplace par `#ifndef HUD_H_` / `#endif` |
| **F1** | Decoupage des fonctions longues (`drawSpeedGauge`, `drawMinimap`, `drawStandingsPanel`, `drawFinishScreen`, `drawTrackCard`, `DrawMenu`) en sous-fonctions |
| **F2** | Toutes les lignes ramenees a 80 colonnes max |
| **F3** | Structs de parametres (`ShadowTextParams`, `WrappedTextParams`, `GaugeArcParams`, `StandingsRowParams`, `FinishRowParams`) pour fonctions a plus de 4 arguments |
| **F5** | Suppression de tous les commentaires dans les corps de fonctions |
| **L1-L6** | Une instruction par ligne, indentation 4 espaces, espacement operateurs, accolades L4, declarations L5 |
| **V1** | `*` et `&` attaches au type variable (`const RaceState &race`) |
| **V2** | `MapProjection` conserve en struct POD avec methode `apply` |
| **C1-C5** | `switch` sur `RacePhase`, range-for, nesting reduit par extraction |
| **A1** | References `const` sur parametres en lecture seule |
| **E1-E6** | Pas de `exit`/`abort` ; retours uniques la ou pertinent |
| **K1-K6** | Helpers locaux en camelCase ; types PascalCase (`HudExtras`, `TrackPreview`) |
| **S1-S3** | `std::vector`, `std::clamp`, `std::move`, algorithmes STL |
| **D1-D6** | Responsabilite unique par sous-fonction de rendu |

## Renommages locaux (portee fichier)

Helpers anonymes PascalCase -> camelCase (`formatTime`, `drawPanel`, `drawMinimap`, etc.).  
API publique **inchangee** : `DrawHud`, `DrawHudEx`, `DrawMenu`, `HudExtras`.

## Items transverses (lots sequentiels)

| Item | Regle | Action requise |
|------|-------|----------------|
| Extension `.h` -> `.hpp` | O2 / S1 | Renommer `hud.h` -> `hud.hpp`, maj includes cross-lot |
| Arborescence PascalCase | N2 / S2 | `src/render/hud.*` -> `src/Render/Hud.hpp/.cpp` |
| API publique camelCase | N3 / S3 | `DrawHud` -> `drawHud`, `DrawHudEx` -> `drawHudEx`, `DrawMenu` -> `drawMenu` |
| Enum `SurfaceStyle::Abimee` | N5 / S3 | -> `SurfaceStyle::ABIMEE` (usage dans `drawTrackCardBadge`) |
| Conversion classe `Hud` | G8 / S4 | Remplacer fonctions libres par classe avec `draw()`, `drawEx()`, `drawMenu()` |
| Appels raylib directs | A7 | Encapsulation eventuelle dans couche Device (hors lot) |

## Verification build

```
cmake --build build --target hud_demo
```

**Resultat :** SUCCES (compilation + liaison `hud_demo.exe`).
