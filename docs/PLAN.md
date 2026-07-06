# Plan technique — Voxel game jam (7 jours, C++/raylib)

**v2 — pivot d'architecture : solo d'abord, transport abstrait, réseau réel différé**

Ce document remplace la v1. Changement majeur : les 7 jours produisent un **jeu voxel solo complet et stable**, avec une architecture conçue pour que le vrai multijoueur (sockets TCP/UDP) ne nécessite plus tard qu'un **nouveau module de transport**, sans toucher ni au client ni à la logique de simulation.

---

## 0. Principe directeur

Trois règles non-négociables, qui découlent directement de votre demande :

1. **`server_core` ne connaît rien du réseau.** C'est une lib pure : des fonctions, des structs de messages, un `tick()`. Elle ne fait ni `socket()`, ni `send()`, ni sérialisation en octets. Elle ne dépend pas de raylib non plus (testable/headless).
2. **`client` ne connaît rien de `server_core`.** Il ne voit qu'un `Transport*` et des structs de messages (`ReliableMessage`/`UnreliableMessage`). Il appelle `send()`/`poll()`/`tick()` sur le transport, jamais une fonction de `server_core` directement.
3. **Le contrat entre les deux est un ensemble de messages**, pas des appels de fonction ad hoc ou des pointeurs partagés vers l'état interne du monde. C'est ce contrat (défini dans `common/messages/`) qui restera strictement identique quand on branchera un vrai réseau — seul son mode de transport changera.

Conséquence directe : le risque du planning se déplace entièrement du réseau (qui était le point le plus incertain en v1) vers le contenu du jeu — ce qui est une bien meilleure position pour un solo de 7 jours, puisque chaque brique de gameplay est indépendamment testable sans jamais lancer deux process.

### Verdict scope (mise à jour v2)

Sans le risque réseau, la quasi-totalité du scope original redevient réaliste en 7 jours :

