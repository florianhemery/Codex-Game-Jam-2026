/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Linear render graph and transient target pool
*/

#include "engine/rhi/render_graph.h"

#include "raylib.h"

#include <utility>

namespace racer::engine {

RenderTargetHandle PassContext::ReadTarget(const std::string &passName) const
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
            device_.DestroyRenderTarget(handle);
    }
}

void RenderGraph::AddPass(std::string name, PassDesc desc)
{
    passes_.push_back(Pass{std::move(name), std::move(desc)});
}

bool RenderGraph::tryReusePooledTarget(
    PoolBucket &bucket, RenderTargetHandle &out)
{
    while (bucket.usedThisFrame < bucket.targets.size()) {
        const RenderTargetHandle handle = bucket.targets[bucket.usedThisFrame];

        if (device_.GetRenderTexture(handle) != nullptr) {
            ++bucket.usedThisFrame;
            out = handle;
            return true;
        }
        bucket.targets.erase(
            bucket.targets.begin() +
            static_cast<std::ptrdiff_t>(bucket.usedThisFrame));
    }
    return false;
}

RenderTargetHandle RenderGraph::AcquireTransientTarget(
    const RenderTargetDesc &desc)
{
    PoolBucket &bucket = pool_[desc];
    RenderTargetHandle handle{};

    if (tryReusePooledTarget(bucket, handle))
        return handle;
    handle = device_.CreateRenderTarget(desc);
    if (handle.IsValid()) {
        bucket.targets.push_back(handle);
        ++bucket.usedThisFrame;
    }
    return handle;
}

void RenderGraph::executePass(const Pass &pass)
{
    RenderTargetHandle output{};

    if (pass.desc.outputDesc.has_value()) {
        output = AcquireTransientTarget(*pass.desc.outputDesc);
        if (!output.IsValid()) {
            TraceLog(
                LOG_WARNING,
                "RHI: passe '%s' ignoree (cible indisponible)",
                pass.name.c_str());
            return;
        }
        producedTargets_[pass.name] = output;
    }

    PassContext context(device_, output, producedTargets_);

    if (output.IsValid())
        device_.BeginRenderTarget(output);
    if (pass.desc.execute)
        pass.desc.execute(context);
    if (output.IsValid())
        device_.EndRenderTarget();
}

void RenderGraph::Execute()
{
    producedTargets_.clear();
    for (auto &[desc, bucket] : pool_)
        bucket.usedThisFrame = 0;
    for (const Pass &pass : passes_)
        executePass(pass);
}

void RenderGraph::Reset()
{
    passes_.clear();
    producedTargets_.clear();
}

} // namespace racer::engine
