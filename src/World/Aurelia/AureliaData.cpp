/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Aurelia static content tables
*/

#include "World/Aurelia/AureliaData.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace racer::world {

namespace {

std::vector<PoiInstance> g_pois = {
    {"marina_race", PoiType::RACE_ENTRY, "Anneau Vitesse", 32.0f, -24.0f, 14.0f,
        0, -1, -1, RegionId::MARINA, Color{255, 210, 80, 255}},
    {"forest_race", PoiType::RACE_ENTRY, "Circuit Sinueux", -96.0f, -160.0f,
        14.0f, 1, -1, -1, RegionId::FOREST, Color{120, 220, 140, 255}},
    {"port_race", PoiType::RACE_ENTRY, "Circuit Technique", 176.0f, -48.0f,
        14.0f, 2, -1, -1, RegionId::PORT, Color{255, 120, 90, 255}},
    {"volcano_race", PoiType::RACE_ENTRY, "Route Abimee", 48.0f, 176.0f, 14.0f,
        3, -1, -1, RegionId::VOLCANO, Color{255, 160, 60, 255}},
    {"cendres_race", PoiType::RACE_ENTRY, "Circuit des Cendres", 200.0f, 200.0f,
        14.0f, 4, -1, -1, RegionId::VOLCANO, Color{255, 90, 40, 255}},
    {"garage_marina", PoiType::GARAGE, "Garage Marina", 8.0f, 8.0f, 10.0f, -1,
        -1, -1, RegionId::MARINA, Color{180, 200, 255, 255}},
    {"garage_forest", PoiType::GARAGE, "Garage Foret", -80.0f, -120.0f, 10.0f,
        -1, -1, -1, RegionId::FOREST, Color{140, 200, 140, 255}},
    {"garage_port", PoiType::GARAGE, "Garage Usine", 160.0f, -16.0f, 10.0f, -1,
        -1, -1, RegionId::PORT, Color{220, 140, 120, 255}},
    {"garage_volcano", PoiType::GARAGE, "Garage Observatoire", 96.0f, 144.0f,
        10.0f, -1, -1, -1, RegionId::VOLCANO, Color{255, 180, 100, 255}},
    {"mission_marina", PoiType::MISSION_GIVER, "Convois cote", 64.0f, 32.0f,
        12.0f, -1, 0, -1, RegionId::MARINA, Color{255, 255, 120, 255}},
    {"mission_forest", PoiType::MISSION_GIVER, "Chrono brume", -64.0f, -96.0f,
        12.0f, -1, 1, -1, RegionId::FOREST, Color{200, 255, 200, 255}},
    {"mission_port", PoiType::MISSION_GIVER, "Livraison docks", 192.0f, -80.0f,
        12.0f, -1, 2, -1, RegionId::PORT, Color{255, 200, 160, 255}},
    {"mission_volcano", PoiType::MISSION_GIVER, "Remontee caldeira", 128.0f,
        192.0f, 12.0f, -1, 3, -1, RegionId::VOLCANO, Color{255, 160, 100, 255}},
};

std::vector<MissionDef> g_missions = {
    {"m_convoy_coast", "Convois cote", MissionKind::CONVOY, RegionId::MARINA,
        95.0f, 12, BiomeId::COAST},
    {"m_ghost_forest", "Fantome brumeux", MissionKind::GHOST_CHASE,
        RegionId::FOREST, 110.0f, 15, BiomeId::FOREST},
    {"m_delivery_port", "Livraison Usine", MissionKind::DELIVERY, RegionId::PORT,
        130.0f, 15, BiomeId::PORT},
    {"m_fog_escape", "Evasion brouillard", MissionKind::FOG_ESCAPE,
        RegionId::FOREST, 100.0f, 18, BiomeId::FOREST},
    {"m_caldera", "Remontee caldeira", MissionKind::CALDERA_CLIMB,
        RegionId::VOLCANO, 140.0f, 20, BiomeId::VOLCANO},
    {"m_time_marina", "Chrono Marina", MissionKind::TIME_TRIAL,
        RegionId::MARINA, 85.0f, 10, BiomeId::COAST},
    {"m_convoy_forest", "Convois forestier", MissionKind::CONVOY,
        RegionId::FOREST, 120.0f, 12, BiomeId::FOREST},
    {"m_delivery_coast", "Pieces littoral", MissionKind::DELIVERY,
        RegionId::MARINA, 100.0f, 10, BiomeId::COAST},
    {"m_ghost_port", "Ghost industriel", MissionKind::GHOST_CHASE,
        RegionId::PORT, 125.0f, 14, BiomeId::PORT},
    {"m_fog_north", "Brume nord", MissionKind::FOG_ESCAPE, RegionId::FOREST,
        105.0f, 16, BiomeId::FOREST},
    {"m_caldera2", "Cendres rapides", MissionKind::CALDERA_CLIMB,
        RegionId::VOLCANO, 135.0f, 18, BiomeId::VOLCANO},
    {"m_time_port", "Chrono docks", MissionKind::TIME_TRIAL, RegionId::PORT,
        115.0f, 12, BiomeId::PORT},
};

RoadGraph g_roads{};
std::vector<std::string> g_raceLabelStorage;

void initRoads()
{
    if (!g_roads.empty()) {
        return;
    }
    // Marina hub + branches vers chaque circuit
    int marina = g_roads.addNode({16.0f, 8.0f});
    int marinaRace = g_roads.addNode({32.0f, -24.0f});
    int forestJct = g_roads.addNode({-40.0f, -48.0f});
    int forestGarage = g_roads.addNode({-80.0f, -120.0f});
    int forestRace = g_roads.addNode({-96.0f, -160.0f});
    int portJct = g_roads.addNode({112.0f, -32.0f});
    int portGarage = g_roads.addNode({160.0f, -16.0f});
    int portRace = g_roads.addNode({176.0f, -48.0f});
    int volcanoJct = g_roads.addNode({56.0f, 80.0f});
    int volcanoGarage = g_roads.addNode({96.0f, 144.0f});
    int volcanoRace = g_roads.addNode({48.0f, 176.0f});
    int cendresRace = g_roads.addNode({200.0f, 200.0f});

    g_roads.addEdge(marina, marinaRace, 40.0f);
    g_roads.addEdge(marina, forestJct, 45.0f);
    g_roads.addEdge(forestJct, forestGarage, 40.0f);
    g_roads.addEdge(forestGarage, forestRace, 35.0f);
    g_roads.addEdge(marina, portJct, 45.0f);
    g_roads.addEdge(portJct, portGarage, 40.0f);
    g_roads.addEdge(portGarage, portRace, 35.0f);
    g_roads.addEdge(marina, volcanoJct, 40.0f);
    g_roads.addEdge(volcanoJct, volcanoGarage, 45.0f);
    g_roads.addEdge(volcanoGarage, volcanoRace, 40.0f);
    g_roads.addEdge(volcanoGarage, cendresRace, 45.0f);
}

} // namespace