| Feature | Verdict v2 |
|---|---|
| Chunks, biomes, grottes, streaming | **Réaliste**, jour 1-2 |
| Greedy meshing | **Réaliste**, jour 3 |
| Casser/placer, DDA raycast | **Réaliste**, jour 3 |
| Inventaire + hotbar | **Réaliste**, jour 3 |
| Persistance monde + joueur | **Réaliste**, jour 4 |
| Santé/faim, jour-nuit | **Réaliste**, jour 4 |
| Sable/gravier, eau statique, arbres | **Réaliste**, jour 5 |
| **Crafting grille 3×3 réelle** | **Redevient réaliste** (n'est plus un fallback dégradé) — jour 6 |
| Mobs + IA simple | **Réaliste en version wander**, A* voxel **en best-effort** — jour 6 |
| Multijoueur réel (process séparés, vraies sockets) | **Hors scope 7 jours / stretch day 7 optionnel** — voir Annexe |

Ordre de coupe en cas de retard, inchangé dans l'esprit : A* voxel → crafting grille (fallback recettes fixes) → eau/physique de blocs → arbres/polish jour-nuit → greedy meshing (fallback mesher naïf) → sauvegarde incrémentale (fallback save-on-exit). **Jamais coupé** : le découpage `server_core`/`transport`/`client` lui-même — c'est la seule chose que la v1 sacrifiait à tort en mélangeant réseau et gameplay dès le départ.

---

## 1. Architecture

### 1.1 Arborescence

```
voxel-game/
├── CMakeLists.txt
├── docs/
│   ├── PROTOCOL.md              # spec des messages = la vraie API client <-> server_core
│   └── PLAN.md
├── common/                      # zéro dépendance raylib, zéro dépendance socket
│   ├── include/common/
│   │   ├── world/
│   │   │   ├── block.h          # enum BlockId, BlockDef
│   │   │   ├── chunk.h          # struct Chunk, ChunkCoord, BlockIndex()
│   │   │   └── world_coords.h
│   │   ├── noise/
│   │   │   ├── perlin.h
│   │   │   └── simplex.h
│   │   ├── messages/
│   │   │   ├── reliable_messages.h    # ChunkData, ChunkUnload, BlockUpdate,
│   │   │   │                          # PlayerJoin/Leave, InventoryUpdate,
│   │   │   │                          # HealthHungerUpdate, CraftRequest/Response
│   │   │   └── unreliable_messages.h  # PlayerInput, PlayerSnapshot
│   │   ├── transport/
│   │   │   └── transport.h      # struct Transport + TransportVTable
│   │   └── util/ (logger.h, ring_buffer.h)
│   └── src/**/*.cpp
├── server_core/                 # lib statique — LE "serveur", sans réseau
│   ├── include/server_core/
│   │   ├── server_core.h        # API opaque : create/destroy/tick/connect_client/
│   │   │                        # submit_reliable/submit_unreliable/poll_outgoing_*
│   │   ├── world_manager.h
│   │   ├── chunk_generator.h
│   │   ├── chunk_storage.h
│   │   ├── block_physics.h
│   │   ├── player_state.h
│   │   ├── inventory.h
│   │   ├── crafting.h
│   │   ├── entity.h
│   │   └── mob_ai.h
│   └── src/**/*.cpp
├── transports/
│   ├── loopback/
│   │   ├── loopback_transport.h
│   │   └── loopback_transport.cpp     # implémentation active en V1
│   └── socket/                        # vide/squelette — hors scope 7 jours, voir Annexe
├── client/                      # seul exécutable jouable de la V1
│   ├── include/client/
│   │   ├── render/ (chunk_mesher.h, chunk_renderer.h, sky_renderer.h, frustum.h)
│   │   ├── world/ (client_world.h)     # cache local des chunks reçus via transport
│   │   ├── ui/ (hud.h, inventory_ui.h, crafting_ui.h)
│   │   └── input/ (input_handler.h, voxel_raycast.h)
│   └── src/main.cpp + src/**/*.cpp
├── tools/
│   └── worldgen_debug/          # CLI headless : tick server_core sans fenêtre, pour debug rapide
└── docker/                      # gardé pour plus tard, non utilisé en V1
    └── Dockerfile.server
```

### 1.2 Garantie du découpage au niveau du build (pas juste "discipline")

Le `CMakeLists.txt` du target `client` **n'ajoute pas** `server_core/include` à ses include paths — seulement `common/include` et `transports/loopback/`. Résultat : si du code client tente un jour d'appeler directement une fonction de `server_core` (par facilité, en plein rush de jam), **ça ne compile pas**. La frontière n'est pas qu'une convention qu'on espère respecter sous pression, elle est imposée par le système de build.

### 1.3 `ITransport` — struct C de pointeurs de fonctions

```c
// common/transport/transport.h
struct Transport;   // opaque

struct TransportVTable {
    void (*send_reliable)(Transport* self, const ReliableMessage& msg);
    bool (*poll_reliable)(Transport* self, ReliableMessage& out);      // false = rien à lire

    void (*send_unreliable)(Transport* self, const UnreliableMessage& msg);
    bool (*poll_unreliable)(Transport* self, UnreliableMessage& out);

    void (*tick)(Transport* self, float dt);   // fait avancer server_core (loopback) ou poll les sockets (futur)
    void (*destroy)(Transport* self);
};

struct Transport {
    const TransportVTable* vtable;
    void* impl;   // opaque : LoopbackState* aujourd'hui, SocketState* plus tard — jamais interprété hors de l'implémentation
};

inline void transport_send_reliable(Transport* t, const ReliableMessage& m)     { t->vtable->send_reliable(t, m); }
inline bool transport_poll_reliable(Transport* t, ReliableMessage& out)        { return t->vtable->poll_reliable(t, out); }
inline void transport_send_unreliable(Transport* t, const UnreliableMessage& m) { t->vtable->send_unreliable(t, m); }
inline bool transport_poll_unreliable(Transport* t, UnreliableMessage& out)     { return t->vtable->poll_unreliable(t, out); }
inline void transport_tick(Transport* t, float dt) { t->vtable->tick(t, dt); }
inline void transport_destroy(Transport* t)         { t->vtable->destroy(t); }
```

Struct de function pointers plutôt qu'une classe abstraite C++ : ça garde la frontière explicite et neutre (ni `server_core` ni `client` n'ont besoin de connaître un type concret au-delà d'un pointeur + vtable), et ça matérialise littéralement la règle "`server_core` ne connaît rien du réseau" — même le vtable vit dans `common/`, à égale distance des deux côtés.

### 1.4 `server_core` — API publique

```cpp
// server_core/include/server_core/server_core.h
namespace server_core {

using ClientId = uint32_t;
struct ServerCore;   // opaque

struct ServerConfig { std::string worldSaveDir; uint32_t seed; };

ServerCore* create(const ServerConfig& cfg);
void destroy(ServerCore* sc);

// Remplace le handshake réseau : même en solo, le joueur local passe par ce chemin
ClientId connect_client(ServerCore* sc, const std::string& playerName);
void disconnect_client(ServerCore* sc, ClientId id);

void submit_reliable(ServerCore* sc, ClientId from, const ReliableMessage& msg);
void submit_unreliable(ServerCore* sc, ClientId from, const UnreliableMessage& msg);

// Accumule dt en interne et exécute des pas de simulation fixes (20 Hz),
// quel que soit le rythme auquel tick() est appelé — le solo tourne donc
// déjà à la cadence exacte qu'utilisera le futur serveur dédié.
void tick(ServerCore* sc, float dt);

bool poll_outgoing_reliable(ServerCore* sc, ClientId to, ReliableMessage& out);
bool poll_outgoing_unreliable(ServerCore* sc, ClientId to, UnreliableMessage& out);

} // namespace server_core
```

Point important : l'API reste **shaped pour le multi-client** dès aujourd'hui (`ClientId` explicite partout, `connect_client` diffuse un `PlayerJoin` — qui n'a juste personne à qui être diffusé en solo). Rien dans `server_core` ne suppose "un seul joueur" ; le jour où `SocketTransport` appellera `connect_client()` pour chaque connexion entrante, aucune ligne de `server_core` ne bougera.

### 1.5 `LoopbackTransport`

```cpp
// transports/loopback/loopback_transport.cpp
struct LoopbackState {
    server_core::ServerCore* core;
    server_core::ClientId localClientId;
};

static void lb_send_reliable(Transport* self, const ReliableMessage& msg) {
    auto* st = static_cast<LoopbackState*>(self->impl);
    server_core::submit_reliable(st->core, st->localClientId, msg);
}
static bool lb_poll_reliable(Transport* self, ReliableMessage& out) {
    auto* st = static_cast<LoopbackState*>(self->impl);
    return server_core::poll_outgoing_reliable(st->core, st->localClientId, out);
}
// lb_send_unreliable / lb_poll_unreliable : symétrique

static void lb_tick(Transport* self, float dt) {
    auto* st = static_cast<LoopbackState*>(self->impl);
    server_core::tick(st->core, dt);   // le solo fait tourner la simulation ICI, dans le tick du transport
}
static void lb_destroy(Transport* self) {
    auto* st = static_cast<LoopbackState*>(self->impl);
    server_core::destroy(st->core);
    delete st; delete self;
}

static const TransportVTable kLoopbackVTable {
    lb_send_reliable, lb_poll_reliable, lb_send_unreliable, lb_poll_unreliable, lb_tick, lb_destroy
};

Transport* loopback_transport_create(const server_core::ServerConfig& cfg, const std::string& playerName) {
    auto* st = new LoopbackState{ server_core::create(cfg), 0 };
    st->localClientId = server_core::connect_client(st->core, playerName);
    auto* t = new Transport{ &kLoopbackVTable, st };
    return t;
}
```

### 1.6 Boucle du client (identique en solo et en futur multijoueur)

```cpp
Transport* transport = loopback_transport_create(cfg, "Player1");

while (!WindowShouldClose()) {
    float dt = GetFrameTime();

    transport_send_unreliable(transport, make_player_input(camera, inputState));
    transport_tick(transport, dt);

    ReliableMessage rmsg;
    while (transport_poll_reliable(transport, rmsg))   handle_reliable(clientWorld, rmsg);
    UnreliableMessage umsg;
    while (transport_poll_unreliable(transport, umsg)) handle_unreliable(clientWorld, umsg);

    render(clientWorld);
}
transport_destroy(transport);
```

Ce bloc ne change **pas un seul caractère** le jour où `loopback_transport_create(...)` devient `socket_transport_create(serverAddress, ...)`.

### 1.7 Faut-il sérialiser "à blanc" en octets dès maintenant ?

**Non, pas en octets — mais oui en passage par valeur dans les structs de message.** Deux niveaux à distinguer :

- **Le passage par la forme "message"** (copier un `ChunkDataMsg{coord, blocks[16384]}` dans une file plutôt que de donner au client un pointeur/référence direct vers le `Chunk` interne de `server_core`) : **on le fait dès maintenant**, systématiquement. C'est ce qui garantit que le client ne dépend jamais de la représentation interne du monde côté serveur, et que la frontière testée aujourd'hui est la même que celle qu'un vrai réseau utilisera.
- **L'encodage en octets bruts** (`ByteWriter`/`ByteReader`, endianness, framing) : **pas maintenant**. Ça n'apporte aucune valeur tant qu'il n'y a pas de vrai câble à traverser, et ça ajoute une classe de bugs (encodage/décodage, taille de buffer) sans rapport avec le jeu lui-même. Cette étape est reportée à l'implémentation de `SocketTransport` (Annexe), où elle devient un travail mécanique et isolé — les structs de message auront déjà été stabilisées et testées pendant 7 jours de jeu solo, ce qui réduit fortement le risque au moment de l'écrire.

Coût du choix "copie par valeur, pas d'octets" : un `ChunkDataMsg` fait 16 Ko copié à chaque chargement de chunk — négligeable (quelques centaines de chunks par session, microsecondes chacun), aucun impact mesurable à l'échelle solo.

---

## 2. Format de données

Inchangé dans son contenu, juste réinterprété comme "structure des messages", pas comme "format réseau" :

```cpp
namespace common::world {

constexpr int CHUNK_SIZE_X = 16;
constexpr int CHUNK_SIZE_Z = 16;
constexpr int CHUNK_SIZE_Y = 64;
constexpr int CHUNK_BLOCK_COUNT = CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z; // 16384

struct ChunkCoord { int32_t x; int32_t z; };

inline int BlockIndex(int x, int y, int z) {
    return (y * CHUNK_SIZE_Z + z) * CHUNK_SIZE_X + x;   // Y-majeur, pratique pour le greedy meshing
}

struct Chunk {
    ChunkCoord coord;
    std::array<uint8_t, CHUNK_BLOCK_COUNT> blocks;   // 0 = air
};

} // namespace
```

```cpp
// common/messages/reliable_messages.h
enum class ReliableMsgType : uint8_t {
    ChunkData, ChunkUnload, BlockUpdate, PlayerJoin, PlayerLeave,
    InventoryUpdate, HealthHungerUpdate, CraftRequest, CraftResponse, ChatMessage,
};

struct ChunkDataMsg    { ChunkCoord coord; std::array<uint8_t, CHUNK_BLOCK_COUNT> blocks; };
struct ChunkUnloadMsg  { ChunkCoord coord; };
struct BlockUpdateMsg  { ChunkCoord coord; uint8_t lx, ly, lz; uint8_t newBlockId; };
// ... InventoryUpdateMsg, HealthHungerUpdateMsg, CraftRequestMsg/CraftResponseMsg, etc.

struct ReliableMessage {
    ReliableMsgType type;
    std::variant<ChunkDataMsg, ChunkUnloadMsg, BlockUpdateMsg, /* ... */> payload;
};
```

```cpp
// common/messages/unreliable_messages.h
enum class UnreliableMsgType : uint8_t { PlayerInput, PlayerSnapshot };

struct PlayerInputMsg    { float moveX, moveZ; float yaw, pitch; uint8_t actionFlags; uint32_t tick; };
struct PlayerSnapshotMsg { std::vector<EntitySnapshot> entities; uint32_t tick; };  // 1 seul par tick, toutes entités visibles

struct UnreliableMessage {
    UnreliableMsgType type;
    std::variant<PlayerInputMsg, PlayerSnapshotMsg> payload;
};
```

`std::variant` plutôt qu'une union C manuelle : le projet est C++17/20, `variant` évite les pièges classiques de lifetime sur des unions contenant des types non-triviaux (`std::vector`, `std::array`), sans coût de conception supplémentaire.

**Format de sauvegarde disque** (inchangé) : un fichier par chunk modifié (`world/chunks/{cx}_{cz}.chunk`, header 9 octets + 16 Ko de blocs bruts) et un fichier par joueur (`world/players/{name}.dat` : position, orientation, santé, faim, inventaire). `server_core/chunk_storage.*` et `server_core/persistence` (dossier fusionné dans `server_core` puisqu'il n'y a plus de séparation "serveur réseau").

---

## 3. Plan jour par jour (7 jours, solo)

### Jour 1 — Fondations, contrat message-based
- **Fichiers** : `CMakeLists.txt` (racine + `common`/`server_core`/`transports/loopback`/`client`/`tools`), `common/world/{block,chunk,world_coords}.h`, `common/messages/{reliable_messages,unreliable_messages}.h` (juste `ChunkData` + `PlayerInput` pour commencer), `common/transport/transport.h`, `server_core/server_core.{h,cpp}` (monde bidon = un chunk plat codé en dur), `transports/loopback/loopback_transport.*`, `client/main.cpp`, `tools/worldgen_debug/main.cpp`
- **Done** : le client ouvre une fenêtre raylib et affiche un chunk (même trivial) reçu via `transport_poll_reliable` — pas via un pointeur direct vers `server_core`. `tools/worldgen_debug` tourne sans fenêtre et loggue le contenu du chunk généré, preuve que `server_core` ne dépend pas de raylib.
- **Risques** : sur-concevoir l'API de messages dès le premier jour. **Mitigation** : ne définir que le strict nécessaire (`ChunkData`, `PlayerInput`), étendre au fil des jours suivants.
- **Test manuel** : commenter temporairement le passage par la queue et essayer d'appeler `server_core::` depuis `client/` → vérifier que ça ne compile pas (le garde-fou CMake fonctionne).

### Jour 2 — Génération procédurale + streaming de chunks
- **Fichiers** : `common/noise/{perlin,simplex}.*`, `server_core/{world_manager,chunk_generator,biome,cave_gen}.*`, extension des messages (`ChunkUnload`), `client/world/client_world.*`, mesher naïf
- **Done** : terrain avec relief (Perlin), 2-3 biomes (bruit température/humidité), grottes (bruit 3D à seuil), chunks chargés/déchargés en marchant, rayon de vue configurable
- **Risques** : générer trop de chunks dans un seul tick fait ramer le rendu (pas de thread séparé nécessaire ici puisqu'il n'y a plus d'I/O réseau à multiplexer, mais la génération reste coûteuse). **Mitigation** : plafonner à 1-2 chunks générés par frame, file FIFO des chunks en attente.
- **Test manuel** : marcher loin du spawn sans à-coup notable, vérifier au log le déchargement des chunks lointains.

### Jour 3 — Casser/placer + greedy meshing + inventaire
- **Fichiers** : `client/input/voxel_raycast.*` (DDA), extension messages (`BlockUpdate`, `InventoryUpdate`), `server_core/{player_state,inventory}.*`, `client/render/chunk_mesher.*` (passage en greedy), `client/ui/{hud,inventory_ui}.*`
- **Done** : casser/placer un bloc parmi 15-20 types, hotbar sélectionnable, greedy meshing actif (comparer le nombre de triangles avant/après dans les logs)
- **Risques** : coutures/trous du greedy mesher en bordure de chunk. **Mitigation** : remesh utilisant les blocs des chunks voisins pour le culling de face de bord, toggle wireframe de debug.
- **Test manuel** : casser/placer en bordure de chunk sans trou visuel ; changer d'item via molette/clavier.

### Jour 4 — Persistance + santé/faim + cycle jour-nuit
- **Fichiers** : `server_core/{chunk_storage,player_save}.*`, extension messages (`HealthHungerUpdate`), `client/render/sky_renderer.*`
- **Done** : fermer/relancer le client → monde modifié et état joueur (position, inventaire, santé, faim) restaurés depuis le disque ; horloge de jeu qui avance en continu, ciel qui change de couleur/luminosité en conséquence
- **Risques** : premier lancement sans fichier de save. **Mitigation** : génération d'un monde neuf si aucun fichier trouvé, testé explicitement dès ce jour.
- **Test manuel** : casser des blocs, quitter, relancer, vérifier leur absence persistante.

### Jour 5 — Physique de blocs + eau + arbres
- **Fichiers** : `server_core/block_physics.*`, bloc eau (statique) dans le registre de blocs, génération d'arbres dans `chunk_generator`
- **Done** : sable/gravier posé en hauteur tombe et se stabilise ; eau placeable ; arbres générés naturellement pendant la génération de terrain
- **Risques** : cascade de mises à jour de blocs qui bloque un tick si beaucoup de sable tombe d'un coup. **Mitigation** : file "dirty blocks" avec plafond de traitement par tick.
- **Test manuel** : creuser sous une colonne de sable, observer une chute progressive sans freeze.

### Jour 6 — Crafting réel (grille 3×3) + mobs
- **Fichiers** : `server_core/crafting.*` (pattern matching sur grille 3×3), `client/ui/crafting_ui.*`, `server_core/{entity,mob_ai}.*`
- **Done** : grille de crafting ouvrable, reconnaissance de pattern, récupération du résultat ; 2 types de mobs présents avec au minimum marche aléatoire + gravité, idéalement un A* voxel simple si le temps le permet
- **Risques** : A* voxel qui explore un espace non borné. **Mitigation** : recherche limitée à un rayon fixe autour du mob, fallback sur marche aléatoire si aucun chemin trouvé rapidement — et c'est le premier élément coupé en cas de retard.
- **Test manuel** : crafter un objet connu (ex. planches → bâtons), observer un mob se déplacer de façon crédible.

### Jour 7 — Polish, stabilisation, (stretch optionnel) ébauche de `SocketTransport`
- **Objectif principal** : un jeu solo complet et stable, jouable de bout en bout (spawn → explorer → construire → crafter → gérer faim/santé → sauvegarder/recharger), sans crash sur une session de 15-20 minutes.
- **Fichiers** : corrections diverses, `docs/PROTOCOL.md` figé sur la spec des messages telle qu'elle existe réellement (pas telle que planifiée en théorie)
- **Stretch hors scope garanti** : commencer `transports/socket/socket_transport.*` — uniquement si tout le reste est stable. Le but n'est **pas** d'avoir du multi-process fonctionnel en fin de semaine, mais de vérifier que l'interface `Transport` ne nécessite aucun changement dans `client/` ou `server_core/` pour l'accueillir (voir Annexe).
- **Test manuel** : session de jeu solo de 15-20 minutes couvrant toutes les mécaniques, sans crash ni incohérence.

---

## 4. Stratégie de développement et de test

- **`server_core` se teste headless**, sans jamais ouvrir de fenêtre : `tools/worldgen_debug` tick la lib N fois et affiche des stats (chunks générés, temps par tick, blocs modifiés). Utile pour itérer vite sur la génération procédurale et le crafting sans repasser par le rendu à chaque changement.
- **Le garde-fou CMake (§1.2)** remplace le besoin d'un "mode mock" séparé : puisque le client ne peut physiquement pas accéder à `server_core` autrement que par le transport, il n'y a rien à mocker — `LoopbackTransport` *est* déjà l'environnement de test complet.
- **Pas de test multi-process en V1** : inutile tant qu'il n'y a qu'un seul joueur possible. Ce point revient dans l'Annexe, au moment où `SocketTransport` existera.
- **Ordre de priorité si retard** (rappel) : A* voxel → crafting grille (fallback recettes fixes) → eau/physique de blocs → arbres/polish jour-nuit → greedy meshing (fallback naïf) → sauvegarde incrémentale (fallback save-on-exit). Le découpage `server_core`/`transport`/`client` ne se sacrifie jamais, quelle que soit la pression de planning — c'est la seule chose qui rend le futur multijoueur possible sans réécriture.

---

## 5. Annexe — chemin vers le vrai multijoueur (hors scope des 7 jours)

Cette section documente ce qu'il faudra faire **plus tard**, quand `SocketTransport` sera à l'ordre du jour. Rien ici n'est à coder pendant le sprint de 7 jours (sauf ébauche optionnelle jour 7).

### 5.1 Ce qui ne change pas
Les structs `ReliableMessage`/`UnreliableMessage` définies dans `common/messages/` restent la seule API. `server_core` et `client` ne changent pas.

### 5.2 Ce qu'il faudra ajouter
- **Encodage/décodage en octets** : `ByteWriter`/`ByteReader` qui sérialisent chaque variante de `ReliableMessage`/`UnreliableMessage` — travail mécanique une fois les structs stabilisées par 7 jours de jeu solo.
- **En-têtes de paquets** :
  ```cpp
  struct TcpHeader { uint8_t type; uint32_t length; };                       // framing flux
  struct UdpHeader { uint8_t type; uint32_t sessionId; uint32_t sequence; uint32_t tick; };
  ```
- **`SocketTransport` côté client** : implémente le même `TransportVTable` que `LoopbackTransport`, mais `send_reliable` sérialise et envoie sur le socket TCP, `poll_reliable` désérialise ce qui a été reçu ; `tick()` fait un `poll()`/`select()` non bloquant sur les sockets au lieu d'appeler `server_core::tick()` directement (le tick de simulation tourne alors dans le process serveur dédié, pas dans le client).
- **Un exécutable serveur dédié** (`server_app/main.cpp`) : lie `server_core` + une future implémentation serveur du transport socket (accepte les connexions TCP, lit l'UDP, appelle `server_core::connect_client`/`submit_*`/`tick`/`poll_outgoing_*` exactement comme le fait `LoopbackTransport` aujourd'hui côté client — la boucle serveur dédiée ressemble en fait beaucoup à `loopback_transport.cpp`, juste multi-client et piloté par de vrais sockets plutôt qu'un appel direct).
- **Boucle serveur dédiée** : tick fixe 20 Hz, `select()`/`poll()`/`WSAPoll()` sur le socket TCP d'écoute + les sockets clients + le socket UDP, éventuellement un pool de threads dédié à la génération de chunks pour ne pas bloquer le tick (le `server_core` actuel, déjà pensé en tick borné, s'y prête directement).
- **Reconnexion** : `sessionId` attribué au handshake TCP, porté par chaque datagramme UDP pour survivre à un changement de port NAT ; à la reconnexion, renvoi d'une rafale complète de chunks plutôt qu'un diff.
- **Déploiement** : `docker/Dockerfile.server` (déjà présent dans l'arborescence, inutilisé pour l'instant), exposition d'un port TCP + un port UDP, bind sur l'interface WireGuard plutôt que derrière Traefik (qui n'apporte rien pour un protocole binaire custom non-HTTP).

Rien de cette section ne bloque le jeu solo — elle existe pour que la transition, le jour où vous voudrez vraiment la faire, soit un projet d'extension isolé et non une réécriture.

---

## 6. État final (jours 1 à 6 exécutés)

Tout le scope V1 du plan a été implémenté et testé (headless + visuel) :

- **J1** fondations : `common`/`server_core`/`transports/loopback`/`client` découplés, garde-fou CMake vérifié.
- **J2** génération procédurale (Perlin, 3 biomes, grottes 3D) + streaming de chunks multi-viewer.
- **J3** casser/placer (DDA), greedy meshing, inventaire 36 slots + hotbar.
- **J4** persistance disque (chunks + joueur), santé/faim, cycle jour-nuit synchronisé.
- **J5** physique de blocs (sable/gravier en cascade), eau statique, arbres procéduraux.
- **J6** crafting réel en grille 3×3 (2 recettes : bois→planches→bâtons), mobs (marche aléatoire + gravité, pas d'A*).

`tools/worldgen_debug` sert de suite de tests de régression headless (5 tests : persistance, physique, arbres/eau, crafting, mobs — tous verts). Un test de stabilité de 60s+ avec mouvement/clics simulés n'a montré ni crash ni fuite mémoire (127 Mo stable). `docs/PROTOCOL.md` documente le contrat de messages réel, y compris les écarts assumés par rapport à ce plan (voir sa section 5).

Le jour 7 (stretch `SocketTransport`) reste hors scope, conformément à la décision du pivot v2.
