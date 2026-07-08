/*
** EPITECH PROJECT, 2026
** racer
** File description:
** ChunkTypes labels
*/

#include "World/Chunk/ChunkTypes.hpp"

#include <cmath>

namespace racer::world {

const char *biomeLabel(BiomeId biome)
{
    switch (biome) {
    case BiomeId::COAST:
        return "Cote solaire";
    case BiomeId::FOREST:
        return "Foret brumeuse";
    case BiomeId::PORT:
        return "Port industriel";
    case BiomeId::VOLCANO:
        return "Caldeira volcanique";
    default:
        return "Aurelia";
    }
}

const char *regionLabel(BiomeId biome)
{
    switch (biome) {
    case BiomeId::COAST:
        return "Marina";
    case BiomeId::FOREST:
        return "Foret";
    case BiomeId::PORT:
        return "Port";
    case BiomeId::VOLCANO:
        return "Volcan";
    default:
        return "Aurelia";
    }
}

} // namespace racer::world
