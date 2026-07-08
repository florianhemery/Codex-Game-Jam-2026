/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Static Aurelia world data — POIs, missions, roads
*/

#ifndef AURELIA_DATA_HPP_
#define AURELIA_DATA_HPP_

#include <vector>

#include "Track/Track.hpp"
#include "World/Aurelia/AureliaTypes.hpp"
#include "World/Road/RoadGraph.hpp"

namespace racer::world {

class AureliaData {
public:
    static const std::vector<PoiInstance> &worldPois();
    static const std::vector<MissionDef> &missions();
    static const RoadGraph &roadGraph();
    static void attachRaceLabels(const std::vector<TrackDef> &presets);
    static void initCollectibles();

    // Encyclopedia content for the 20 "plaques des Veilleurs" lore
    // fragments (see PoiInstance::loreIndex in initCollectibles()).
    // index must be in [0, 19]; out-of-range returns a safe placeholder.
    static const char *loreTitle(int index);
    static const char *loreText(int index);
    static constexpr int kLoreEntryCount = 20;
};

} // namespace racer::world

#endif /* !AURELIA_DATA_HPP_ */
