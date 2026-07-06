#pragma once

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
    Model trackModel_{};
    Model groundModel_{};
};

} // namespace racer
