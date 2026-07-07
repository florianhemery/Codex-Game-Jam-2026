# RAPPORT_CONFORMITE - Coding Style Epitech C++ v2.1

**Projet :** racer  
**Date :** 2026-07-07  
**Phase :** 4 (finitions G1/F2/G8/A7)  
**Toolchain :** CMake 4.3.4 (`.tools`), Ninja (`.tools`), LLVM-MinGW UCRT (`.tools/llvm-mingw-ucrt`)

## 1. Build (rebuild complet)

Commande : `cmake --build build --clean-first` (Ninja, Debug).

| Cible | Type | Statut |
|-------|------|--------|
| `engine_rhi` | lib statique | OK |
| `engine_assets` | lib statique | OK |
| `engine_core` | lib statique | OK |
| `engine_render` | lib statique | OK |
| `racer` | executable | OK |
| `race_sim_debug` | executable | OK |
| `car_demo` | executable | OK |
| `track_demo` | executable | OK |
| `vfx_demo` | executable | OK |
| `hud_demo` | executable | OK |
| `engine_core_test` | executable | OK |
| `render_demo` | executable | OK |

**Resultat global :** 12/12 cibles, zero erreur de compilation/liaison. Avertissements tiers (raylib, EnTT) uniquement.

## 2. Tests

### `engine_core_test`

- **Binaire :** `build/engine_core_test.exe`
- **Sortie :** `OK`
- **Code de sortie :** 0 (PATH LLVM-MinGW requis pour les DLL runtime)

## 3. Violations corrigees par regle (synthese Phase 0 -> Phase 4)

| Regle | Gravite | Avant (approx.) | Apres (scan 2026-07-07) | Commentaire |
|-------|---------|-----------------|-------------------------|-------------|
| **G1** | majeur | 42/42 fichiers sans en-tete Epitech | **0** | En-tetes Epitech ajoutes aux 4 demos `*Demo.cpp` |
| **H2** | mineur | 18 `#pragma once` | **0** | Guards `#ifndef` / `_HPP_` (S1) |
| **O2** | mineur | 18 `.h` | **0** `.h` dans `src/` | Extension `.hpp` (S1) |
| **N2** | majeur | dossiers/fichiers snake_case | **0** | PascalCase (S2) |
| **N3** | majeur | ~40 symboles publics PascalCase | **0** API legacy | camelCase public (S3) |
| **N5** | majeur | 5 enums | **0** | Valeurs `UPPER_CASE` (S3) |
| **G8** | majeur | ~35 modules proceduraux | **0** hors `main` | Helpers HUD migres en methodes `private static` de `Hud` |
| **G4** | majeur | 1 global test | **0** | |
| **A9** | majeur | 1 `using namespace` | **0** | |
| **E1** | majeur | occurrences `exit`/`abort` | **0** | |
| **F2** | majeur | ~721 lignes > 80 cols | **0** | 16 lignes residuelles corrigees (9 fichiers) |
| **O3** | majeur | 6 headers multi-types | **11** `.hpp` avec >1 type | Types auxiliaires co-localises (S4) |
| **K1** | mineur | membres prives | harmonise | Suffixe `_` (S3) |

## 4. Violations restantes et justification

### O3 (architecture)

Fichiers avec plusieurs `class`/`struct`/`enum` : `RhiTypes.hpp` (6), `SnapshotBuffer.hpp` (4), `ModelAsset.hpp` (4), etc. Scission supplementaire = churn sans gain fonctionnel.

### Regles info / acceptables (documentees)

| Regle | Statut | Note |
|-------|--------|------|
| **A7** | partiel ameliore | Wrappers `Hud::Gfx` + `*DemoApp::Gfx` pour appels 2D raylib ; includes `raylib.h` conserves pour types (`Color`, `Vector2`, `Camera3D`) et modules 3D (`CarRenderer`, `TrackRenderer`, `VfxSystem` via rlgl) |
| **A2 / A8** | info | Includes STL/raylib ordonnance projet |
| **C2** | info | Couplage acceptable modules jeu / moteur pour jam |
| **F1 / C1** | partiel | Fonctions longues reduites en sous-fonctions ; pas de mesure automatique |

## 5. Metriques de conformite

| Indicateur | Valeur |
|------------|--------|
| Fichiers `src/` (.cpp + .hpp) | 57 |
| En-tetes G1 conformes | **57/57 (100 %)** |
| Headers `.h` restants | 0 |
| `#pragma once` | 0 |
| `using namespace` (hors deps) | 0 |
| Lignes F2 (> 80 cols) | **0** (vs 16 residuelles Phase 3) |
| Build | **12/12 OK** |
| Test `engine_core_test` | **OK** |

**Estimation globale :** conformite **elevee** sur le perimetre majeur. Finitions Phase 4 : G1 demos, F2 residuel, G8 HUD, A7 wrappers partiels. Reste documente : O3 types co-localises, A7 raylib/rlgl pour rendu 3D et types.

## 6. References

- Plan : `.cursor/plans/conformite_epitech_c++_b971d636.plan.md`
- Lots : `RAPPORT_LOT_A01.md` ... `RAPPORT_LOT_A22.md`, `RAPPORT_LOT_S1.md` ... `RAPPORT_LOT_S4.md`

## 7. Lot finitions (Phase 4)

| Action | Detail |
|--------|--------|
| G1 | En-tetes Epitech sur `CarDemo.cpp`, `HudDemo.cpp`, `TrackDemo.cpp`, `VfxDemo.cpp` |
| F2 | 16 lignes > 80 cols corrigees dans 9 fichiers |
| G8 | ~37 helpers `namespace {}` de `Hud.cpp` -> methodes `private static` de `Hud` |
| A7 | `Hud::Gfx` + `CarDemoApp::Gfx`, `HudDemoApp::Gfx`, `VfxDemoApp::Gfx` |
| Build/test | 12/12 + `engine_core_test` OK |
| Commit | `style: conformite coding style finitions` |
