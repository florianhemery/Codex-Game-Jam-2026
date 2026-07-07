// Simulation headless d'une course complete (le "joueur" est pilote par une
// IA comme les autres) : verifie la logique de jeu (tours, classement,
// arrivee) sans avoir besoin de conduire manuellement ni d'ouvrir de fenetre.
#include <cstdio>

#include "ai/ai_driver.h"
#include "race/race_state.h"
#include "track/track.h"

int main() {
    racer::RaceState race(racer::Track::Make(racer::Track::Presets()[0]), /*lapsToWin=*/2, /*aiCount=*/3);
    racer::AIDriver playerAsAI(1.0f);

    constexpr float dt = 1.0f / 60.0f;
    constexpr int kMaxSteps = 60 * 90; // 90s de simulation max, garde-fou anti-boucle infinie

    bool earlyFalseLap = false;
    int step = 0;
    for (; step < kMaxSteps; ++step) {
        racer::CarInput playerInput = playerAsAI.ComputeInput(race.Racers()[static_cast<size_t>(race.PlayerIndex())].car,
                                                               race.GetTrack());
        race.Update(dt, playerInput);

        // Garde-fou de non-regression : un tour compte dans les 5 premieres
        // secondes de course serait forcement un faux positif (cf. bug du
        // "rattrapage de grille de depart" corrige le 2026-07-06).
        if (race.ElapsedTime() < 5.0f) {
            for (const auto& r : race.Racers()) {
                if (r.lap > 0) earlyFalseLap = true;
            }
        }

        if (step % 60 == 0) { // ~1x par seconde simulee
            std::printf("t=%5.1fs  ", static_cast<float>(step) * dt);
            for (const auto& r : race.Racers()) {
                std::printf("%s[lap=%d,v=%5.1f] ", r.name.c_str(), r.lap, r.car.speed);
            }
            std::printf("pos_joueur=%d\n", race.PlayerPosition());
        }

        if (race.Phase() == racer::RacePhase::Finished) break;
    }

    if (earlyFalseLap) {
        std::printf("ERREUR: un tour a ete comptabilise dans les 5 premieres secondes (faux positif de grille de depart)\n");
    }

    bool finished = race.Phase() == racer::RacePhase::Finished;
    std::printf("Simulation terminee apres %d pas (%.1fs simulees). Phase finished=%d\n", step,
                static_cast<float>(step) * dt, finished);

    for (size_t i = 0; i < race.Racers().size(); ++i) {
        const auto& r = race.Racers()[i];
        std::printf("  %s : tours=%d fini=%d temps=%.2f pos=(%.1f,%.1f,%.1f) vitesse=%.1f\n", r.name.c_str(), r.lap,
                    r.finished, r.finishTime, r.car.position.x, r.car.position.y, r.car.position.z, r.car.speed);
    }

    std::printf("Classement final :");
    for (int idx : race.Standings()) {
        std::printf(" %s", race.Racers()[static_cast<size_t>(idx)].name.c_str());
    }
    std::printf("\n");

    bool ok = finished && !earlyFalseLap && race.Racers()[static_cast<size_t>(race.PlayerIndex())].lap >= 2;
    std::printf("RACE_SIM: %s\n", ok ? "OK" : "ECHEC");
    return ok ? 0 : 1;
}
