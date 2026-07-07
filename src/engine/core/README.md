# engine_core — ECS, jobs, double buffering

Fondations moteur (Phase 0) : lib statique `engine_core`, namespace `racer::engine`.
Aucune dépendance vers les autres modules engine (`rhi`, `assets`) ; seules
dépendances : EnTT v3.13.2 (header-only, FetchContent local à ce module) et
raylib (types `Vector3`/`Color` uniquement, aucun appel fenêtre/rendu).

## API

### `components.h` — composants POD de base
- `TransformComponent { Vector3 position; float heading, roll, pitch; }`
- `KinematicsComponent { float speed, velocityHeading; bool isDrifting; }`
- `RenderMeshComponent { uint32_t meshId, materialId; Color tint; }` (ids opaques, résolus plus tard par le rendu)
- `LapProgressComponent { int lap, lastSegment; bool passedMidpoint, finished; float finishTime; }`
- `NameComponent { std::string name; }`
- `PlayerTag`, `AiTag` (tags vides)

### `world.h` — `class World`, fine surcouche d'`entt::registry`
- `entt::entity CreateEntity()` / `void DestroyEntity(entt::entity)`
- `entt::registry& Registry()` (accès direct EnTT : views, groups…)
- `Add<T>(entity, args...)`, `Get<T>(entity)`, `Has<T>(entity)`

### `jobs.h` — `class JobSystem` (pool de threads)
- `JobSystem(workerCount = hardware_concurrency - 1, min 1)` ; destructeur : arrêt propre (draine la file puis join)
- `std::future<void> Submit(std::function<void()>)`
- `void ParallelFor(begin, end, grainSize, fn(index))` — bloquant, le thread
  appelant participe à l'exécution ; propage la première exception de `fn`

### `frame_snapshot.h` — découplage sim/rendu
- `RenderItem { meshId, materialId, position, heading, roll, tint }`
- `FrameSnapshot { double simTime; std::vector<RenderItem> items; }`
- `SnapshotBuffer` (2 buffers) : `WriteBegin()` → buffer d'écriture (thread sim),
  `Publish()` → swap sous mutex, `ReadLatest()` → dernier publié (thread rendu).
  Contrat : la référence de `ReadLatest()` est valable jusqu'au `Publish()` suivant.
- `CaptureSnapshot(World&, FrameSnapshot&)` : remplit `items` depuis les entités
  `Transform + RenderMesh` (`simTime` reste à la charge de l'appelant)

## Test

`engine_core_test` (cible console, headless) : 100 entités, `ParallelFor` sur les
transforms, capture/publication/relecture de snapshots, vérifications + « OK ».

## Migration prévue du jeu actuel

- `Car` (`src/vehicle/car.h`) → une entité avec `TransformComponent`
  (position/heading) + `KinematicsComponent` (speed/velocityHeading/isDrifting) ;
  le tuning et la logique `Update()` deviendront un système de simulation.
- `RacerEntry` (`src/race/race_state.h`) → entité + `LapProgressComponent`
  (lap/lastSegment/passedMidpoint/finished/finishTime) + `NameComponent` +
  `PlayerTag` ou `AiTag` (remplace `isPlayer`).
- La boucle de rendu consommera un `FrameSnapshot` publié par la sim au lieu de
  lire directement l'état du jeu ; les systèmes lourds (IA, physique) passeront
  par `JobSystem::ParallelFor`.
