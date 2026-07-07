/// \file device.h
/// \brief Peripherique graphique : abstraction de creation/destruction des
///        ressources GPU au-dessus de raylib/rlgl.

#pragma once

#include "raylib.h"

#include "engine/rhi/rhi_types.h"

#include <cstdint>
#include <unordered_map>

namespace racer::engine {

/// Peripherique graphique du moteur : cree et detruit les ressources GPU
/// (render targets, shaders) et pilote les changements de cible de rendu.
///
/// Backend actuel : raylib/rlgl compile en OpenGL 3.3. L'API publique est
/// pensee pour etre agnostique du backend : seuls les accesseurs
/// GetRenderTexture() / GetShader() exposent des types raylib (pont necessaire
/// tant que le dessin passe par raylib). Un backend GL 4.6 ou Vulkan pourra
/// remplacer l'implementation sans changer les signatures de creation,
/// destruction et Begin/End ; seuls ces deux accesseurs seront re-exprimes.
///
/// Toutes les methodes exigent un contexte GL valide (apres InitWindow) et
/// l'objet doit etre detruit avant CloseWindow.
class Device {
public:
    Device() = default;

    /// Filet de securite : libere toutes les ressources encore enregistrees.
    ~Device();

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    // --- Render targets ----------------------------------------------------

    /// Cree une cible de rendu (framebuffer + texture couleur et/ou profondeur).
    /// \return handle invalide en cas d'echec.
    RenderTargetHandle CreateRenderTarget(const RenderTargetDesc& desc);

    /// Detruit la cible et libere les ressources GPU. Ignore un handle inconnu.
    void DestroyRenderTarget(RenderTargetHandle handle);

    /// Acces au RenderTexture2D raylib sous-jacent (nullptr si handle inconnu).
    /// Pour une cible DEPTH24, la profondeur echantillonnable est dans .depth.
    const RenderTexture2D* GetRenderTexture(RenderTargetHandle handle) const;

    /// Descripteur ayant servi a creer la cible (nullptr si handle inconnu).
    const RenderTargetDesc* GetRenderTargetDesc(RenderTargetHandle handle) const;

    /// Ouvre la cible comme destination de dessin (equivalent BeginTextureMode).
    void BeginRenderTarget(RenderTargetHandle handle);

    /// Referme la cible courante et revient au backbuffer.
    void EndRenderTarget();

    // --- Shaders -------------------------------------------------------------

    /// Compile un programme depuis des sources GLSL en memoire.
    /// nullptr = shader raylib par defaut pour l'etage correspondant. En cas
    /// d'echec de compilation, raylib retombe sur son shader par defaut et
    /// trace l'erreur dans le log (le handle retourne reste utilisable).
    ShaderRhiHandle CreateShaderFromMemory(const char* vsSource, const char* fsSource);

    /// Detruit le programme shader. Ignore un handle inconnu.
    void DestroyShader(ShaderRhiHandle handle);

    /// Acces au Shader raylib sous-jacent (nullptr si handle inconnu).
    const Shader* GetShader(ShaderRhiHandle handle) const;

private:
    struct RenderTargetEntry {
        RenderTexture2D target{};
        RenderTargetDesc desc{};
    };

    // Registres indexes par l'id du handle ; 0 reste reserve (handle invalide).
    std::unordered_map<std::uint32_t, RenderTargetEntry> renderTargets_;
    std::unordered_map<std::uint32_t, Shader> shaders_;
    std::uint32_t nextRenderTargetId_ = 1;
    std::uint32_t nextShaderId_ = 1;
};

} // namespace racer::engine
