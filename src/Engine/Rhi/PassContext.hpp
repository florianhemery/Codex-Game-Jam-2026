/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Render graph pass context and pass descriptor
*/

#ifndef PASS_CONTEXT_HPP_
#define PASS_CONTEXT_HPP_

#include "Engine/Rhi/Device.hpp"
#include "Engine/Rhi/RhiTypes.hpp"

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>

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

} // namespace racer::engine

#endif /* !PASS_CONTEXT_HPP_ */
