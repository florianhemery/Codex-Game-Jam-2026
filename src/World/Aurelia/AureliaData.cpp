/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Aurelia static content tables
*/

#include "World/Aurelia/AureliaData.hpp"

#include <algorithm>
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
    {"mission_marina", PoiType::MISSION_GIVER, "Convoi du Bord de Mer", 64.0f,
        32.0f, 12.0f, -1, 0, -1, RegionId::MARINA, Color{255, 255, 120, 255}},
    {"mission_forest", PoiType::MISSION_GIVER, "Le Fantome de la Brume",
        -64.0f, -96.0f, 12.0f, -1, 1, -1, RegionId::FOREST,
        Color{200, 255, 200, 255}},
    {"mission_port", PoiType::MISSION_GIVER, "Livraison Usine Rouge", 192.0f,
        -80.0f, 12.0f, -1, 2, -1, RegionId::PORT, Color{255, 200, 160, 255}},
    {"mission_volcano", PoiType::MISSION_GIVER, "Remontee de la Caldeira",
        128.0f, 192.0f, 12.0f, -1, 4, -1, RegionId::VOLCANO,
        Color{255, 160, 100, 255}},
    {"mission_marina_time", PoiType::MISSION_GIVER, "Chrono de l'Anneau",
        40.0f, 24.0f, 12.0f, -1, 5, -1, RegionId::MARINA,
        Color{255, 255, 120, 255}},
    {"mission_marina_delivery", PoiType::MISSION_GIVER, "Pieces du Littoral",
        0.0f, -8.0f, 12.0f, -1, 7, -1, RegionId::MARINA,
        Color{255, 255, 120, 255}},
    {"mission_forest_fog", PoiType::MISSION_GIVER, "Evasion sous la Canopee",
        -20.0f, -100.0f, 12.0f, -1, 3, -1, RegionId::FOREST,
        Color{200, 255, 200, 255}},
    {"mission_forest_convoy", PoiType::MISSION_GIVER, "Convoi Forestier",
        -48.0f, -140.0f, 12.0f, -1, 6, -1, RegionId::FOREST,
        Color{200, 255, 200, 255}},
    {"mission_forest_north", PoiType::MISSION_GIVER, "Brume du Nord", -100.0f,
        -60.0f, 12.0f, -1, 9, -1, RegionId::FOREST, Color{200, 255, 200, 255}},
    {"mission_port_ghost", PoiType::MISSION_GIVER, "Fantome des Docks", 140.0f,
        -60.0f, 12.0f, -1, 8, -1, RegionId::PORT, Color{255, 200, 160, 255}},
    {"mission_port_time", PoiType::MISSION_GIVER, "Chrono des Quais", 200.0f,
        -16.0f, 12.0f, -1, 11, -1, RegionId::PORT, Color{255, 200, 160, 255}},
    {"mission_volcano_cendres", PoiType::MISSION_GIVER, "Cendres Rapides",
        160.0f, 160.0f, 12.0f, -1, 10, -1, RegionId::VOLCANO,
        Color{255, 160, 100, 255}},
};

