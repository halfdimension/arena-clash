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
constexpr int kSpecialImpactDuration = 8;
constexpr float kVoltStartingPositionX = 360.0F;
constexpr float kEmberStartingPositionX = 920.0F;

struct PendingHit {
    bool valid = false;
    bool blocked = false;
    std::size_t attackerIndex = 0;
    std::size_t defenderIndex = 0;
    const MoveDefinition* move = nullptr;
    std::size_t projectileIndex = kMaximumProjectiles;
    float impactOriginX = 0.0F;
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
        false,
        0,
        0,
        MoveId::None,
        0,
    };
}

void ResetFightersAndProjectiles(GameState& state, const MatchRules& matchRules) {
    state.fighters = {{MakeFighter(FighterId::Volt, kVoltStartingPositionX,
                                   kVoltMovementSpeed, kVoltJumpVelocity,
                                   FacingDirection::Right),
                       MakeFighter(FighterId::Ember, kEmberStartingPositionX,
                                   kEmberMovementSpeed, kEmberJumpVelocity,
                                   FacingDirection::Left)}};
    state.projectiles = {};
    state.matchPhase = MatchPhase::RoundIntro;
    state.roundResult = RoundResult::None;
    state.roundEndReason = RoundEndReason::None;
    state.roundTimerTicksRemaining = matchRules.roundDurationTicks;
    state.phaseTicksRemaining = matchRules.roundIntroDurationTicks;
}

void StartNewMatch(GameState& state, const MatchRules& matchRules) {
    state.roundWins = {};
    state.matchWinner = RoundResult::None;
    state.roundNumber = 1;
    ResetFightersAndProjectiles(state, matchRules);
}

void FinishRound(GameState& state, const MatchRules& matchRules, RoundResult result,
                 RoundEndReason reason) {
    state.matchPhase = MatchPhase::RoundOver;
    state.roundResult = result;
    state.roundEndReason = reason;
    state.phaseTicksRemaining = matchRules.roundOverDurationTicks;
    state.projectiles = {};

    if (result == RoundResult::VoltWins) {
        ++state.roundWins[0];
    } else if (result == RoundResult::EmberWins) {
        ++state.roundWins[1];
    }
}

RoundResult ResultFromRemainingHealth(const GameState& state) {
    if (state.fighters[0].health > state.fighters[1].health) {
        return RoundResult::VoltWins;
    }
    if (state.fighters[1].health > state.fighters[0].health) {
        return RoundResult::EmberWins;
    }
    return RoundResult::Draw;
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
    fighter.chargeFrames = 0;
    fighter.attackHasConnected = false;
    fighter.moveEffectCreated = false;
}

void StartAttack(Fighter& fighter, MoveId moveId) {
    fighter.combatState = CombatState::Attacking;
    fighter.currentMove = moveId;
    fighter.attackPhase = AttackPhase::Startup;
    fighter.combatStateFrame = 0;
    fighter.attackHasConnected = false;
    fighter.moveEffectCreated = false;
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
    fighter.moveEffectCreated = false;
    fighter.velocity.x = 0.0F;
}

void FinishCharging(Fighter& fighter) {
    const int minimumChargeFrames = GetCombatRules().minimumChargeFrames;
    if (fighter.chargeFrames < minimumChargeFrames) {
        fighter.chargeReleaseOutcome = ChargeReleaseOutcome::Cancelled;
        SetAvailable(fighter);
        return;
    }

    const MoveId chargedMove = GetChargedMoveId(fighter, fighter.chargeFrames);
    fighter.chargeReleaseOutcome = ChargeReleaseOutcome::Ready;
    fighter.chargeFrames = 0;
    StartAttack(fighter, chargedMove);
}

