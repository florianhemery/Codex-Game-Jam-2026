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
    for (const auto &[desc, bucket] : transientTargetPool_) {
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
    while (bucket.borrowedThisFrame < bucket.targets.size()) {
        const RenderTargetHandle handle =
            bucket.targets[bucket.borrowedThisFrame];

        if (device_.getRenderTexture(handle) != nullptr) {
            ++bucket.borrowedThisFrame;
            out = handle;
            return true;
        }
        bucket.targets.erase(
            bucket.targets.begin() +
            static_cast<std::ptrdiff_t>(bucket.borrowedThisFrame));
    }
    return false;
}

RenderTargetHandle RenderGraph::acquireTransientTarget(
    const RenderTargetDesc &desc)
{
    PoolBucket &bucket = transientTargetPool_[desc];
    RenderTargetHandle handle{};

    if (tryReusePooledTarget(bucket, handle))
        return handle;
    handle = device_.createRenderTarget(desc);
    if (handle.isValid()) {
        bucket.targets.push_back(handle);
        ++bucket.borrowedThisFrame;
    }
    return handle;
}

RenderTargetHandle RenderGraph::resolvePassOutput(const Pass &pass)
{
    if (!pass.desc.outputDesc.has_value())
        return RenderTargetHandle{};
    const RenderTargetHandle output =
        acquireTransientTarget(*pass.desc.outputDesc);
    if (!output.isValid()) {
        TraceLog(
            LOG_WARNING,
            "RHI: passe '%s' ignoree (cible indisponible)",
            pass.name.c_str());
        return RenderTargetHandle{};
    }
    producedTargets_[pass.name] = output;
    return output;
}

void RenderGraph::runPassCallback(
    const Pass &pass, RenderTargetHandle output)
{
    PassContext context(device_, output, producedTargets_);
    if (output.isValid())
        device_.beginRenderTarget(output);
    if (pass.desc.execute)
        pass.desc.execute(context);
    if (output.isValid())
        device_.endRenderTarget();
}

void RenderGraph::executePass(const Pass &pass)
{
    const RenderTargetHandle output = resolvePassOutput(pass);
    if (pass.desc.outputDesc.has_value() && !output.isValid())
        return;
    runPassCallback(pass, output);
}

void RenderGraph::execute()
{
    producedTargets_.clear();
    for (auto &[desc, bucket] : transientTargetPool_)
        bucket.borrowedThisFrame = 0;
    for (const Pass &pass : passes_)
        executePass(pass);
}

void RenderGraph::reset()
{
    passes_.clear();
    producedTargets_.clear();
}

} // namespace racer::engine
