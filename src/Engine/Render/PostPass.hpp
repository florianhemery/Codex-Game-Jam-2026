/*
** EPITECH PROJECT, 2026
** racer
** File description:
** post-process render pass
*/

#ifndef POST_PASS_HPP_
#define POST_PASS_HPP_

#include "Engine/Render/RenderPipeline.hpp"
#include "Engine/Render/ShaderLocations.hpp"

namespace racer::engine {

class PostPass {
public:
    static void run(Device &device, RenderTargetHandle sceneRT, Shader postShader,
                    const AmbianceParams &params, const PostLocs &postLocs,
                    float time, const RenderPipeline::PostParams &postParams);
};

} // namespace racer::engine

#endif /* !POST_PASS_HPP_ */
