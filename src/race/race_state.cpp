#include "race/race_state.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <numeric>

namespace racer {

namespace {
float NormalizeAngle(float a) {
    while (a > PI) a -= 2.0f * PI;
    while (a < -PI) a += 2.0f * PI;
    return a;
}
float Sign(float v) { return v > 0.0f ? 1.0f : (v < 0.0f ? -1.0f : 0.0f); }
} // namespace

RaceState::RaceState(Track track, int lapsToWin, int aiCount) : track_(std::move(track)), lapsToWin_(lapsToWin) {
    int totalCars = aiCount + 1;

    RacerEntry player;
    player.name = "Joueur";
    player.isPlayer = true;
    player.car.position = track_.StartPosition(0, totalCars);
    player.car.heading = track_.StartHeading();
    player.car.velocityHeading = player.car.heading;
    player.lastSegment = track_.ProjectPosition(player.car.position).segmentIndex;
    racers_.push_back(player);
    playerIndex_ = 0;

    for (int i = 0; i < aiCount; ++i) {
        RacerEntry ai;
        ai.name = "IA " + std::to_string(i + 1);
        ai.car.position = track_.StartPosition(i + 1, totalCars);
        ai.car.heading = track_.StartHeading();
        ai.car.velocityHeading = ai.car.heading;
        // La grille de depart place les voitures en retrait (cf. Track::StartPosition,
        // parametre "back") -- leur position initiale projette donc deja pres de la
        // FIN de la piste, pas du segment 0. Sans le garde-fou passedMidpoint (voir
        // Update), ce petit rattrapage initial serait compte comme un tour complet.
        ai.lastSegment = track_.ProjectPosition(ai.car.position).segmentIndex;
        racers_.push_back(ai);

        // Grille "qualifications" : la meilleure IA part devant. L'inverse
        // (faibles devant) transforme la course en bouchon -- personne ne
        // depasse un peloton compact et l'ordre d'arrivee ne reflete plus
        // du tout le niveau des pilotes.
        float skill = 1.0f - 0.05f * static_cast<float>(i);
        // Le skill s'incarne aussi (legerement) dans la voiture, pour que la
        // hierarchie persiste meme en ligne droite.
        ai.car.tuning.maxSpeed *= 0.90f + 0.10f * skill;
        ai.car.tuning.acceleration *= 0.85f + 0.15f * skill;
        // Graine stable par index : personnalites distinctes meme a skill egal.
        aiDrivers_.emplace_back(skill, static_cast<unsigned int>(1000 + i * 7919));
    }
}

void RaceState::Update(float dt, const CarInput& playerInput) {
    if (phase_ == RacePhase::Countdown) {
        countdownRemaining_ -= dt;
        if (countdownRemaining_ <= 0.0f) {
            countdownRemaining_ = 0.0f;
            phase_ = RacePhase::Racing;
        }
        return;
    }

    if (phase_ == RacePhase::Finished) return;

    elapsedTime_ += dt;

    int numSegments = static_cast<int>(track_.Waypoints().size());

    for (size_t i = 0; i < racers_.size(); ++i) {
        RacerEntry& r = racers_[i];
        if (r.finished) continue;

        CarInput input = r.isPlayer ? playerInput : aiDrivers_[i - 1].ComputeInput(r.car, track_);
        r.lastInput = input; // expose l'input reellement applique (feux stop, VFX)
        r.car.Update(input, dt);

        Track::Progress prog = track_.ProjectPosition(r.car.position);

        // Surfaces qui comptent : pose le grip/la trainee pour la prochaine
        // frame selon la position actuelle. Herbe (au-dela du vibreur) tres
        // punitive ; chaussee abimee legerement glissante ; sinon reference.
        if (std::fabs(prog.lateralOffset) > track_.Width() * 0.5f + 0.6f) {
            r.car.surfaceGrip = 0.55f;
            r.car.surfaceDrag = 3.0f;
        } else if (track_.Style() == SurfaceStyle::Abimee) {
            r.car.surfaceGrip = 0.85f;
            r.car.surfaceDrag = 1.15f;
        } else {
            r.car.surfaceGrip = 1.0f;
            r.car.surfaceDrag = 1.0f;
        }

        // Passage pres de la moitie de la piste : condition necessaire avant
        // qu'un "haut segment -> bas segment" compte comme un tour. Sans ce
        // garde-fou, une voiture qui demarre en retrait de la ligne (grille
        // de depart) verrait son court rattrapage initial jusqu'au segment 0
        // comptabilise comme un tour complet.
        int mid = numSegments / 2;
        int midWindow = numSegments / 10;
        if (std::abs(prog.segmentIndex - mid) <= midWindow) {
            r.passedMidpoint = true;
        }

        // Detection de tour complete : passage de la fin de piste (dernier
        // segment) au debut (segment 0) dans le sens de la marche.
        if (r.passedMidpoint && r.lastSegment > numSegments * 7 / 10 && prog.segmentIndex < numSegments * 3 / 10) {
            r.lap += 1;
            r.passedMidpoint = false;
            if (r.lap >= lapsToWin_) {
                r.finished = true;
                r.finishTime = elapsedTime_;
            }
        }
        r.lastSegment = prog.segmentIndex;

        if (r.isPlayer && r.finished) {
            phase_ = RacePhase::Finished;
        }
    }

    ResolveCarContacts();
}

// Collisions voiture-voiture : spheres de rayon 1.5 sur les positions.
// Resolution purement positionnelle + amortissement leger, pensee pour rester
// stable a 60 Hz meme en peloton serre (corrections bornees, relaxation 50 %,
// jamais de NaN si deux positions sont confondues).
void RaceState::ResolveCarContacts() {
    constexpr float kContactDist = 3.0f;    // somme des rayons (1.5 + 1.5)
    constexpr float kMaxPush = 0.25f;       // separation max par voiture et par frame
    constexpr float kSpeedDamping = 0.96f;  // ~4 % de vitesse perdue par frame de contact
    constexpr float kMaxDeflect = 0.06f;    // deviation max de velocityHeading (rad)

    for (size_t i = 0; i < racers_.size(); ++i) {
        if (racers_[i].finished) continue; // les arrivees deviennent des "fantomes"
        for (size_t j = i + 1; j < racers_.size(); ++j) {
            if (racers_[j].finished) continue;
            Car& a = racers_[i].car;
            Car& b = racers_[j].car;

            float dx = b.position.x - a.position.x;
            float dz = b.position.z - a.position.z;
            float distSq = dx * dx + dz * dz;
            if (distSq >= kContactDist * kContactDist) continue;

            float dist = std::sqrt(distSq);
            float nx, nz; // axe de contact, de A vers B
            if (dist > 1e-4f) {
                nx = dx / dist;
                nz = dz / dist;
            } else {
                // Positions confondues : axe fallback = forward de A (pas de division par ~0).
                Vector3 fwd = a.Forward();
                nx = fwd.x;
                nz = fwd.z;
                dist = 0.0f;
            }

            // Relaxation : ne resorbe que la moitie du recouvrement par frame,
            // partagee 50/50 -- converge sans osciller quand plusieurs paires
            // se poussent mutuellement (peloton de depart).
            float overlap = kContactDist - dist;
            float push = std::min(overlap * 0.25f, kMaxPush);
            a.position.x -= nx * push;
            a.position.z -= nz * push;
            b.position.x += nx * push;
            b.position.z += nz * push;

            // Le contact ne freine que la voiture qui avance VERS l'autre
            // (celle qui percute), proportionnellement a la violence du
            // rapprochement (4 % pleins a partir de ~6 u/s de fermeture).
            // Amortir les deux inconditionnellement cree des "trains"
            // nez-a-queue figes ; amortir a fond un simple frottement lateral
            // ecrase tout le peloton dans les virages.
            Vector3 va = a.Velocity();
            Vector3 vb = b.Velocity();
            float closing = (va.x - vb.x) * nx + (va.z - vb.z) * nz;
            float damping = 1.0f - (1.0f - kSpeedDamping) * std::clamp(closing / 6.0f, 0.0f, 1.0f);
            bool aRams = va.x * nx + va.z * nz > 0.0f;  // A avance vers B
            bool bRams = vb.x * nx + vb.z * nz < 0.0f;  // B avance vers A
            if (aRams) a.speed *= damping;
            if (bRams) b.speed *= damping;

            // Deviation de trajectoire : chacun est deflechi du cote oppose au
            // contact, proportionnellement au recouvrement (borne kMaxDeflect).
            // Le signe vient du produit vectoriel 2D (direction x normale).
            float deflect = std::min(kMaxDeflect, overlap * 0.04f);
            float ax = std::sin(a.velocityHeading), az = std::cos(a.velocityHeading);
            float bx = std::sin(b.velocityHeading), bz = std::cos(b.velocityHeading);
            float sideA = Sign(ax * nz - az * nx);
            float sideB = Sign(bx * (-nz) - bz * (-nx));
            a.velocityHeading = NormalizeAngle(a.velocityHeading + sideA * deflect);
            b.velocityHeading = NormalizeAngle(b.velocityHeading + sideB * deflect);

            // Echappatoire laterale : la voiture qui percute glisse aussi vers
            // le cote de sa deviation. Sans cela, un contact nez-a-queue est un
            // mur : l'axe de contact etant longitudinal, la separation ne fait
            // que repousser le poursuivant en arriere et tout depassement est
            // impossible (le peloton fige en train derriere le plus lent).
            if (aRams) {
                a.position.x += sideA * az * push * 0.6f;
                a.position.z += sideA * (-ax) * push * 0.6f;
            }
            if (bRams) {
                b.position.x += sideB * bz * push * 0.6f;
                b.position.z += sideB * (-bx) * push * 0.6f;
            }
        }
    }
}

float RaceState::RaceProgress(const RacerEntry& r) const {
    Track::Progress prog = track_.ProjectPosition(r.car.position);
    return static_cast<float>(r.lap) * track_.TotalLength() + track_.CumulativeDistance(prog);
}

std::vector<int> RaceState::Standings() const {
    std::vector<int> order(racers_.size());
    std::iota(order.begin(), order.end(), 0);

    std::sort(order.begin(), order.end(), [this](int a, int b) {
        const RacerEntry& ra = racers_[static_cast<size_t>(a)];
        const RacerEntry& rb = racers_[static_cast<size_t>(b)];
        if (ra.finished != rb.finished) return ra.finished && !rb.finished ? true : false;
        if (ra.finished && rb.finished) return ra.finishTime < rb.finishTime;
        return RaceProgress(ra) > RaceProgress(rb);
    });

    return order;
}

int RaceState::PlayerPosition() const {
    std::vector<int> order = Standings();
    for (size_t i = 0; i < order.size(); ++i) {
        if (order[i] == playerIndex_) return static_cast<int>(i) + 1;
    }
    return static_cast<int>(order.size());
}

} // namespace racer
