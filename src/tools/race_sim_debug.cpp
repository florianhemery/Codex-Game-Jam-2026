// Simulation headless d'une course complete (le "joueur" est pilote par une
// IA comme les autres) : verifie la logique de jeu (tours, classement,
// arrivee) sans avoir besoin de conduire manuellement ni d'ouvrir de fenetre.
#include <cstdio>

#include "ai/ai_driver.h"
#include "race/race_state.h"
#include "track/track.h"

int main() {
    racer::RaceState race(racer::Track::MakeStadiumTrack(), /*lapsToWin=*/2, /*aiCount=*/3);
    racer::AIDriver playerAsAI(1.0f);

    constexpr float dt = 1.0f / 60.0f;
    constexpr int kMaxSteps = 60 * 90; // 90s de simulation max, garde-fou anti-boucle infinie

    int step = 0;
    for (; step < kMaxSteps; ++step) {
        racer::CarInput playerInput = playerAsAI.ComputeInput(race.Racers()[static_cast<size_t>(race.PlayerIndex())].car,
                                                               race.GetTrack());
        race.Update(dt, playerInput);
        if (race.Phase() == racer::RacePhase::Finished) break;
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

    bool ok = finished && race.Racers()[static_cast<size_t>(race.PlayerIndex())].lap >= 2;
    std::printf("RACE_SIM: %s\n", ok ? "OK" : "ECHEC");
    return ok ? 0 : 1;
}
