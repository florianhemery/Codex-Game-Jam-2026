/*
** EPITECH PROJECT, 2026
** racer
** File description:
** WorldRenderTimings — read-only, additive instrumentation for per-section
** world draw costs (terrain / props / triggers / traffic), in milliseconds.
** Written narrowly in WorldRenderer.cpp around existing draw call sites
** (no rendering logic changes). Read by HudDebugOverlay for the debug FPS
** overlay. Not used by any gameplay or rendering decision.
*/

#ifndef WORLD_RENDER_TIMINGS_HPP_
#define WORLD_RENDER_TIMINGS_HPP_

namespace racer::world {

struct WorldRenderTimings {
    double terrainMs = 0.0;  // WorldRenderer::drawOpaque() — terrain mesh pass
    double propsMs = 0.0;    // WorldRenderer::drawLit() — landmarks + scatter/props
    double triggersMs = 0.0; // WorldRenderer::drawTriggers() — POI markers
    double trafficMs = 0.0;  // WorldRenderer::drawTraffic() — traffic vehicles
};

inline WorldRenderTimings &worldRenderTimings()
{
    static WorldRenderTimings timings;
    return timings;
}

} // namespace racer::world

#endif /* !WORLD_RENDER_TIMINGS_HPP_ */
