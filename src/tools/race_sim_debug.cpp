// Simulation headless de courses completes (le "joueur" est pilote par une
// IA comme les autres) : verifie la logique de jeu (tours, classement,
// arrivee, surfaces, collisions) sans fenetre ni pilotage manuel.
//
// Usage : race_sim_debug [indexPreset]
//   - sans argument : boucle sur TOUS les presets de Track::Presets()
//   - avec un index : ne simule que ce preset
// Code retour 0 si toutes les courses simulees sont saines.
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "ai/ai_driver.h"
#include "race/race_state.h"
#include "track/track.h"

namespace {

// Progression totale d'une voiture en unites monde (tours + distance dans le tour).
float TotalProgress(const racer::Track& track, const racer::RacerEntry& r) {
    racer::Track::Progress prog = track.ProjectPosition(r.car.position);
    return static_cast<float>(r.lap) * track.TotalLength() + track.CumulativeDistance(prog);
}

bool IsFiniteCar(const racer::Car& c) {
    return std::isfinite(c.position.x) && std::isfinite(c.position.z) && std::isfinite(c.speed) &&
           std::isfinite(c.velocityHeading) && std::isfinite(c.heading);
}

// Simule une course complete sur un preset ; retourne true si tout est sain.
bool RunRace(int presetIndex) {
    const racer::TrackDef& def = racer::Track::Presets()[static_cast<size_t>(presetIndex)];
    constexpr int kAiCount = 4; // 5 voitures au depart : eprouve la stabilite des collisions en peloton
    constexpr int kLaps = 3;    // comme le vrai jeu (main.cpp) : laisse le temps au skill de s'exprimer
    racer::RaceState race(racer::Track::Make(def), kLaps, kAiCount);
    racer::AIDriver playerAsAI(1.0f);

    std::printf("=== Preset %d : %s (surface %s, longueur %.0f u) ===\n", presetIndex, def.name.c_str(),
                def.surfaceStyle == racer::SurfaceStyle::Abimee ? "abimee" : "propre",
                race.GetTrack().TotalLength());

    constexpr float dt = 1.0f / 60.0f;
    constexpr int kMaxSteps = 60 * 180; // 3 min max, garde-fou anti-boucle infinie

    size_t carCount = race.Racers().size();
    std::vector<int> offTrackExits(carCount, 0);   // transitions piste -> herbe
    std::vector<bool> wasOffTrack(carCount, false);
    float minPairDist = 1e9f;   // distance min entre deux voitures sur toute la course
    int contactPairFrames = 0;  // (paire, frame) en contact (< 3.0 u) -- mesure la frequence des frottements
    int overlapPairFrames = 0;  // (paire, frame) interpenetrees (< 2.7 u) : resolution en retard = instabilite
    bool sawNaN = false;
    bool earlyFalseLap = false;
    float offTrackHalfWidth = race.GetTrack().Width() * 0.5f + 0.6f; // meme tolerance vibreur que RaceState

    int step = 0;
    for (; step < kMaxSteps; ++step) {
        racer::CarInput playerInput =
            playerAsAI.ComputeInput(race.Racers()[static_cast<size_t>(race.PlayerIndex())].car, race.GetTrack());
        race.Update(dt, playerInput);

        // Garde-fou de non-regression : un tour compte dans les 5 premieres
        // secondes serait un faux positif (bug de grille corrige le 2026-07-06).
        if (race.ElapsedTime() < 5.0f) {
            for (const auto& r : race.Racers()) {
                if (r.lap > 0) earlyFalseLap = true;
            }
        }

        for (size_t i = 0; i < carCount; ++i) {
            const racer::RacerEntry& r = race.Racers()[i];
            if (!IsFiniteCar(r.car)) sawNaN = true;

            bool off = std::fabs(race.GetTrack().ProjectPosition(r.car.position).lateralOffset) > offTrackHalfWidth;
            if (off && !wasOffTrack[i]) offTrackExits[i]++;
            wasOffTrack[i] = off;

            for (size_t j = i + 1; j < carCount; ++j) {
                const racer::Car& a = r.car;
                const racer::Car& b = race.Racers()[j].car;
                float dx = b.position.x - a.position.x;
                float dz = b.position.z - a.position.z;
                float d = std::sqrt(dx * dx + dz * dz);
                minPairDist = std::min(minPairDist, d);
                if (d < 3.0f) contactPairFrames++;
                if (d < 2.7f) overlapPairFrames++;
            }
        }

        if (step % 300 == 0) { // ~1 ligne toutes les 5 s simulees
            std::printf("t=%5.1fs  ", static_cast<float>(step) * dt);
            for (const auto& r : race.Racers()) {
                std::printf("%s[lap=%d,v=%4.1f] ", r.name.c_str(), r.lap, r.car.speed);
            }
            std::printf("pos_joueur=%d\n", race.PlayerPosition());
        }

        if (race.Phase() == racer::RacePhase::Finished) break;
    }

    bool finished = race.Phase() == racer::RacePhase::Finished;
    std::printf("-- Course terminee : %s apres %.1fs simulees --\n", finished ? "oui" : "NON (cutoff!)",
                static_cast<float>(step) * dt);

    // Classement final + ecarts au leader (en s si arrive, sinon en metres de retard).
    std::vector<int> order = race.Standings();
    const racer::RacerEntry& leader = race.Racers()[static_cast<size_t>(order[0])];
    float leaderProgress = TotalProgress(race.GetTrack(), leader);
    float minGap = 1e9f, maxGap = 0.0f; // ecarts successifs (metres) entre voitures consecutives au classement
    float prevProgress = leaderProgress;

    for (size_t p = 0; p < order.size(); ++p) {
        const racer::RacerEntry& r = race.Racers()[static_cast<size_t>(order[p])];
        float progress = TotalProgress(race.GetTrack(), r);
        if (p > 0) {
            float gap = prevProgress - progress;
            minGap = std::min(minGap, gap);
            maxGap = std::max(maxGap, gap);
        }
        prevProgress = progress;

        std::printf("  %zu. %-7s tours=%d sorties_piste=%d ", p + 1, r.name.c_str(), r.lap,
                    offTrackExits[static_cast<size_t>(order[p])]);
        if (r.finished) {
            std::printf("temps=%.2fs", r.finishTime);
        } else {
            std::printf("retard=%.0fu", leaderProgress - progress);
        }
        std::printf(" v=%.1f\n", r.car.speed);
    }

    std::printf("  Stabilite contacts : dist_min_paires=%.2fu paires_frames_contact=%d paires_frames_interpen=%d\n",
                minPairDist, contactPairFrames, overlapPairFrames);
    std::printf("  Ecarts consecutifs au classement : min=%.0fu max=%.0fu\n", minGap, maxGap);

    // Criteres de sante : course finie, pas de NaN, pas de faux tour, le
    // joueur (skill 1.0) a bien boucle ses tours, et personne n'est fige.
    bool anyFrozen = false;
    for (const auto& r : race.Racers()) {
        if (!r.finished && std::fabs(r.car.speed) < 0.5f) anyFrozen = true;
    }
    bool ok = finished && !earlyFalseLap && !sawNaN && !anyFrozen &&
              race.Racers()[static_cast<size_t>(race.PlayerIndex())].lap >= kLaps;
    if (sawNaN) std::printf("  ERREUR: NaN detecte dans l'etat d'une voiture\n");
    if (earlyFalseLap) std::printf("  ERREUR: tour comptabilise dans les 5 premieres secondes\n");
    if (anyFrozen) std::printf("  ERREUR: voiture figee (v~0) a la fin de la course\n");
    std::printf("  RACE_SIM[%d]: %s\n\n", presetIndex, ok ? "OK" : "ECHEC");
    return ok;
}

} // namespace

int main(int argc, char** argv) {
    int presetCount = static_cast<int>(racer::Track::Presets().size());
    bool allOk = true;

    if (argc > 1) {
        int idx = std::atoi(argv[1]);
        if (idx < 0 || idx >= presetCount) {
            std::printf("Index de preset invalide : %d (0..%d)\n", idx, presetCount - 1);
            return 1;
        }
        allOk = RunRace(idx);
    } else {
        for (int i = 0; i < presetCount; ++i) {
            allOk = RunRace(i) && allOk;
        }
    }

    std::printf("RACE_SIM_GLOBAL: %s\n", allOk ? "OK" : "ECHEC");
    return allOk ? 0 : 1;
}
