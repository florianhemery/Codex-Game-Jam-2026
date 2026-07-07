/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Declarative render graph: named passes and transient targets
*/

#ifndef RENDER_GRAPH_HPP_
#define RENDER_GRAPH_HPP_

#include "Engine/Rhi/Device.hpp"
#include "Engine/Rhi/RhiTypes.hpp"

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace racer::engine {

class RenderGraph;

class PassContext {
public:
    Device &getDevice() const
    {
        return device_;
    }

    RenderTargetHandle getOutput() const
    {
        return output_;
    }

    RenderTargetHandle readTarget(const std::string &passName) const;

private:
    friend class RenderGraph;
    PassContext(
        Device &device,
        RenderTargetHandle output,
        const std::unordered_map<std::string, RenderTargetHandle>
            &producedTargets)
        : device_(device),
          output_(output),
          producedTargets_(producedTargets)
    {
    }

    Device &device_;
    RenderTargetHandle output_;
    const std::unordered_map<std::string, RenderTargetHandle>
        &producedTargets_;
};

struct PassDesc {
    std::optional<RenderTargetDesc> outputDesc;
    std::function<void(PassContext &)> execute;
};

class RenderGraph {
public:
    explicit RenderGraph(Device &device) : device_(device)
    {
    }

    ~RenderGraph();

    RenderGraph(const RenderGraph &) = delete;
    RenderGraph &operator=(const RenderGraph &) = delete;

    void addPass(std::string name, PassDesc desc);
    void execute();
    void reset();

private:
    struct Pass {
        std::string name;
        PassDesc desc;
    };

    struct PoolBucket {
        std::vector<RenderTargetHandle> targets;
        std::size_t usedThisFrame_ = 0;
    };

    RenderTargetHandle acquireTransientTarget(const RenderTargetDesc &desc);
    bool tryReusePooledTarget(
        PoolBucket &bucket, RenderTargetHandle &out);
    void executePass(const Pass &pass);

    Device &device_;
    std::vector<Pass> passes_;
    std::unordered_map<RenderTargetDesc, PoolBucket, RenderTargetDescHash>
        pool_;
    std::unordered_map<std::string, RenderTargetHandle> producedTargets_;
};

} // namespace racer::engine

#endif /* !RENDER_GRAPH_HPP_ */
