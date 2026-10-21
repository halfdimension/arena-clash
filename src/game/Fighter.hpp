#pragma once

#include "combat/Combat.hpp"

namespace arena {

enum class FighterId {
    Volt,
    Ember,
};

enum class FacingDirection {
    Left,
    Right,
};

struct Vector2f {
    float x;
    float y;
};

struct Fighter {
    FighterId id;
    Vector2f position;
    Vector2f velocity;
    float movementSpeed;
    float jumpVelocity;
    bool grounded;
    FacingDirection facing;
    float collisionWidth;
    float collisionHeight;
    int health;
    int maximumHealth;
    int energy;
    int maximumEnergy;
    CombatState combatState;
    MoveId currentMove;
    AttackPhase attackPhase;
    int combatStateFrame;
    int hitStunFrames;
    int chargeFrames;
    ChargeReleaseOutcome chargeReleaseOutcome;
    bool attackHasConnected;
    bool moveEffectCreated;
    int hitFlashFrames;
    int blockFlashFrames;
    MoveId specialImpactMove;
    int specialImpactFrames;
};

}  // namespace arena
