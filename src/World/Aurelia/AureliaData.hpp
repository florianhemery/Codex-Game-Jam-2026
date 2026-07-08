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
};

} // namespace racer::world

#endif /* !AURELIA_DATA_HPP_ */
