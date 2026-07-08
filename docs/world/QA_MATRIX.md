# Matrice QA monde Aurélia (extrait 200 cases)

| ID | Zone | Test | Attendu |
|----|------|------|---------|
| Q001 | Marina | Spawn joueur | Y cohérent, pas sous sol |
| Q002 | Marina | Stream 3×3 | 9 chunks, pas de trou visible |
| Q003 | Marina | Entrée Anneau | Trigger + course démarre |
| Q004 | Marina | Retour course | Respawn garage marina |
| Q005 | Forêt | Brouillard | Densité > côte |
| Q006 | Forêt | Circuit Sinueux | Trigger actif |
| Q007 | Port | Trafic | ≤20 véhicules, despawn hors chunk |
| Q008 | Port | Circuit Technique | Trigger actif |
| Q009 | Volcan | Ambiance | Crépuscule/orage selon heure |
| Q010 | Volcan | Route Abîmée | Trigger actif |
| Q011 | Volcan | Circuit Cendres | Débloqué rep ≥50 |
| Q012 | Global | Tab course rapide | Overlay depuis OW |
| Q013 | Global | Échap menu | Retour menu sans crash |
| Q014 | Global | Pause course OW | Identique course menu |
| Q015 | Global | FPS Marina | ≥60 @ 1280×720 |
| Q016 | Missions | 6 rejouables | Score + récompense rep |
| Q017 | Progression | Garage forêt | Rep forêt ≥25 |
| Q018 | Carte | Touche M | Minimap monde + POI |
| Q019 | Audio | Région forêt | Mix plus sourd |
| Q020 | Collectibles | 20 fragments | Compteur HUD |

*(Cases Q021–Q200 : répétition par chunk, mission, et combinaison biome/météo.)*

## Dépréciation OpenWorldHub

- [x] M1 : `USE_AURELIA=1` par défaut
- [x] M6 : suppression `OpenWorldHub.*`, tests verts
