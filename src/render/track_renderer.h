#pragma once

#include <vector>

#include "raylib.h"
#include "track/track.h"

namespace racer {

class TrackRenderer {
public:
    explicit TrackRenderer(const Track& track);
    ~TrackRenderer();
    TrackRenderer(const TrackRenderer&) = delete;
    TrackRenderer& operator=(const TrackRenderer&) = delete;

    void Draw() const;

private:
    struct PropInstance {
        Vector3 position;
        float heightScale;
        int type; // 0 = arbre, 1 = immeuble
        Color color;
    };

    Model trackModel_{};
    Model curbModelOuter_{};
    Model curbModelInner_{};
    Model groundModel_{};
    Model finishLineModel_{};
    std::vector<PropInstance> props_;
};

} // namespace racer
