#include "ai/ai_driver.h"

#include <algorithm>
#include <cmath>

namespace racer {

namespace {
float NormalizeAngle(float a) {
    while (a > PI) a -= 2.0f * PI;
    while (a < -PI) a += 2.0f * PI;
    return a;
}

// Petit hash entier deterministe (pas de <random> : pas d'etat, pas d'alea global).
unsigned int HashU32(unsigned int x) {
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}

// Budget de rotation de trajectoire (rad/s) que le grip normal absorbe sans
// glisse excessive : v_max_virage = kGripBudget / courbure.
constexpr float kGripBudget = 1.4f;
// En dessous de cette vitesse on ne freine plus pour un virage (arcade :
// mieux vaut glisser un peu que ramper).
constexpr float kMinCornerSpeed = 7.0f;
} // namespace

AIDriver::AIDriver(float skill, unsigned int seed) : skill_(skill) {
    if (seed == 0) {
        seed = static_cast<unsigned int>(skill * 997.0f) + 17u; // derive du skill, reproductible
    }
    unsigned int h = HashU32(seed);
    // Offset lateral prefere dans +-1.8 u : trajectoires variees -> depassements naturels.
    laneOffset_ = (static_cast<float>(h % 1000u) / 999.0f * 2.0f - 1.0f) * 1.8f;
    // Les pilotes faibles gardent une reserve plus haute : nitro plus rare
    // et par petites rafales (ils "n'osent pas" vider la bouteille).
    float weakness = std::max(0.0f, 1.0f - skill_);
    nitroReserve_ = 1.2f + weakness * 4.0f;
}

CarInput AIDriver::ComputeInput(const Car& car, const Track& track) const {
    Track::Progress prog = track.ProjectPosition(car.position);
    float currentDist = track.CumulativeDistance(prog);
    float speedAbs = std::fabs(car.speed);

    // --- Anticipation : courbure de la ligne centrale echantillonnee par
    // cordes devant la voiture (fenetre allongee avec la vitesse). Chaque
    // echantillon impose une limite "je dois pouvoir freiner d'ici la" :
    // v_ok^2 = v_virage^2 + 2*a_frein*d. On roule donc vite ENTRE les
    // virages et on freine juste avant, au lieu de subir toute courbure
    // apercue au loin.
    float chord = 4.0f + speedAbs * 0.12f;
    float aBrake = car.tuning.braking * 0.7f; // marge : le freinage reel n'est jamais parfait
    float maxTurnPerU = 0.0f;
    float vLimit = 1e9f;
    Vector2 prev = track.PointAtDistance(currentDist);
    Vector2 cur = track.PointAtDistance(currentDist + chord);
    for (int s = 1; s <= 5; ++s) {
        Vector2 next = track.PointAtDistance(currentDist + chord * static_cast<float>(s + 1));
        float h1 = std::atan2(cur.x - prev.x, cur.y - prev.y);
        float h2 = std::atan2(next.x - cur.x, next.y - cur.y);
        float turnPerU = std::fabs(NormalizeAngle(h2 - h1)) / chord;
        maxTurnPerU = std::max(maxTurnPerU, turnPerU);

        // Vitesse de passage du virage echantillonne, en skill^2 : c'est en
        // virage que la hierarchie des pilotes doit s'exprimer (sinon l'ordre
        // de grille decide de tout, personne ne double jamais).
        float vCorner = std::max(kMinCornerSpeed, kGripBudget * skill_ * skill_ / std::max(turnPerU, 1e-3f));
        // ...relevee de ce que la distance restante permet encore de freiner.
        float distToTurn = chord * static_cast<float>(s - 1); // premier echantillon = contrainte immediate
        float vAllowed = std::sqrt(vCorner * vCorner + 2.0f * aBrake * distToTurn);
        vLimit = std::min(vLimit, vAllowed);

        prev = cur;
        cur = next;
    }

    // --- Point vise : ligne centrale + offset de personnalite, ramene vers 0
    // quand un virage serre approche (tout le monde repasse par la corde).
    float lookahead = 10.0f + speedAbs * 0.6f;
    float targetDist = currentDist + lookahead;
    Vector2 target = track.PointAtDistance(targetDist);
    // Garde ~40 % de l'offset meme en virage serre : deux voitures au coude a
    // coude n'ont pas exactement la meme corde, sinon tout le monde se gene.
    float offsetScale = std::clamp(1.0f - maxTurnPerU / 0.06f, 0.4f, 1.0f);
    Vector2 ta = track.PointAtDistance(targetDist - 2.0f);
    Vector2 tb = track.PointAtDistance(targetDist + 2.0f);
    float dxT = tb.x - ta.x;
    float dzT = tb.y - ta.y;
    float len = std::sqrt(dxT * dxT + dzT * dzT);
    if (len > 1e-4f) {
        // Perpendiculaire a la ligne centrale (plan XZ).
        target.x += (-dzT / len) * laneOffset_ * offsetScale;
        target.y += (dxT / len) * laneOffset_ * offsetScale;
    }

    float dx = target.x - car.position.x;
    float dz = target.y - car.position.z; // Vector2::y stocke la coordonnee monde Z
    float desiredHeading = std::atan2(dx, dz);
    float headingError = NormalizeAngle(desiredHeading - car.heading);
    float turnSeverity = std::fabs(headingError);

    CarInput input;
    input.steer = std::clamp(headingError * 2.0f, -1.0f, 1.0f);

    // --- Nitro : nez aligne, reserve suffisante, et assez de marge de
    // freinage devant (vLimit haut = portion a faible courbure) pour
    // absorber le surplus de vitesse. Les pilotes faibles gardent une
    // reserve plus haute (nitro rare) et exigent moins de marge (mal place).
    float nitroMargin = 6.0f + 8.0f * skill_;
    bool wantNitro =
        car.nitroRemaining > nitroReserve_ && turnSeverity < 0.15f && vLimit > speedAbs + nitroMargin;

    // --- Vitesse cible : min entre la limite de freinage anticipe (vLimit)
    // et la croisiere reduite si le nez n'est pas encore aligne sur la cible.
    // Le nitro releve la croisiere (sinon on couperait le boost des que la
    // vitesse depasse la cible sans nitro), jamais la limite de freinage.
    float alignFactor = std::clamp(1.0f - turnSeverity * 0.8f, 0.35f, 1.0f);
    float vCruise = car.tuning.maxSpeed * alignFactor * skill_;
    if (wantNitro) vCruise += car.tuning.nitroMaxSpeedBonus;
    float vTarget = std::min(vCruise, vLimit);

    if (car.speed > vLimit * 1.08f) {
        input.throttle = -0.75f; // freine AVANT le virage, pas dedans
    } else if (speedAbs < vTarget) {
        input.throttle = 1.0f;   // sous la cible (sortie de virage incluse) : plein gaz
    } else {
        input.throttle = 0.15f;  // proche de la cible : laisse filer
    }

    // Frein a main reserve aux vrais tete-a-queue a rattraper : en usage
    // courant il fait perdre la trajectoire (grip drift tres bas).
    input.handbrake = turnSeverity > 1.2f && speedAbs > 6.0f;

    // Nitro effectif seulement a plein gaz (pas en frein ni en roue libre).
    input.nitro = wantNitro && input.throttle >= 1.0f;

    return input;
}

} // namespace racer
