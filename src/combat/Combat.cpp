#include "combat/Combat.hpp"

#include "game/Fighter.hpp"

#include <algorithm>
#include <cassert>

namespace arena {
namespace {

constexpr MoveDefinition kLightMove = {
    MoveId::Light,
    6,
    5,
    3,
    10,
    {52.0F, 44.0F, 4.0F, 42.0F},
    12,
    300.0F,
};

constexpr MoveDefinition kHeavyMove = {
    MoveId::Heavy,
    14,
    12,
    4,
    17,
    {72.0F, 60.0F, 6.0F, 26.0F},
    20,
    480.0F,
};

constexpr CombatRules kCombatRules = {
    100,
    0,
    8,
    12,
    4,
    6,
    25,
    18,
    72,
};

}  // namespace

const MoveDefinition& GetMoveDefinition(MoveId moveId) {
    assert(moveId != MoveId::None);
    return moveId == MoveId::Heavy ? kHeavyMove : kLightMove;
}

const CombatRules& GetCombatRules() {
    return kCombatRules;
}

void GainEnergy(Fighter& fighter, int amount) {
    if (amount <= 0) {
        return;
    }
    fighter.energy = std::clamp(fighter.energy + amount, 0, fighter.maximumEnergy);
}

bool CanActivateSpecial(const Fighter& fighter) {
    return fighter.combatState == CombatState::Available &&
           fighter.energy >= kCombatRules.specialEnergyCost;
}

bool SpendEnergyForSpecial(Fighter& fighter) {
    if (!CanActivateSpecial(fighter)) {
        return false;
    }
    fighter.energy -= kCombatRules.specialEnergyCost;
    return true;
}

Rectanglef GetHurtbox(const Fighter& fighter) {
    return {
        fighter.position.x - fighter.collisionWidth * 0.5F,
        fighter.position.y - fighter.collisionHeight,
        fighter.collisionWidth,
        fighter.collisionHeight,
    };
}

Rectanglef GetAttackHitbox(const Fighter& fighter, const MoveDefinition& move) {
    const float bodyEdge = fighter.collisionWidth * 0.5F + move.hitbox.forwardOffset;
    const float x = fighter.facing == FacingDirection::Right
                        ? fighter.position.x + bodyEdge
                        : fighter.position.x - bodyEdge - move.hitbox.width;
    return {
        x,
        fighter.position.y - move.hitbox.bottomOffset - move.hitbox.height,
        move.hitbox.width,
        move.hitbox.height,
    };
}

bool RectanglesOverlap(const Rectanglef& first, const Rectanglef& second) {
    return first.x < second.x + second.width && first.x + first.width > second.x &&
           first.y < second.y + second.height && first.y + first.height > second.y;
}

bool IsAttackInFrontOfDefender(const Fighter& attacker, const Fighter& defender) {
    if (defender.facing == FacingDirection::Right) {
        return attacker.position.x >= defender.position.x;
    }
    return attacker.position.x <= defender.position.x;
}

}  // namespace arena
