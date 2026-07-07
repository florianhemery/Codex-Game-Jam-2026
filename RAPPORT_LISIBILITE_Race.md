# Rapport Lisibilite - Module Race

**Lot :** L09  
**Module :** Race  
**Date :** 2026-07-07  
**Perimetre exclusif :** `src/Race/`

## Resume

Extraction de la resolution de contacts voiture-voiture hors de `RaceState`
vers `RaceContactResolver`. Decoupage de `tryPrepareContact` et
`applyContactDeflection` en sous-fonctions. API publique de `RaceState`
inchangee.

## Metriques avant / apres

| Fichier | Avant | Apres |
|---------|-------|-------|
| `RaceState.cpp` | 357 lignes | 214 lignes |
| `RaceState.hpp` | 100 lignes | 87 lignes |
| `RaceContactResolver.cpp` | — | 252 lignes |
| `RaceContactResolver.hpp` | — | 87 lignes |

## Fonctions decoupees

### `tryPrepareContact` -> 3 etapes

| Sous-fonction | Role |
|---------------|------|
| `isWithinContactRange` | Test distance au carre vs seuil |
| `computeContactNormal` | Normal + distance (cas degenere via fallback) |
| `applyFallbackNormal` | Normal par defaut si positions confondues |
| `tryPrepareContact` | Orchestration (<= 18 lignes) |

### `applyContactDeflection` -> 5 etapes

| Sous-fonction | Role |
|---------------|------|
| `computeDeflectScalars` | Push et deflect bornes |
| `computeHeadingSides` | Signes lateraux A/B |
| `applyHeadingDeflection` | Rotation des velocityHeading |
| `nudgeIfApproaching` | Nudge lateral si ramming |
| `applyLateralNudges` | Application des nudges A/B |
| `applyContactDeflection` | Orchestration (<= 12 lignes) |

## NEW_SOURCES

Fichiers ajoutes au build (CMakeLists.txt mis a jour sur les cibles
`racer`, `race_sim_debug`, `hud_demo`) :

```
src/Race/RaceContactResolver.cpp
src/Race/RaceContactResolver.hpp
```

## Conformite F1 / taille fichiers

| Regle | Statut |
|-------|--------|
| Fonctions <= 20 lignes | OK (toutes verifiees module Race) |
| Fichiers <= 300 lignes | OK |
| API publique `RaceState` | Inchangee |
| Modifications hors `src/Race/` | `CMakeLists.txt` uniquement (enregistrement NEW_SOURCES) |

## Commit

`refactor: lisibilite race`
