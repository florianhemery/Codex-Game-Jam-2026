/// \file render_graph.h
/// \brief Render graph declaratif : passes nommees executees dans l'ordre,
///        cibles transientes recyclees via un pool keye par descripteur.

#pragma once

#include "engine/rhi/device.h"
#include "engine/rhi/rhi_types.h"

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace racer::engine {

class RenderGraph;

/// Contexte passe au callback d'une passe : acces au device, a la cible de
/// sortie de la passe et aux sorties des passes precedentes (par nom).
class PassContext {
public:
    /// Peripherique graphique (creation de shaders, acces aux textures...).
    Device& GetDevice() const { return device_; }

    /// Cible de sortie de la passe courante (invalide si rendu a l'ecran).
    RenderTargetHandle GetOutput() const { return output_; }

    /// Sortie d'une passe deja executee cette frame, par nom.
    /// \return handle invalide si la passe est inconnue ou rendait a l'ecran.
    RenderTargetHandle ReadTarget(const std::string& passName) const;

private:
    friend class RenderGraph;
    PassContext(Device& device, RenderTargetHandle output,
                const std::unordered_map<std::string, RenderTargetHandle>& producedTargets)
        : device_(device), output_(output), producedTargets_(producedTargets) {}

    Device& device_;
    RenderTargetHandle output_;
    const std::unordered_map<std::string, RenderTargetHandle>& producedTargets_;
};

/// Description d'une passe de rendu.
struct PassDesc {
    /// Cible de sortie ; std::nullopt = dessin direct au backbuffer (ecran).
    std::optional<RenderTargetDesc> outputDesc;

    /// Corps de la passe : commandes de dessin emises entre Begin/End.
    std::function<void(PassContext&)> execute;
};

/// Render graph lineaire redeclare chaque frame :
///   graph.Reset();
///   graph.AddPass("scene", {sceneDesc, ...});
///   graph.AddPass("tonemap", {std::nullopt, ...});  // lit "scene", ecrit l'ecran
///   graph.Execute();
///
/// Les cibles transientes sont recyclees d'une frame a l'autre via un pool
/// keye par descripteur : deux passes demandant le meme descripteur a des
/// frames differentes reutilisent la meme texture GPU. Le graphe ne doit pas
/// survivre au Device qui lui est confie.
class RenderGraph {
public:
    explicit RenderGraph(Device& device) : device_(device) {}

    /// Rend au Device toutes les cibles du pool.
    ~RenderGraph();

    RenderGraph(const RenderGraph&) = delete;
    RenderGraph& operator=(const RenderGraph&) = delete;

    /// Declare une passe, executee dans l'ordre d'ajout. Les noms doivent etre
    /// uniques au sein d'une frame (sinon ReadTarget resout le dernier ajout).
    void AddPass(std::string name, PassDesc desc);

    /// Execute les passes : ouvre/ferme les cibles, fournit le PassContext.
    void Execute();

    /// Vide les passes declarees ; le pool de cibles transientes persiste.
    void Reset();

private:
    struct Pass {
        std::string name;
        PassDesc desc;
    };

    /// Cibles partageant un meme descripteur ; usedThisFrame remonte a zero a
    /// chaque Execute pour recycler les textures des frames precedentes.
    struct PoolBucket {
        std::vector<RenderTargetHandle> targets;
        std::size_t usedThisFrame = 0;
    };

    /// Reserve une cible libre du pool (ou en cree une) pour cette frame.
    RenderTargetHandle AcquireTransientTarget(const RenderTargetDesc& desc);

    Device& device_;
    std::vector<Pass> passes_;
    std::unordered_map<RenderTargetDesc, PoolBucket, RenderTargetDescHash> pool_;
    std::unordered_map<std::string, RenderTargetHandle> producedTargets_;
};

} // namespace racer::engine
