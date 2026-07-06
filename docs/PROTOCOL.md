# PROTOCOL.md — Contrat de messages `client` ↔ `server_core`

Ce document décrit l'API réelle telle qu'implémentée à la fin des 7 jours (pas telle que planifiée en théorie — voir `docs/PLAN.md` pour l'historique de conception). C'est la seule interface que `client/` connaisse : tout transite par `Transport` (`common/transport/transport.h`) sous forme de `ReliableMessage` ou `UnreliableMessage`.

Le jeu est solo pour l'instant (`LoopbackTransport`), mais l'API est volontairement pensée multi-client (`ClientId` explicite partout) pour qu'un futur `SocketTransport` puisse la réutiliser sans modification — voir l'Annexe de `docs/PLAN.md`.

---

## 1. Blocs (`common/world/block.h`)

| Id | Nom | Notes |
|---|---|---|
| 0 | Air | vide |
| 1 | Stone | |
| 2 | Dirt | |
| 3 | Grass | bloc de surface (Plaines/Montagnes) |
| 4 | Sand | bloc de surface (Désert) ; `IsFallable` |
| 5 | Gravel | `IsFallable` |
| 6 | Water | statique, remplit les creux sous y=16 |
| 7 | Wood | tronc d'arbre |
| 8 | Leaves | canopée d'arbre |
| 9 | Planks | craft : 1 Wood (centre) → 4 Planks |
| 10 | Stick | craft : 2 Planks (colonne du milieu) → 4 Stick |

## 2. Chunk (`common/world/chunk.h`)

- Dimensions : `16 (X) × 64 (Y) × 16 (Z)`, soit 16384 blocs par chunk.
- `BlockIndex(x,y,z) = (y*16 + z)*16 + x` (Y-majeur).
- `ChunkCoord{x,z}` identifie un chunk ; `WorldToChunkCoordInt`/`WorldToLocal` (`common/world/world_coords.h`) font la conversion monde↔chunk (division entière correcte pour les coordonnées négatives).

---

## 3. Messages fiables (`ReliableMessage`, équivalent TCP)

Portés dans une file par client (`ClientChannel::outgoingReliable`), consommés via `poll_outgoing_reliable`. Ordre de livraison préservé (FIFO), jamais perdus (dans le process courant — la vraie fiabilité réseau reste à faire par `SocketTransport`).

| Type | Sens | Payload | Description |
|---|---|---|---|
| `ChunkData` | serveur→client | `ChunkDataMsg{coord, blocks[16384]}` | Chunk complet, envoyé quand il entre dans le rayon de vue du joueur |
| `ChunkUnload` | serveur→client | `ChunkUnloadMsg{coord}` | Le chunk sort du rayon de vue, le client peut libérer son mesh |
| `BlockUpdate` | serveur→client | `BlockUpdateMsg{coord, lx,ly,lz, newBlockId}` | Résultat authoritaire d'un changement de bloc (casser/placer/chute physique), diffusé à tous les clients qui ont ce chunk chargé |
| `BreakBlockRequest` | client→serveur | `BreakBlockRequestMsg{coord, lx,ly,lz}` | Intention de casser. Le serveur valide (bloc non-air), ajoute l'item à l'inventaire, répond par `BlockUpdate`+`InventoryUpdate` |
| `PlaceBlockRequest` | client→serveur | `PlaceBlockRequestMsg{coord, lx,ly,lz, hotbarSlot}` | Intention de poser. Le serveur choisit **lui-même** le bloc posé (celui du slot indiqué) — ne fait jamais confiance à un blockId envoyé par le client |
| `InventoryUpdate` | serveur→client | `InventoryUpdateMsg{slots[36]}` | Snapshot complet de l'inventaire (36 slots, les 9 premiers = hotbar côté client), envoyé à chaque changement |
| `SpawnState` | serveur→client | `SpawnStateMsg{posX,posY,posZ, health, hunger}` | Envoyé juste après `connect_client`, avant tout `ChunkData` : position/état restaurés depuis la sauvegarde disque, ou valeurs par défaut sinon |
| `HealthHungerUpdate` | serveur→client | `HealthHungerUpdateMsg{health, hunger}` | Envoyé quand l'un des deux change (faim: -1/s ; santé: -1/2s une fois affamé, plancher à 1 — pas de mort/respawn en V1) |
| `CraftRequest` | client→serveur | `CraftRequestMsg{gridSlots[9]}` | Grille 3×3 ligne-majeure référençant des **index de slot d'inventaire** (`kCraftSlotEmpty=255` = case vide), jamais des blockId directs |
| `CraftResponse` | serveur→client | `CraftResponseMsg{success, resultBlockId, resultCount}` | Résultat du crafting ; en cas de succès, les slots référencés ont déjà été débités et `InventoryUpdate` suit |

---

## 4. Messages non-fiables (`UnreliableMessage`, équivalent UDP)

Pas de file FIFO stricte nécessaire côté conception (l'état le plus récent suffit), tolérants à la perte — le prochain tick renvoie une valeur à jour de toute façon.

| Type | Sens | Payload | Description |
|---|---|---|---|
| `PlayerInput` | client→serveur | `PlayerInputMsg{posX,posY,posZ, yaw,pitch, actionFlags, tick}` | Porte la position **courante** du joueur (pas des deltas d'intention) — simplification déliberée tant qu'il n'y a pas d'enjeu anti-triche (voir §5). Utilisé par `server_core` uniquement pour savoir autour de quel point streamer les chunks |
| `WorldTime` | serveur→client | `WorldTimeMsg{timeOfDay01}` | Horloge de jeu partagée (0=minuit, 0.5=midi), diffusée à tous chaque tick, cycle de 180s |
| `EntitySnapshot` | serveur→client | `EntitySnapshotMsg{entities: vector<EntityStateWire>}` | Un seul message par tick contenant tous les mobs (`id, mobType, x,y,z, yaw`), pas un message par entité |

---

## 5. Écarts assumés par rapport au plan v2 initial

- **`PlayerInputMsg` porte une position, pas des deltas d'intention.** Le plan prévoyait un futur mouvement simulé côté serveur (anti-triche). Sans réseau réel, il n'y a personne à tricher : le client calcule sa caméra localement et informe simplement le serveur d'où streamer. À corriger si `SocketTransport` voit le jour.
- **`tick(ServerCore*, float dt)` n'accumule pas en interne à 20 Hz fixe** — il utilise directement le `dt` de la frame client (`transport_tick` appelé une fois par frame via `LoopbackTransport`). Ça fonctionne correctement en solo (santé/faim utilisent déjà des accumulateurs internes tolérants à un pas de temps variable), mais dévie du plan initial qui visait une cadence fixe pour préparer un futur serveur dédié tournant indépendamment du framerate d'un client. À corriger avant d'écrire `SocketTransport`.
- **Pas de mort/respawn** : la santé plafonne à 1, jamais 0. Simplification volontaire (hors scope 7 jours).
- **Pas de pathfinding A\*** pour les mobs : marche aléatoire + gravité uniquement (le plan l'autorisait explicitement comme premier candidat à la coupe).
- **`SocketTransport` n'existe pas** : `transports/socket/` reste vide, conformément au plan (hors scope garanti des 7 jours).
