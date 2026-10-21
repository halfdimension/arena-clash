#include "combat/Combat.hpp"

#include "game/Fighter.hpp"

#include <algorithm>
#include <cassert>

namespace arena {
namespace {

constexpr int kSpecialEnergyCost = 25;

constexpr MoveDefinition kLightMove = {
    MoveId::Light,
    MoveOwner::Any,
    MoveBehavior::Melee,
    0,
    6,
    5,
    3,
    10,
    {52.0F, 44.0F, 4.0F, 42.0F},
    12,
    300.0F,
    0.0F,
    0,
};

constexpr MoveDefinition kHeavyMove = {
    MoveId::Heavy,
    MoveOwner::Any,
    MoveBehavior::Melee,
    0,
    14,
    12,
    4,
    17,
    {72.0F, 60.0F, 6.0F, 26.0F},
    20,
    480.0F,
    0.0F,
    0,
};

constexpr MoveDefinition kThunderBoltMove = {
    MoveId::ThunderBolt,
    MoveOwner::Volt,
    MoveBehavior::Projectile,
    kSpecialEnergyCost,
    16,
    12,
    1,
    20,
    {34.0F, 22.0F, 18.0F, 63.0F},
    18,
    420.0F,
    650.0F,
    90,
};

constexpr MoveDefinition kInfernoBurstMove = {
    MoveId::InfernoBurst,
    MoveOwner::Ember,
    MoveBehavior::Melee,
    kSpecialEnergyCost,
    20,
    16,
    5,
    24,
    {110.0F, 96.0F, -16.0F, 8.0F},
    24,
    620.0F,
    0.0F,
    0,
};

constexpr CombatRules kCombatRules = {
    100,
    0,
    8,
    12,
    4,
    6,
    kSpecialEnergyCost,
    18,
    72,
};

}  // namespace

const MoveDefinition& GetMoveDefinition(MoveId moveId) {
    assert(moveId != MoveId::None);
    switch (moveId) {
        case MoveId::Light:
            return kLightMove;
        case MoveId::Heavy:
            return kHeavyMove;
        case MoveId::ThunderBolt:
            return kThunderBoltMove;
        case MoveId::InfernoBurst:
            return kInfernoBurstMove;
        case MoveId::None:
            break;
    }
    return kLightMove;
}

MoveId GetSpecialMoveId(const Fighter& fighter) {
    return fighter.id == FighterId::Volt ? MoveId::ThunderBolt : MoveId::InfernoBurst;
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
    const MoveDefinition& move = GetMoveDefinition(GetSpecialMoveId(fighter));
    return fighter.combatState == CombatState::Available &&
           fighter.energy >= move.energyCost;
}

bool SpendEnergyForSpecial(Fighter& fighter) {
    if (!CanActivateSpecial(fighter)) {
        return false;
    }
    fighter.energy -= GetMoveDefinition(GetSpecialMoveId(fighter)).energyCost;
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
    return IsPositionInFrontOfDefender(attacker.position.x, defender);
}

bool IsPositionInFrontOfDefender(float positionX, const Fighter& defender) {
    if (defender.facing == FacingDirection::Right) {
        return positionX >= defender.position.x;
    }
    return positionX <= defender.position.x;
}

}  // namespace arena
