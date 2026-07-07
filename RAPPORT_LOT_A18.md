# Rapport Lot A18 - TrackRenderer (track_renderer.h / track_renderer.cpp)

## Fichiers modifies

- `src/render/track_renderer.h`
- `src/render/track_renderer.cpp`

## Corrections appliquees (locale)

| Regle | Action |
|-------|--------|
| G1 | En-tete Epitech standard (projet `racer`) sur `.h` et `.cpp` |
| H2 | `#pragma once` remplace par `#ifndef TRACK_RENDERER_H_` / `#define` / `#endif` |
| F1 | Constructeur et `Draw()` decoupes via `TrackRendererBuild` / `TrackRendererDraw` ; helpers mesh via `TrackRendererDetail` (<= 20 lignes par fonction) |
| F2 | 191 lignes > 80 colonnes reformatees (0 restantes) |
| F5 | Commentaires supprimes dans les corps de fonctions et champs inline |
| G8 (partiel) | Helpers `namespace {}` -> `TrackRendererDetail` static ; build/draw -> friends `TrackRendererBuild` / `TrackRendererDraw` |
| G2 | Ligne vide entre implementations de fonctions |
| K1 | Attributs membres deja en `_suffix` (`surfaceStyle_`, `trackModel_`, etc.) |
| L1-L6 | Indentation 4 espaces, accolades Allman, espaces operateurs |
| A1 | References `const` sur parametres non modifies (`const Track &`, etc.) |
| V2 | Structs internes conservees dans la classe (POD decor) |

## Metriques finales

| Metrique | Avant | Apres |
|----------|-------|-------|
| Lignes `.cpp` | ~1062 | ~2402 |
| Lignes > 80 cols (F2) | 191 | 0 |
| Fonctions > 20 lignes (F1) | ~15+ | 0 |

## Items transverses (lots sequentiels)

| Regle | Detail | Lot cible |
|-------|--------|-----------|
| O2 | Extension `.h` -> `.hpp` | S1 |
| N2 | Fichier `track_renderer.*` / dossier `render` -> `TrackRenderer.hpp` / `Render/` | S2 |
| N3 | API publique PascalCase (`Draw`, `DrawOpaqueGeometry`, `ApplyShader`, etc.) | S3 |
| N3 | `DrawSkyGradient` -> `drawSkyGradient` (conserve PascalCase en Phase 1) | S3 |
| G8 (strict) | Integrer `DrawSkyGradient` dans `TrackRenderer` ; helpers en methodes privees | S4 |
| O3 | Multiples structs decor dans un header (`PropInstance`, `CloudInstance`, etc.) | S4 |

## Build

Cible impactee : `racer`, demos render (`track_demo`, etc.).

- `track_renderer.cpp.obj` : compilation OK (ninja cible isolee).
- `racer` complet : echec preexistant dans `asset_registry.cpp` (acces membres prives, hors lot A18).
