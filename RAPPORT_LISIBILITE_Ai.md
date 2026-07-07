# RAPPORT_LISIBILITE_Ai - Module Ai

**Lot :** L08  
**Fichiers :** `src/Ai/AiDriver.hpp`, `src/Ai/AiDriver.cpp`  
**Date :** 2026-07-07  
**Commit :** `refactor: lisibilite ai`

## Audit F1 (fonctions <= 20 lignes)

| Fonction | Avant | Apres | Action |
|----------|-------|-------|--------|
| `normalizeAngle` | 10 | 10 | OK |
| `hashU32` | 9 | 9 | OK |
| `AIDriver` (constructeur) | 11 | 12 | OK |
| `anticipateCorners` | 23 | 9 | Extrait `scanCornerSamples` |
| `accumulateCornerSample` | 17 | 14 | OK |
| `computeTarget` | 21 | 6 | Extrait `computeLookahead`, `offsetTargetFromLane` |
| `computeHeadingError` | 7 | 7 | OK |
| `computeTargetSpeed` | 13 | 14 | OK |
| `applyThrottle` | 15 | 12 | OK |
| `applyDriveInput` | 16 | 11 | Extrait `shouldUseNitro` |
| `computeInput` | 15 | 15 | OK |
| `scanCornerSamples` | - | 17 | Nouvelle (boucle courbure) |
| `computeLookahead` | - | 3 | Nouvelle |
| `offsetTargetFromLane` | - | 17 | Nouvelle (decalage voie) |
| `shouldUseNitro` | - | 8 | Nouvelle |

**Resultat :** 0 fonction > 20 lignes (15 fonctions au total).

## Constantes constexpr extraites

| Constante | Valeur | Usage |
|-----------|--------|-------|
| `kGripBudget` | 1.4f | Budget grip virage (existant) |
| `kMinCornerSpeed` | 7.0f | Vitesse min en virage (existant) |
| `kUnlimitedSpeed` | 1e9f | Plafond vitesse initial anticipation |
| `kChordBase` | 4.0f | Base longueur corde echantillonnage |
| `kChordSpeedScale` | 0.12f | Corde proportionnelle a la vitesse |
| `kCornerSampleCount` | 5 | Nombre d'echantillons courbure |
| `kMinTurnPerUnit` | 1e-3f | Diviseur min turnPerUnit |
| `kBrakeFraction` | 0.7f | Fraction freinage tuning |
| `kLookaheadBase` | 10.0f | Distance visée de base |
| `kLookaheadSpeedScale` | 0.6f | Lookahead proportionnel vitesse |
| `kTangentDelta` | 2.0f | Delta tangentielle decalage voie |
| `kMaxTurnPerUnit` | 0.06f | Seuil courbure pour offset |
| `kOffsetScaleMin` / `kOffsetScaleMax` | 0.4f / 1.0f | Clamp echelle decalage |
| `kMinTangentLength` | 1e-4f | Longueur tangentielle min |
| `kAlignTurnScale` | 0.8f | Reduction vitesse en virage |
| `kAlignFactorMin` / `kAlignFactorMax` | 0.35f / 1.0f | Clamp facteur alignement |
| `kOverspeedRatio` | 1.08f | Seuil sur-vitesse freinage |
| `kBrakeThrottle` | -0.75f | Gaz frein fort |
| `kFullThrottle` / `kCoastThrottle` | 1.0f / 0.15f | Gaz plein / maintien |
| `kNitroMarginBase` | 6.0f | Marge nitro de base |
| `kNitroMarginSkillScale` | 8.0f | Marge nitro selon skill |
| `kNitroTurnThreshold` | 0.15f | Seuil virage pour nitro |
| `kHandbrakeTurnThreshold` | 1.2f | Seuil virage frein a main |
| `kHandbrakeMinSpeed` | 6.0f | Vitesse min frein a main |
| `kSteerGain` | 2.0f | Gain direction |
| `kSteerMin` / `kSteerMax` | -1.0f / 1.0f | Clamp direction |
| `kDefaultSeedMultiplier` | 997.0f | Graine derivee du skill |
| `kDefaultSeedOffset` | 17u | Offset graine par defaut |
| `kLaneHashModulus` | 1000u | Modulo hash personnalite |
| `kLaneHashNormalizer` | 999.0f | Normalisation hash voie |
| `kLaneOffsetRange` | 1.8f | Amplitude decalage voie |
| `kNitroReserveBase` | 1.2f | Reserve nitro de base |
| `kNitroReserveWeaknessScale` | 4.0f | Reserve nitro selon faiblesse |

## Renommages (lisibilite)

| Avant | Apres | Contexte |
|-------|-------|----------|
| `h` | `hash` | Constructeur |
| `h1`, `h2` | `prevHeading`, `nextHeading` | Echantillonnage courbure |
| `aBrake` | `brakeAccel` | Freinage anticipation |
| `ta`, `tb` | `tangentA`, `tangentB` | Tangente piste |
| `dxT`, `dzT` | `tangentDx`, `tangentDz` | Composantes tangente |
| `len` | `tangentLen` | Longueur tangente |
| `prog` | `trackProgress` | Projection sur piste |
| `s` | `sampleIndex` | Index echantillon courbure |

## Perimetre respecte

- Modifications exclusivement dans `src/Ai/`
- Aucun changement d'API publique (`AIDriver`, `computeInput`)
- Aucun changement fonctionnel (refactor structurel uniquement)

## Build

`cmake --build build --target CMakeFiles/racer.dir/src/Ai/AiDriver.cpp.obj` : OK  
(La liaison complete `racer` echoue sur des erreurs preexistantes hors lot dans `engine_render`.)
