# engine_assets — pipeline d'assets

Lib statique `engine_assets` (namespace `racer::engine`). Deux briques :

- **`asset_registry.h`** : cache centralisé de modèles (GLTF/GLB/OBJ… via
  `LoadModel` raylib 5.5) et de textures, avec refcount coopératif.
- **`shader_watcher.h`** : hot-reload de shaders GLSL par surveillance des
  mtimes (`std::filesystem::last_write_time`).

Aucune dépendance hors raylib. À utiliser depuis le thread principal
uniquement (contexte OpenGL requis).

## API

```cpp
racer::engine::AssetRegistry registry;

// Acquire() implicite ; asset manquant -> warning + cube magenta.
auto& car = registry.LoadModelAsset("assets/models/car.glb");
DrawModel(car.Get(), pos, 1.0f, WHITE);
car.PbrInfos()[0].albedoColor;   // infos PBR par matériau (metallic, roughness…)

// Mipmaps + filtrage trilinéaire automatiques ; manquante -> damier magenta.
auto& tex = registry.LoadTextureAsset("assets/textures/asphalt.png");

car.Release();                   // décrémente le refcount
registry.UnloadUnused();         // décharge tout ce qui est à refcount 0
registry.UnloadAll();            // avant CloseWindow()
```

```cpp
racer::engine::ShaderWatcher watcher;

// vsPath OU fsPath peuvent être vides (étage par défaut raylib).
auto& slot = watcher.RegisterShader("basic", "assets/shaders/basic.vs",
                                             "assets/shaders/basic.fs");
watcher.SetOnReload([](const std::string& name, Shader& sh) {
    // re-binder les uniforms ici après chaque hot-reload réussi
});

// Chaque frame : recharge si un fichier a changé. Si la compilation échoue,
// l'ancien shader reste actif et l'erreur GLSL est loguée.
watcher.Poll();
BeginShaderMode(slot.Get());
```

## Conventions de chemins

Chemins **relatifs au répertoire de travail** (lancer l'exécutable depuis la
racine du dépôt), séparateur `/` :

- `assets/shaders/` — shaders GLSL 330 (`basic.vs` / `basic.fs` fournis,
  rendu par défaut raylib + teinte `tintColor` optionnelle).
- `assets/models/`, `assets/textures/` — à créer au besoin.

Les clés de cache sont les chemins normalisés (`a//b`, `./a`, `a/../b`
pointent vers la même entrée).

## Plan futur

- Compression GPU : textures KTX2/BC7 pré-transcodées (raylib charge DDS/KTX),
  pipeline d'import offline.
- Streaming asynchrone : décodage disque -> RAM sur thread worker, upload GPU
  sur le thread principal, budgets mémoire + éviction LRU.
- Shaders : `#include` maison, permutations (defines), cache binaire.
