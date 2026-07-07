/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Linear render graph and transient target pool
*/

#include "Engine/Rhi/RenderGraph.hpp"

#include "raylib.h"

#include <utility>

namespace racer::engine {

RenderTargetHandle PassContext::readTarget(const std::string &passName) const
{
    const auto it = producedTargets_.find(passName);

    if (it == producedTargets_.end())
        return RenderTargetHandle{};
    return it->second;
}

RenderGraph::~RenderGraph()
{
    for (const auto &[desc, bucket] : pool_) {
        for (const RenderTargetHandle handle : bucket.targets)
            device_.destroyRenderTarget(handle);
    }
}

void RenderGraph::addPass(std::string name, PassDesc desc)
{
    passes_.push_back(Pass{std::move(name), std::move(desc)});
}

bool RenderGraph::tryReusePooledTarget(
    PoolBucket &bucket, RenderTargetHandle &out)
{
    while (bucket.usedThisFrame_ < bucket.targets.size()) {
        const RenderTargetHandle handle = bucket.targets[bucket.usedThisFrame_];

        if (device_.getRenderTexture(handle) != nullptr) {
            ++bucket.usedThisFrame_;
            out = handle;
            return true;
        }
        bucket.targets.erase(
            bucket.targets.begin() +
            static_cast<std::ptrdiff_t>(bucket.usedThisFrame_));
    }
    return false;
}

RenderTargetHandle RenderGraph::acquireTransientTarget(
    const RenderTargetDesc &desc)
{
    PoolBucket &bucket = pool_[desc];
    RenderTargetHandle handle{};

    if (tryReusePooledTarget(bucket, handle))
        return handle;
    handle = device_.createRenderTarget(desc);
    if (handle.isValid()) {
        bucket.targets.push_back(handle);
        ++bucket.usedThisFrame_;
    }
    return handle;
}

void RenderGraph::executePass(const Pass &pass)
{
    RenderTargetHandle output{};

    if (pass.desc.outputDesc.has_value()) {
        output = acquireTransientTarget(*pass.desc.outputDesc);
        if (!output.isValid()) {
            TraceLog(
                LOG_WARNING,
                "RHI: passe '%s' ignoree (cible indisponible)",
                pass.name.c_str());
            return;
        }
        producedTargets_[pass.name] = output;
    }

    PassContext context(device_, output, producedTargets_);

    if (output.isValid())
        device_.beginRenderTarget(output);
    if (pass.desc.execute)
        pass.desc.execute(context);
    if (output.isValid())
        device_.endRenderTarget();
}

void RenderGraph::execute()
{
    producedTargets_.clear();
    for (auto &[desc, bucket] : pool_)
        bucket.usedThisFrame_ = 0;
    for (const Pass &pass : passes_)
        executePass(pass);
}

void RenderGraph::reset()
{
    passes_.clear();
    producedTargets_.clear();
}

} // namespace racer::engine
