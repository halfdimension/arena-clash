#pragma once

namespace arena {

struct Fighter;

enum class MoveId {
    None,
    Light,
    Heavy,
};

enum class CombatState {
    Available,
    Attacking,
    Blocking,
    Charging,
    HitStun,
    KnockedOut,
};

enum class ChargeReleaseOutcome {
    None,
    Cancelled,
    Ready,
    Interrupted,
};

enum class AttackPhase {
    None,
    Startup,
    Active,
    Recovery,
};

struct Rectanglef {
    float x;
    float y;
    float width;
    float height;
};

struct HitboxDefinition {
    float width;
    float height;
    float forwardOffset;
    float bottomOffset;
};

struct MoveDefinition {
    MoveId id;
    int damage;
    int startupFrames;
    int activeFrames;
    int recoveryFrames;
    HitboxDefinition hitbox;
    int hitStunFrames;
    float knockbackStrength;
};

struct CombatRules {
    int maximumEnergy;
    int initialEnergy;
    int lightHitEnergyGain;
    int heavyHitEnergyGain;
    int blockedAttackerEnergyGain;
    int blockedDefenderEnergyGain;
    int specialEnergyCost;
    int minimumChargeFrames;
    int maximumChargeFrames;
};

const MoveDefinition& GetMoveDefinition(MoveId moveId);
const CombatRules& GetCombatRules();
void GainEnergy(Fighter& fighter, int amount);
bool CanActivateSpecial(const Fighter& fighter);
bool SpendEnergyForSpecial(Fighter& fighter);
Rectanglef GetHurtbox(const Fighter& fighter);
Rectanglef GetAttackHitbox(const Fighter& fighter, const MoveDefinition& move);
bool RectanglesOverlap(const Rectanglef& first, const Rectanglef& second);
bool IsAttackInFrontOfDefender(const Fighter& attacker, const Fighter& defender);

}  // namespace arena
