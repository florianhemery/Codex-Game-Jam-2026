/// \file render_graph.cpp
/// \brief Implementation du render graph lineaire et du pool de cibles.

#include "engine/rhi/render_graph.h"

#include "raylib.h"

#include <utility>

namespace racer::engine {

RenderTargetHandle PassContext::ReadTarget(const std::string& passName) const {
    const auto it = producedTargets_.find(passName);
    return (it != producedTargets_.end()) ? it->second : RenderTargetHandle{};
}

RenderGraph::~RenderGraph() {
    for (const auto& [desc, bucket] : pool_) {
        for (const RenderTargetHandle handle : bucket.targets) {
            device_.DestroyRenderTarget(handle);
        }
    }
}

void RenderGraph::AddPass(std::string name, PassDesc desc) {
    passes_.push_back(Pass{std::move(name), std::move(desc)});
}

RenderTargetHandle RenderGraph::AcquireTransientTarget(const RenderTargetDesc& desc) {
    PoolBucket& bucket = pool_[desc];

    // Reutilise une cible d'une frame precedente si toutes celles du bucket
    // n'ont pas deja ete reservees cette frame.
    while (bucket.usedThisFrame < bucket.targets.size()) {
        const RenderTargetHandle handle = bucket.targets[bucket.usedThisFrame];
        if (device_.GetRenderTexture(handle) != nullptr) {
            ++bucket.usedThisFrame;
            return handle;
        }
        // Cible detruite exterieurement : on la purge du pool.
        bucket.targets.erase(bucket.targets.begin() +
                             static_cast<std::ptrdiff_t>(bucket.usedThisFrame));
    }

    const RenderTargetHandle handle = device_.CreateRenderTarget(desc);
    if (handle.IsValid()) {
        bucket.targets.push_back(handle);
        ++bucket.usedThisFrame;
    }
    return handle;
}

void RenderGraph::Execute() {
    producedTargets_.clear();

    // Nouvelle frame : toutes les cibles du pool redeviennent recyclables.
    for (auto& [desc, bucket] : pool_) {
        bucket.usedThisFrame = 0;
    }

    for (const Pass& pass : passes_) {
        RenderTargetHandle output{};

        if (pass.desc.outputDesc.has_value()) {
            output = AcquireTransientTarget(*pass.desc.outputDesc);
            if (!output.IsValid()) {
                TraceLog(LOG_WARNING, "RHI: passe '%s' ignoree (cible indisponible)", pass.name.c_str());
                continue;
            }
            producedTargets_[pass.name] = output;
        }

        PassContext context(device_, output, producedTargets_);

        if (output.IsValid()) device_.BeginRenderTarget(output);
        if (pass.desc.execute) pass.desc.execute(context);
        if (output.IsValid()) device_.EndRenderTarget();
    }
}

void RenderGraph::Reset() {
    passes_.clear();
    producedTargets_.clear();
}

} // namespace racer::engine
