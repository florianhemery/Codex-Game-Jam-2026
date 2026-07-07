#pragma once

// Hot-reload de shaders : surveille les mtimes des fichiers GLSL et recharge
// a la volee. Si la compilation echoue, l'ancien shader reste actif.
// Contrainte : thread principal uniquement (contexte OpenGL requis).

#include "raylib.h"

#include <chrono>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace racer::engine {

// Emplacement stable d'un shader surveille. La reference retournee par
// RegisterShader() reste valide toute la vie du ShaderWatcher.
class ShaderSlot {
public:
    // Shader actif (toujours utilisable, meme apres un echec de reload).
    Shader& Get() { return shader_; }
    const Shader& Get() const { return shader_; }

    const std::string& Name() const { return name_; }
    const std::string& VsPath() const { return vsPath_; }
    const std::string& FsPath() const { return fsPath_; }

    bool IsValid() const { return valid_; }        // dernier (re)chargement OK
    int ReloadCount() const { return reloadCount_; } // nb de hot-reloads reussis

private:
    friend class ShaderWatcher;

    Shader shader_{};
    std::string name_;
    std::string vsPath_; // vide -> etage vertex par defaut raylib
    std::string fsPath_; // vide -> etage fragment par defaut raylib
    std::filesystem::file_time_type vsTime_{};
    std::filesystem::file_time_type fsTime_{};
    bool valid_ = false;
    int reloadCount_ = 0;
};

// Registre + surveillance de shaders. Poll() est a appeler chaque frame.
class ShaderWatcher {
public:
    // Appele apres chaque hot-reload reussi (re-binder les uniforms ici).
    using ReloadCallback = std::function<void(const std::string& name, Shader& shader)>;

    ShaderWatcher() = default;
    ~ShaderWatcher();

    ShaderWatcher(const ShaderWatcher&) = delete;
    ShaderWatcher& operator=(const ShaderWatcher&) = delete;

    // Charge et enregistre un shader. vsPath OU fsPath peuvent etre vides
    // (etage par defaut raylib). Nom deja pris -> renvoie le slot existant.
    ShaderSlot& RegisterShader(const std::string& name,
                               const std::string& vsPath,
                               const std::string& fsPath);

    // Compare les mtimes (std::filesystem::last_write_time) et recharge les
    // shaders modifies. Echec de compilation -> ancien shader conserve + log.
    void Poll();

    ShaderSlot* Find(const std::string& name);

    void SetOnReload(ReloadCallback callback) { onReload_ = std::move(callback); }

    // Intervalle mini entre deux scans disque (defaut 250 ms, 0 = chaque frame).
    void SetPollInterval(double seconds);

    // Decharge tous les shaders (a appeler avant CloseWindow()).
    void UnloadAll();

    int ShaderCount() const { return static_cast<int>(slots_.size()); }

private:
    bool TryReload(ShaderSlot& slot);

    std::unordered_map<std::string, std::unique_ptr<ShaderSlot>> slots_;
    ReloadCallback onReload_;
    std::chrono::steady_clock::duration pollInterval_ = std::chrono::milliseconds(250);
    std::chrono::steady_clock::time_point lastPoll_{};
};

} // namespace racer::engine