const std::vector<PoiInstance> &AureliaData::worldPois()
{
    return g_pois;
}

const std::vector<MissionDef> &AureliaData::missions()
{
    return g_missions;
}

const RoadGraph &AureliaData::roadGraph()
{
    initRoads();
    return g_roads;
}

void AureliaData::attachRaceLabels(const std::vector<TrackDef> &presets)
{
    g_raceLabelStorage.clear();

    for (PoiInstance &poi : g_pois) {
        if (poi.type != PoiType::RACE_ENTRY || poi.trackIndex < 0) {
            continue;
        }
        if (poi.trackIndex < static_cast<int>(presets.size())) {
            g_raceLabelStorage.emplace_back(
                presets[static_cast<size_t>(poi.trackIndex)].name);
        } else {
            g_raceLabelStorage.emplace_back("Circuit");
        }
    }

    size_t labelIdx = 0;
    for (PoiInstance &poi : g_pois) {
        if (poi.type != PoiType::RACE_ENTRY || poi.trackIndex < 0) {
            continue;
        }
        if (labelIdx < g_raceLabelStorage.size()) {
            poi.label = g_raceLabelStorage[labelIdx].c_str();
            ++labelIdx;
        }
    }
}

void AureliaData::initCollectibles()
{
    static bool done = false;
    if (done) {
        return;
    }
    done = true;
    for (int i = 0; i < 20; ++i) {
        float angle = static_cast<float>(i) / 20.0f * 6.28318f;
        float radius = 80.0f + static_cast<float>(i % 5) * 24.0f;
        PoiInstance lore{};
        lore.id = "lore";
        lore.type = PoiType::COLLECTIBLE;
        lore.label = "Fragment Veilleur";
        lore.worldX = std::cos(angle) * radius;
        lore.worldZ = std::sin(angle) * radius;
        lore.radius = 6.0f;
        lore.loreIndex = i;
        lore.color = Color{255, 220, 120, 200};
        if (lore.worldZ > 120.0f) {
            lore.region = RegionId::VOLCANO;
        } else if (lore.worldX > 100.0f) {
            lore.region = RegionId::PORT;
        } else if (lore.worldZ < -60.0f) {
            lore.region = RegionId::FOREST;
        }
        g_pois.push_back(lore);
    }
}

} // namespace racer::world