std::vector<MissionDef> g_missions = {
    {"m_convoy_coast", "Convoi du Bord de Mer",
        "Escortez le camion de pieces le long de la corniche sans le semer.",
        MissionKind::CONVOY, RegionId::MARINA, 95.0f, 12, BiomeId::COAST},
    {"m_ghost_forest", "Le Fantome de la Brume",
        "Rattrapez le chrono fantome avant qu'il ne disparaisse dans le "
        "brouillard.",
        MissionKind::GHOST_CHASE, RegionId::FOREST, 110.0f, 15,
        BiomeId::FOREST},
    {"m_delivery_port", "Livraison Usine Rouge",
        "Amenez les pieces detachees a l'atelier avant la fermeture des "
        "docks.",
        MissionKind::DELIVERY, RegionId::PORT, 130.0f, 15, BiomeId::PORT},
    {"m_fog_escape", "Evasion sous la Canopee",
        "Filez vers le sud avant que la brume n'avale la route.",
        MissionKind::FOG_ESCAPE, RegionId::FOREST, 100.0f, 18,
        BiomeId::FOREST},
    {"m_caldera", "Remontee de la Caldeira",
        "Grimpez jusqu'a l'observatoire avant que les cendres ne "
        "retombent.",
        MissionKind::CALDERA_CLIMB, RegionId::VOLCANO, 140.0f, 20,
        BiomeId::VOLCANO},
    {"m_time_marina", "Chrono de l'Anneau",
        "Un tour rapide sur l'Anneau Vitesse, comme au bon vieux temps des "
        "essais.",
        MissionKind::TIME_TRIAL, RegionId::MARINA, 85.0f, 10, BiomeId::COAST},
    {"m_convoy_forest", "Convoi Forestier",
        "Guidez le fourgon de pieces a travers les serpentins boises.",
        MissionKind::CONVOY, RegionId::FOREST, 120.0f, 12, BiomeId::FOREST},
    {"m_delivery_coast", "Pieces du Littoral",
        "Livrez le kit de restauration au garage avant la maree haute.",
        MissionKind::DELIVERY, RegionId::MARINA, 100.0f, 10, BiomeId::COAST},
    {"m_ghost_port", "Fantome des Docks",
        "Un ancien pilote d'usine a laisse son chrono. A vous de l'effacer.",
        MissionKind::GHOST_CHASE, RegionId::PORT, 125.0f, 14, BiomeId::PORT},
    {"m_fog_north", "Brume du Nord",
        "Ressortez de la foret avant que le brouillard ne coupe la "
        "visibilite.",
        MissionKind::FOG_ESCAPE, RegionId::FOREST, 105.0f, 16,
        BiomeId::FOREST},
    {"m_caldera2", "Cendres Rapides",
        "Filez vers le Circuit des Cendres avant la prochaine coulee.",
        MissionKind::CALDERA_CLIMB, RegionId::VOLCANO, 135.0f, 18,
        BiomeId::VOLCANO},
    {"m_time_port", "Chrono des Quais",
        "Un chrono honnete sur le Circuit Technique, entre les grues et "
        "l'acier.",
        MissionKind::TIME_TRIAL, RegionId::PORT, 115.0f, 12, BiomeId::PORT},
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

namespace {

struct LoreSpot {
    float x;
    float z;
    RegionId region;
};

// 20 plaques des Veilleurs, cinq par region, placees pres des routes et
// reperes existants (hubs, garages, circuits) pour rester accessibles a
// pied/en voiture dans les WorldBounds actuelles.
const LoreSpot kLoreSpots[20] = {
    // Marina (cote solaire)
    {20.0f, -10.0f, RegionId::MARINA},
    {40.0f, 20.0f, RegionId::MARINA},
    {0.0f, 20.0f, RegionId::MARINA},
    {60.0f, 10.0f, RegionId::MARINA},
    {28.0f, -40.0f, RegionId::MARINA},
    // Foret brumeuse
    {-50.0f, -70.0f, RegionId::FOREST},
    {-90.0f, -140.0f, RegionId::FOREST},
    {-30.0f, -110.0f, RegionId::FOREST},
    {-70.0f, -50.0f, RegionId::FOREST},
    {-100.0f, -170.0f, RegionId::FOREST},
    // Port industriel
    {130.0f, -40.0f, RegionId::PORT},
    {170.0f, -10.0f, RegionId::PORT},
    {190.0f, -60.0f, RegionId::PORT},
    {150.0f, -70.0f, RegionId::PORT},
    {200.0f, -30.0f, RegionId::PORT},
    // Caldeira volcanique
    {70.0f, 100.0f, RegionId::VOLCANO},
    {100.0f, 150.0f, RegionId::VOLCANO},
    {60.0f, 180.0f, RegionId::VOLCANO},
    {180.0f, 190.0f, RegionId::VOLCANO},
    {140.0f, 170.0f, RegionId::VOLCANO},
};

// Titres et textes de l'encyclopedie des Veilleurs, un par plaque (voir
// kLoreSpots ci-dessus : indices 0-4 Marina, 5-9 Foret, 10-14 Port,
// 15-19 Volcan). Contenu narratif ajoute pour l'ecran encyclopedie du
// menu pause ; ne modifie pas le placement des collectibles existants.
const char *const kLoreTitles[20] = {
    // Marina
    "Carnet de l'Anneau Vitesse",
    "Note d'atelier, Garage Marina",
    "Essais de nuit",
    "La Corniche",
    "Tableau des chronos",
    // Foret
    "Carnet des premiers releves",
    "Freins dans la brume",
    "Le vieux funiculaire",
    "Racines et differentiel",
    "La lanterne du soir",
    // Port
    "Veille des docks",
    "Note de l'Usine Rouge",
    "VEIL-01",
    "Chronos de livraison",
    "Devise du dernier quai",
    // Volcan
    "Pneus sur roche tiede",
    "Registre de l'Observatoire",
    "La route abimee",
    "Le dernier tracé",
    "Derniere plaque connue",
};

const char *const kLoreTexts[20] = {
    // Marina (cote solaire)
    "Plaque retrouvee pres de l'Anneau Vitesse. \"Ici, les Veilleurs "
    "reglaient les carburateurs au son du ressac. Une bonne mise au "
    "point ne se mesure pas au banc, disaient-ils, mais a l'oreille, "
    "face a la mer.\"",
    "Sous la tole rouillee du Garage Marina, une note d'atelier des "
    "Veilleurs subsiste encore : \"Graissez les rotules avant chaque "
    "maree haute. Le sel ne pardonne rien, mais une mecanique honnete "
    "tient bon.\"",
    "Fragment grave dans le beton du quai. Les Veilleurs y organisaient "
    "leurs essais de nuit, phares eteints, pour apprendre la route par "
    "coeur avant de la dompter de jour.",
    "Une plaque tournee vers le large. On y lit : \"La corniche ne "
    "pardonne pas les exces d'orgueil. Roulez comme si la mer vous "
    "regardait, elle le fait.\" Signe, un Veilleur anonyme, 1986.",
    "Vestige d'un vieux tableau d'affichage des chronos. Les meilleurs "
    "temps de l'Anneau Vitesse y sont encore lisibles, a moitie effaces "
    "par les embruns, comme un defi lance aux pilotes d'aujourd'hui.",
    // Foret brumeuse
    "Plaque humide, presque avalee par la mousse. Les Veilleurs "
    "cartographiaient la foret a pied, un carnet et une lampe a "
    "petrole, avant meme que la premiere route n'y soit tracee.",
    "Ici, disent les archives des Veilleurs, la brume se levait "
    "toujours au meme endroit a l'aube. Ils y testaient leurs freins, "
    "sachant qu'on ne voit le danger qu'a la derniere seconde sous la "
    "canopee.",
    "Un morceau de rail rouille, vestige d'un ancien funiculaire "
    "forestier. Les Veilleurs l'utilisaient pour hisser les pieces "
    "detachees jusqu'a leur atelier cache entre les pins.",
    "Plaque a demi enterree sous les racines. On y devine : \"La foret "
    "ne se domine pas, elle se traverse avec respect, et un bon "
    "differentiel.\"",
    "Dernier repere avant le circuit sinueux. Les Veilleurs y "
    "laissaient une lanterne allumee toute la nuit, pour guider les "
    "mecaniciens en retard revenant de leurs essais.",
    // Port industriel
    "Plaque rivetee a une grue rouillee. Les Veilleurs y testaient "
    "l'endurance des moteurs, faisant tourner les voitures au ralenti "
    "des nuits entieres face aux docks, pour ecouter le moindre rate.",
    "Note retrouvee dans un casier de l'Usine Rouge : \"Un moteur qui "
    "tousse en dit plus qu'un ingenieur qui parle. Ecoutez avant de "
    "demonter.\" Atelier des Veilleurs, section Port.",
    "Fragment d'une plaque d'immatriculation d'essai, gravee \"VEIL-01\". "
    "On raconte que c'etait la premiere voiture-laboratoire du Port, "
    "jamais retrouvee depuis sa disparition en pleine tempete.",
    "Plaque scellee dans le beton d'un quai desaffecte. Les Veilleurs y "
    "chronometraient les livraisons comme des courses, convaincus que "
    "la rigueur d'un pilote se forge aussi dans le travail ordinaire.",
    "Dernier vestige industriel avant l'eau libre. On y lit une devise "
    "a moitie rouillee : \"Charge honnete, moteur honnete, route "
    "honnete.\"",
    // Caldeira volcanique
    "Plaque noircie par les cendres. Les Veilleurs y etudiaient la "
    "tenue des pneus sur roche volcanique encore tiede, une science "
    "qu'ils ne partageaient qu'a voix basse.",
    "Sous l'Observatoire, un registre des Veilleurs recense chaque "
    "ascension de la caldeira. La derniere entree, inachevee, s'arrete "
    "au milieu d'une phrase sur la chaleur des freins.",
    "Plaque tournee vers le crater. \"Ici, la route abimee n'est pas un "
    "defaut, c'est un maitre. Elle vous apprend en un virage ce que dix "
    "circuits lisses n'enseignent jamais.\"",
    "Fragment retrouve au bord du Circuit des Cendres. Les Veilleurs y "
    "auraient trace leur dernier parcours avant de disparaitre, "
    "laissant derriere eux plus de questions que de reponses.",
    "Derniere plaque connue des Veilleurs, presque effacee par la lave "
    "refroidie. On y devine seulement : \"Merci d'avoir continue la "
    "route.\" Le reste s'est perdu dans les cendres.",
};

} // namespace

const char *AureliaData::loreTitle(int index)
{
    if (index < 0 || index >= 20) {
        return "???";
    }
    return kLoreTitles[static_cast<size_t>(index)];
}

const char *AureliaData::loreText(int index)
{
    if (index < 0 || index >= 20) {
        return "";
    }
    return kLoreTexts[static_cast<size_t>(index)];
}

void AureliaData::initCollectibles()
{
    static bool done = false;
    if (done) {
        return;
    }
    done = true;
    for (int i = 0; i < 20; ++i) {
        const LoreSpot &spot = kLoreSpots[static_cast<size_t>(i)];
        PoiInstance lore{};
        lore.id = "lore";
        lore.type = PoiType::COLLECTIBLE;
        lore.label = "Plaque des Veilleurs";
        lore.worldX = spot.x;
        lore.worldZ = spot.z;
        lore.radius = 6.0f;
        lore.loreIndex = i;
        lore.region = spot.region;
        lore.color = Color{255, 220, 120, 200};
        g_pois.push_back(lore);
    }
}

} // namespace racer::world