void PrepareCombatState(Fighter& fighter, const FighterCommand& command) {
    fighter.chargeReleaseOutcome = ChargeReleaseOutcome::None;
    fighter.hitFlashFrames = std::max(0, fighter.hitFlashFrames - 1);
    fighter.blockFlashFrames = std::max(0, fighter.blockFlashFrames - 1);
    fighter.specialImpactFrames = std::max(0, fighter.specialImpactFrames - 1);
    if (fighter.specialImpactFrames == 0) {
        fighter.specialImpactMove = MoveId::None;
    }

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
    } else if (command.specialAttackPressed && CanActivateSpecial(fighter)) {
        if (SpendEnergyForSpecial(fighter)) {
            StartAttack(fighter, GetSpecialMoveId(fighter));
        }
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

std::size_t FighterIndex(FighterId fighterId) {
    return fighterId == FighterId::Volt ? 0U : 1U;
}

Rectanglef MakeProjectileHitbox(const Projectile& projectile) {
    const MoveDefinition& move = GetMoveDefinition(projectile.sourceMove);
    return {
        projectile.position.x - move.hitbox.width * 0.5F,
        projectile.position.y - move.hitbox.height * 0.5F,
        move.hitbox.width,
        move.hitbox.height,
    };
}

void AdvanceProjectiles(GameState& state) {
    for (Projectile& projectile : state.projectiles) {
        if (!projectile.active) {
            continue;
        }
        projectile.position.x +=
            projectile.velocity.x * static_cast<float>(kFixedDeltaSeconds);
        projectile.position.y +=
            projectile.velocity.y * static_cast<float>(kFixedDeltaSeconds);
        projectile.hitbox = MakeProjectileHitbox(projectile);
        if (projectile.hitbox.x + projectile.hitbox.width < kArenaLeftBoundary ||
            projectile.hitbox.x > kArenaRightBoundary) {
            projectile.active = false;
        }
    }
}

void SpawnActiveProjectiles(GameState& state) {
    for (Fighter& fighter : state.fighters) {
        if (fighter.combatState != CombatState::Attacking ||
            fighter.attackPhase != AttackPhase::Active || fighter.moveEffectCreated) {
            continue;
        }

        const MoveDefinition& move = GetMoveDefinition(fighter.currentMove);
        if (move.behavior != MoveBehavior::Projectile) {
            continue;
        }

        fighter.moveEffectCreated = true;
        for (Projectile& projectile : state.projectiles) {
            if (projectile.active) {
                continue;
            }

            const float direction = fighter.facing == FacingDirection::Right ? 1.0F : -1.0F;
            const float bodyEdge = fighter.collisionWidth * 0.5F + move.hitbox.forwardOffset;
            projectile.active = true;
            projectile.owner = fighter.id;
            projectile.sourceMove = move.id;
            projectile.direction = fighter.facing;
            projectile.position = {
                fighter.position.x + direction * (bodyEdge + move.hitbox.width * 0.5F),
                fighter.position.y - move.hitbox.bottomOffset - move.hitbox.height * 0.5F,
            };
            projectile.velocity = {direction * move.projectileSpeed, 0.0F};
            projectile.remainingLifetimeFrames = move.projectileLifetimeFrames;
            projectile.hitbox = MakeProjectileHitbox(projectile);
            break;
        }
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
    if (move.behavior == MoveBehavior::Projectile) {
        return {};
    }
    if (!RectanglesOverlap(GetAttackHitbox(attacker, move), GetHurtbox(defender))) {
        return {};
    }

    const bool blocked = defender.combatState == CombatState::Blocking && defender.grounded &&
                         IsAttackInFrontOfDefender(attacker, defender);
    return {true, blocked, attackerIndex, defenderIndex, &move, kMaximumProjectiles,
            attacker.position.x};
}

PendingHit FindProjectilePendingHit(const GameState& state, std::size_t projectileIndex) {
    const Projectile& projectile = state.projectiles[projectileIndex];
    if (!projectile.active) {
        return {};
    }

    const std::size_t attackerIndex = FighterIndex(projectile.owner);
    const std::size_t defenderIndex = 1U - attackerIndex;
    const Fighter& defender = state.fighters[defenderIndex];
    if (!RectanglesOverlap(projectile.hitbox, GetHurtbox(defender))) {
        return {};
    }

    const bool blocked = defender.combatState == CombatState::Blocking && defender.grounded &&
                         IsPositionInFrontOfDefender(projectile.position.x, defender);
    return {true, blocked, attackerIndex, defenderIndex,
            &GetMoveDefinition(projectile.sourceMove), projectileIndex,
            projectile.position.x};
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
    if (moveId == MoveId::Light) {
        return combatRules.lightHitEnergyGain;
    }
    if (moveId == MoveId::Heavy) {
        return combatRules.heavyHitEnergyGain;
    }
    return 0;
}

void AgeProjectiles(GameState& state) {
    for (Projectile& projectile : state.projectiles) {
        if (!projectile.active) {
            continue;
        }
        --projectile.remainingLifetimeFrames;
        if (projectile.remainingLifetimeFrames <= 0) {
            projectile.active = false;
        }
    }
}

void ApplyPendingHit(GameState& state, const PendingHit& hit) {
    if (!hit.valid) {
        return;
    }

    Fighter& attacker = state.fighters[hit.attackerIndex];
    Fighter& defender = state.fighters[hit.defenderIndex];
    const int hitEnergyGain = EnergyGainForHit(hit.move->id);
    const bool hasDistinctImpact = hit.move->energyCost > 0 || IsChargedMove(hit.move->id);

    if (hit.blocked) {
        if (hitEnergyGain > 0) {
            const CombatRules& combatRules = GetCombatRules();
            GainEnergy(attacker, combatRules.blockedAttackerEnergyGain);
            GainEnergy(defender, combatRules.blockedDefenderEnergyGain);
        }
        defender.blockFlashFrames = kBlockFlashDuration;
        if (hasDistinctImpact) {
            defender.specialImpactMove = hit.move->id;
            defender.specialImpactFrames = kSpecialImpactDuration;
        }
        return;
    }

    GainEnergy(attacker, hitEnergyGain);
    defender.health = std::max(0, defender.health - hit.move->damage);
    defender.hitFlashFrames = kHitFlashDuration;
    if (hasDistinctImpact) {
        defender.specialImpactMove = hit.move->id;
        defender.specialImpactFrames = kSpecialImpactDuration;
    }
    const float knockbackDirection = hit.impactOriginX <= defender.position.x ? 1.0F : -1.0F;
    defender.velocity.x = hit.move->knockbackStrength * knockbackDirection;
    if (defender.combatState == CombatState::Charging) {
        defender.chargeReleaseOutcome = ChargeReleaseOutcome::Interrupted;
        defender.chargeFrames = 0;
    }
    defender.currentMove = MoveId::None;
    defender.attackPhase = AttackPhase::None;
    defender.attackHasConnected = false;
    defender.moveEffectCreated = false;
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

Game::Game(MatchRules matchRules)
    : matchRules_{std::max(1, matchRules.roundDurationTicks),
                  std::max(0, matchRules.roundIntroDurationTicks),
                  std::max(0, matchRules.roundOverDurationTicks)} {
}

void Game::initialize() {
    state_ = {};
    StartNewMatch(state_, matchRules_);
}

void Game::advanceSimulation(const FrameCommands& commands) {
    if (state_.matchPhase == MatchPhase::RoundIntro) {
        state_.phaseTicksRemaining = std::max(0, state_.phaseTicksRemaining - 1);
        if (state_.phaseTicksRemaining == 0) {
            state_.matchPhase = MatchPhase::Fighting;
        }
        ++state_.simulationFrame;
        return;
    }

    if (state_.matchPhase == MatchPhase::RoundOver) {
        state_.phaseTicksRemaining = std::max(0, state_.phaseTicksRemaining - 1);
        if (state_.phaseTicksRemaining == 0) {
            if (state_.roundWins[0] >= kRoundWinsRequired) {
                state_.matchPhase = MatchPhase::MatchOver;
                state_.matchWinner = RoundResult::VoltWins;
            } else if (state_.roundWins[1] >= kRoundWinsRequired) {
                state_.matchPhase = MatchPhase::MatchOver;
                state_.matchWinner = RoundResult::EmberWins;
            } else {
                ++state_.roundNumber;
                ResetFightersAndProjectiles(state_, matchRules_);
            }
        }
        ++state_.simulationFrame;
        return;
    }

    if (state_.matchPhase == MatchPhase::MatchOver) {
        if (commands.restartPressed) {
            const std::uint64_t nextSimulationFrame = state_.simulationFrame + 1;
            StartNewMatch(state_, matchRules_);
            state_.simulationFrame = nextSimulationFrame;
        } else {
            ++state_.simulationFrame;
        }
        return;
    }

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
    AdvanceProjectiles(state_);
    SpawnActiveProjectiles(state_);

    // Snapshot every fighter and projectile contact before applying any outcome.
    std::array<PendingHit, 2 + kMaximumProjectiles> pendingHits{};
    pendingHits[0] = FindPendingHit(state_, 0);
    pendingHits[1] = FindPendingHit(state_, 1);
    for (std::size_t projectileIndex = 0; projectileIndex < state_.projectiles.size();
         ++projectileIndex) {
        pendingHits[2 + projectileIndex] =
            FindProjectilePendingHit(state_, projectileIndex);
    }

    for (const PendingHit& hit : pendingHits) {
        if (hit.valid) {
            if (hit.projectileIndex < state_.projectiles.size()) {
                state_.projectiles[hit.projectileIndex].active = false;
            } else {
                state_.fighters[hit.attackerIndex].attackHasConnected = true;
            }
        }
    }

    AdvanceCombatFrame(state_.fighters[0]);
    AdvanceCombatFrame(state_.fighters[1]);
    for (const PendingHit& hit : pendingHits) {
        ApplyPendingHit(state_, hit);
    }
    AgeProjectiles(state_);

    state_.roundTimerTicksRemaining =
        std::max(0, state_.roundTimerTicksRemaining - 1);
    const bool voltKnockedOut = state_.fighters[0].health == 0;
    const bool emberKnockedOut = state_.fighters[1].health == 0;
    if (voltKnockedOut || emberKnockedOut) {
        if (voltKnockedOut && emberKnockedOut) {
            FinishRound(state_, matchRules_, RoundResult::Draw,
                        RoundEndReason::DoubleKnockout);
        } else {
            FinishRound(state_, matchRules_,
                        voltKnockedOut ? RoundResult::EmberWins
                                      : RoundResult::VoltWins,
                        RoundEndReason::Knockout);
        }
    } else if (state_.roundTimerTicksRemaining == 0) {
        FinishRound(state_, matchRules_, ResultFromRemainingHealth(state_),
                    RoundEndReason::TimeOver);
    }
    ++state_.simulationFrame;
}

const GameState& Game::state() const {
    return state_;
}

}  // namespace arena
