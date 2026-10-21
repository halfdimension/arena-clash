#include "core/Game.hpp"

#include "combat/Combat.hpp"

#include <algorithm>
#include <array>
#include <cstddef>

namespace arena {
namespace {

constexpr float kVoltMovementSpeed = 360.0F;
constexpr float kVoltJumpVelocity = 780.0F;
constexpr float kEmberMovementSpeed = 300.0F;
constexpr float kEmberJumpVelocity = 740.0F;
constexpr float kFighterCollisionWidth = 64.0F;
constexpr float kFighterCollisionHeight = 112.0F;
constexpr float kGroundKnockbackFriction = 0.82F;
constexpr int kHitFlashDuration = 5;
constexpr int kBlockFlashDuration = 4;

struct PendingHit {
    bool valid = false;
    bool blocked = false;
    std::size_t attackerIndex = 0;
    std::size_t defenderIndex = 0;
    const MoveDefinition* move = nullptr;
};

Fighter MakeFighter(FighterId id, float positionX, float movementSpeed, float jumpVelocity,
                    FacingDirection facing) {
    const CombatRules& combatRules = GetCombatRules();
    return {
        id,
        {positionX, kArenaGroundY},
        {0.0F, 0.0F},
        movementSpeed,
        jumpVelocity,
        true,
        facing,
        kFighterCollisionWidth,
        kFighterCollisionHeight,
        100,
        100,
        combatRules.initialEnergy,
        combatRules.maximumEnergy,
        CombatState::Available,
        MoveId::None,
        AttackPhase::None,
        0,
        0,
        0,
        ChargeReleaseOutcome::None,
        false,
        0,
        0,
    };
}

float MinimumPositionX(const Fighter& fighter) {
    return kArenaLeftBoundary + fighter.collisionWidth * 0.5F;
}

float MaximumPositionX(const Fighter& fighter) {
    return kArenaRightBoundary - fighter.collisionWidth * 0.5F;
}

int HorizontalAxis(const FighterCommand& command) {
    return static_cast<int>(command.moveRightHeld) - static_cast<int>(command.moveLeftHeld);
}

void SetAvailable(Fighter& fighter) {
    fighter.combatState = CombatState::Available;
    fighter.currentMove = MoveId::None;
    fighter.attackPhase = AttackPhase::None;
    fighter.combatStateFrame = 0;
    fighter.hitStunFrames = 0;
    fighter.attackHasConnected = false;
}

void StartAttack(Fighter& fighter, MoveId moveId) {
    fighter.combatState = CombatState::Attacking;
    fighter.currentMove = moveId;
    fighter.attackPhase = AttackPhase::Startup;
    fighter.combatStateFrame = 0;
    fighter.attackHasConnected = false;
    if (fighter.grounded) {
        fighter.velocity.x = 0.0F;
    }
}

void StartCharging(Fighter& fighter) {
    fighter.combatState = CombatState::Charging;
    fighter.currentMove = MoveId::None;
    fighter.attackPhase = AttackPhase::None;
    fighter.combatStateFrame = 0;
    fighter.chargeFrames = 0;
    fighter.chargeReleaseOutcome = ChargeReleaseOutcome::None;
    fighter.attackHasConnected = false;
    fighter.velocity.x = 0.0F;
}

void FinishCharging(Fighter& fighter) {
    const int minimumChargeFrames = GetCombatRules().minimumChargeFrames;
    fighter.chargeReleaseOutcome = fighter.chargeFrames >= minimumChargeFrames
                                       ? ChargeReleaseOutcome::Ready
                                       : ChargeReleaseOutcome::Cancelled;
    SetAvailable(fighter);
}

void PrepareCombatState(Fighter& fighter, const FighterCommand& command) {
    fighter.chargeReleaseOutcome = ChargeReleaseOutcome::None;
    fighter.hitFlashFrames = std::max(0, fighter.hitFlashFrames - 1);
    fighter.blockFlashFrames = std::max(0, fighter.blockFlashFrames - 1);

    if (fighter.combatState == CombatState::HitStun &&
        fighter.combatStateFrame >= fighter.hitStunFrames) {
        SetAvailable(fighter);
    }

    if (fighter.combatState == CombatState::Attacking) {
        const MoveDefinition& move = GetMoveDefinition(fighter.currentMove);
        if (fighter.attackPhase == AttackPhase::Startup &&
            fighter.combatStateFrame >= move.startupFrames) {
            fighter.attackPhase = AttackPhase::Active;
            fighter.combatStateFrame = 0;
        } else if (fighter.attackPhase == AttackPhase::Active &&
                   fighter.combatStateFrame >= move.activeFrames) {
            fighter.attackPhase = AttackPhase::Recovery;
            fighter.combatStateFrame = 0;
        } else if (fighter.attackPhase == AttackPhase::Recovery &&
                   fighter.combatStateFrame >= move.recoveryFrames) {
            SetAvailable(fighter);
        }
    }

    if (fighter.combatState == CombatState::Blocking &&
        (!command.blockHeld || !fighter.grounded)) {
        SetAvailable(fighter);
    }

    if (fighter.combatState == CombatState::Charging) {
        if (command.chargedAttackReleased || !command.chargedAttackHeld) {
            FinishCharging(fighter);
        }
        return;
    }

    if (fighter.combatState != CombatState::Available) {
        return;
    }

    // A grounded held block takes priority over attack presses on the same tick.
    if (command.blockHeld && fighter.grounded) {
        fighter.combatState = CombatState::Blocking;
        fighter.velocity.x = 0.0F;
    } else if (command.lightAttackPressed) {
        StartAttack(fighter, MoveId::Light);
    } else if (command.heavyAttackPressed) {
        StartAttack(fighter, MoveId::Heavy);
    } else if (command.chargedAttackPressed && fighter.grounded) {
        StartCharging(fighter);
        if (command.chargedAttackReleased) {
            FinishCharging(fighter);
        }
    }
}

void ConstrainToArena(Fighter& fighter) {
    const float unconstrainedX = fighter.position.x;
    fighter.position.x = std::clamp(
        fighter.position.x, MinimumPositionX(fighter), MaximumPositionX(fighter));

    if (fighter.position.x != unconstrainedX) {
        fighter.velocity.x = 0.0F;
    }
}

void AdvanceFighter(Fighter& fighter, const FighterCommand& command) {
    if (fighter.combatState == CombatState::Available) {
        const int horizontalAxis = HorizontalAxis(command);
        const float controlFactor = fighter.grounded ? 1.0F : kAirControlFactor;
        fighter.velocity.x =
            static_cast<float>(horizontalAxis) * fighter.movementSpeed * controlFactor;

        if (command.jumpPressed && fighter.grounded) {
            fighter.velocity.y = -fighter.jumpVelocity;
            fighter.grounded = false;
        }
    } else if ((fighter.combatState == CombatState::Attacking ||
                fighter.combatState == CombatState::Blocking ||
                fighter.combatState == CombatState::Charging) &&
               fighter.grounded) {
        fighter.velocity.x = 0.0F;
    }

    fighter.position.x += fighter.velocity.x * static_cast<float>(kFixedDeltaSeconds);
    ConstrainToArena(fighter);

    if ((fighter.combatState == CombatState::HitStun ||
         fighter.combatState == CombatState::KnockedOut) &&
        fighter.grounded) {
        fighter.velocity.x *= kGroundKnockbackFriction;
        if (fighter.velocity.x > -1.0F && fighter.velocity.x < 1.0F) {
            fighter.velocity.x = 0.0F;
        }
    }

    if (!fighter.grounded) {
        fighter.velocity.y += kGravity * static_cast<float>(kFixedDeltaSeconds);
        fighter.position.y += fighter.velocity.y * static_cast<float>(kFixedDeltaSeconds);

        if (fighter.position.y >= kArenaGroundY) {
            fighter.position.y = kArenaGroundY;
            fighter.velocity.y = 0.0F;
            fighter.grounded = true;
        }
    } else {
        fighter.position.y = kArenaGroundY;
        fighter.velocity.y = 0.0F;
    }
}

void ResolveFighterCollision(GameState& state, const std::array<float, 2>& previousPositionsX) {
    Fighter& first = state.fighters[0];
    Fighter& second = state.fighters[1];
    const bool verticallySeparated =
        first.position.y <= second.position.y - second.collisionHeight ||
        second.position.y <= first.position.y - first.collisionHeight;
    if (verticallySeparated) {
        return;
    }

    const bool firstWasLeft = previousPositionsX[0] <= previousPositionsX[1];
    Fighter& left = firstWasLeft ? first : second;
    Fighter& right = firstWasLeft ? second : first;
    const float minimumSeparation = (left.collisionWidth + right.collisionWidth) * 0.5F;
    float overlap = minimumSeparation - (right.position.x - left.position.x);
    if (overlap <= 0.0F) {
        return;
    }

    left.position.x -= overlap * 0.5F;
    right.position.x += overlap * 0.5F;
    ConstrainToArena(left);
    ConstrainToArena(right);

    // If one fighter is against a wall, move the other by the unresolved remainder.
    overlap = minimumSeparation - (right.position.x - left.position.x);
    if (overlap > 0.0F) {
        const float rightMovement = std::min(overlap, MaximumPositionX(right) - right.position.x);
        right.position.x += rightMovement;
        overlap -= rightMovement;
        left.position.x = std::max(MinimumPositionX(left), left.position.x - overlap);
    }

    if (left.velocity.x > 0.0F) {
        left.velocity.x = 0.0F;
    }
    if (right.velocity.x < 0.0F) {
        right.velocity.x = 0.0F;
    }
}

void UpdateFacingDirections(GameState& state) {
    Fighter& first = state.fighters[0];
    Fighter& second = state.fighters[1];
    if (first.position.x < second.position.x) {
        first.facing = FacingDirection::Right;
        second.facing = FacingDirection::Left;
    } else if (first.position.x > second.position.x) {
        first.facing = FacingDirection::Left;
        second.facing = FacingDirection::Right;
    }
}

PendingHit FindPendingHit(const GameState& state, std::size_t attackerIndex) {
    const std::size_t defenderIndex = 1U - attackerIndex;
    const Fighter& attacker = state.fighters[attackerIndex];
    const Fighter& defender = state.fighters[defenderIndex];
    if (attacker.combatState != CombatState::Attacking ||
        attacker.attackPhase != AttackPhase::Active || attacker.attackHasConnected) {
        return {};
    }

    const MoveDefinition& move = GetMoveDefinition(attacker.currentMove);
    if (!RectanglesOverlap(GetAttackHitbox(attacker, move), GetHurtbox(defender))) {
        return {};
    }

    const bool blocked = defender.combatState == CombatState::Blocking && defender.grounded &&
                         IsAttackInFrontOfDefender(attacker, defender);
    return {true, blocked, attackerIndex, defenderIndex, &move};
}

void AdvanceCombatFrame(Fighter& fighter) {
    if (fighter.combatState == CombatState::Attacking ||
        fighter.combatState == CombatState::HitStun) {
        ++fighter.combatStateFrame;
    }
    if (fighter.combatState == CombatState::Charging) {
        fighter.chargeFrames =
            std::min(fighter.chargeFrames + 1, GetCombatRules().maximumChargeFrames);
    }
}

int EnergyGainForHit(MoveId moveId) {
    const CombatRules& combatRules = GetCombatRules();
    return moveId == MoveId::Heavy ? combatRules.heavyHitEnergyGain
                                   : combatRules.lightHitEnergyGain;
}

void ApplyPendingHit(GameState& state, const PendingHit& hit) {
    if (!hit.valid) {
        return;
    }

    Fighter& attacker = state.fighters[hit.attackerIndex];
    Fighter& defender = state.fighters[hit.defenderIndex];

    if (hit.blocked) {
        const CombatRules& combatRules = GetCombatRules();
        GainEnergy(attacker, combatRules.blockedAttackerEnergyGain);
        GainEnergy(defender, combatRules.blockedDefenderEnergyGain);
        defender.blockFlashFrames = kBlockFlashDuration;
        return;
    }

    GainEnergy(attacker, EnergyGainForHit(hit.move->id));
    defender.health = std::max(0, defender.health - hit.move->damage);
    defender.hitFlashFrames = kHitFlashDuration;
    const float knockbackDirection =
        attacker.position.x <= defender.position.x ? 1.0F : -1.0F;
    defender.velocity.x = hit.move->knockbackStrength * knockbackDirection;
    if (defender.combatState == CombatState::Charging) {
        defender.chargeReleaseOutcome = ChargeReleaseOutcome::Interrupted;
    }
    defender.currentMove = MoveId::None;
    defender.attackPhase = AttackPhase::None;
    defender.attackHasConnected = false;
    defender.combatStateFrame = 0;

    if (defender.health == 0) {
        defender.combatState = CombatState::KnockedOut;
        defender.hitStunFrames = 0;
    } else {
        defender.combatState = CombatState::HitStun;
        defender.hitStunFrames = hit.move->hitStunFrames;
    }
}

}  // namespace

void Game::initialize() {
    state_ = {
        {{MakeFighter(FighterId::Volt, 360.0F, kVoltMovementSpeed, kVoltJumpVelocity,
                      FacingDirection::Right),
          MakeFighter(FighterId::Ember, 920.0F, kEmberMovementSpeed, kEmberJumpVelocity,
                      FacingDirection::Left)}},
        0,
    };
}

void Game::advanceSimulation(const FrameCommands& commands) {
    const std::array<float, 2> previousPositionsX = {
        state_.fighters[0].position.x,
        state_.fighters[1].position.x,
    };

    PrepareCombatState(state_.fighters[0], commands.fighters[0]);
    PrepareCombatState(state_.fighters[1], commands.fighters[1]);
    AdvanceFighter(state_.fighters[0], commands.fighters[0]);
    AdvanceFighter(state_.fighters[1], commands.fighters[1]);
    ResolveFighterCollision(state_, previousPositionsX);
    UpdateFacingDirections(state_);

    // Snapshot both hit results before applying either, so valid same-tick trades are fair.
    const std::array<PendingHit, 2> pendingHits = {
        FindPendingHit(state_, 0),
        FindPendingHit(state_, 1),
    };

    for (const PendingHit& hit : pendingHits) {
        if (hit.valid) {
            state_.fighters[hit.attackerIndex].attackHasConnected = true;
        }
    }

    AdvanceCombatFrame(state_.fighters[0]);
    AdvanceCombatFrame(state_.fighters[1]);
    ApplyPendingHit(state_, pendingHits[0]);
    ApplyPendingHit(state_, pendingHits[1]);
    ++state_.simulationFrame;
}

const GameState& Game::state() const {
    return state_;
}

}  // namespace arena
