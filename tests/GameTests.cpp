#include "combat/Combat.hpp"
#include "core/Game.hpp"
#include "input/FrameCommandBuffer.hpp"

#include <array>
#include <cmath>
#include <iostream>

namespace {

constexpr float kTolerance = 0.02F;

bool Expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        return false;
    }
    return true;
}

bool NearlyEqual(float first, float second) {
    return std::fabs(first - second) <= kTolerance;
}

const arena::Fighter& Volt(const arena::Game& game) {
    return game.state().fighters[0];
}

const arena::Fighter& Ember(const arena::Game& game) {
    return game.state().fighters[1];
}

void AdvanceEmpty(arena::Game& game, int tickCount) {
    for (int tick = 0; tick < tickCount; ++tick) {
        game.advanceSimulation({});
    }
}

void AdvanceToFighting(arena::Game& game) {
    for (int tick = 0; tick <= arena::kDefaultRoundIntroDurationTicks; ++tick) {
        if (game.state().matchPhase == arena::MatchPhase::Fighting) {
            return;
        }
        game.advanceSimulation({});
    }
}

void ApproachToBodyContact(arena::Game& game) {
    arena::FrameCommands commands;
    commands.fighters[0].moveRightHeld = true;
    commands.fighters[1].moveLeftHeld = true;
    for (int tick = 0; tick < arena::kSimulationRate; ++tick) {
        game.advanceSimulation(commands);
    }
}

void PressMove(arena::Game& game, std::size_t fighterIndex, arena::MoveId moveId,
               bool otherFighterBlocks = false) {
    arena::FrameCommands commands;
    if (moveId == arena::MoveId::Light) {
        commands.fighters[fighterIndex].lightAttackHeld = true;
        commands.fighters[fighterIndex].lightAttackPressed = true;
    } else {
        commands.fighters[fighterIndex].heavyAttackHeld = true;
        commands.fighters[fighterIndex].heavyAttackPressed = true;
    }
    commands.fighters[1U - fighterIndex].blockHeld = otherFighterBlocks;
    game.advanceSimulation(commands);
}

void AdvanceToFirstActiveFrame(arena::Game& game, std::size_t fighterIndex,
                               arena::MoveId moveId, bool otherFighterBlocks = false) {
    PressMove(game, fighterIndex, moveId, otherFighterBlocks);
    const arena::MoveDefinition& move = arena::GetMoveDefinition(moveId);
    arena::FrameCommands commands;
    commands.fighters[fighterIndex].lightAttackHeld = moveId == arena::MoveId::Light;
    commands.fighters[fighterIndex].heavyAttackHeld = moveId == arena::MoveId::Heavy;
    commands.fighters[1U - fighterIndex].blockHeld = otherFighterBlocks;
    for (int tick = 0; tick < move.startupFrames; ++tick) {
        game.advanceSimulation(commands);
    }
}

void AdvanceUntilBothAvailable(arena::Game& game) {
    for (int tick = 0; tick < 240; ++tick) {
        if (Volt(game).combatState == arena::CombatState::Available &&
            Ember(game).combatState == arena::CombatState::Available) {
            return;
        }
        game.advanceSimulation({});
    }
}

void EarnEnergy(arena::Game& game, std::size_t fighterIndex, int targetEnergy) {
    while (game.state().fighters[fighterIndex].energy < targetEnergy) {
        AdvanceUntilBothAvailable(game);
        ApproachToBodyContact(game);
        AdvanceToFirstActiveFrame(game, fighterIndex, arena::MoveId::Heavy);
    }
    AdvanceUntilBothAvailable(game);
}

void PressSpecial(arena::Game& game, std::size_t fighterIndex) {
    arena::FrameCommands commands;
    commands.fighters[fighterIndex].specialAttackHeld = true;
    commands.fighters[fighterIndex].specialAttackPressed = true;
    game.advanceSimulation(commands);
}

int ActiveProjectileCount(const arena::Game& game) {
    int count = 0;
    for (const arena::Projectile& projectile : game.state().projectiles) {
        count += projectile.active ? 1 : 0;
    }
    return count;
}

const arena::Projectile* FirstActiveProjectile(const arena::Game& game) {
    for (const arena::Projectile& projectile : game.state().projectiles) {
        if (projectile.active) {
            return &projectile;
        }
    }
    return nullptr;
}

bool CompleteStatesEqual(const arena::Game& first, const arena::Game& second);

bool TestInitialStateAndMoveData() {
    arena::Game game;
    game.initialize();
    AdvanceToFighting(game);
    const arena::MoveDefinition& light = arena::GetMoveDefinition(arena::MoveId::Light);
    const arena::MoveDefinition& heavy = arena::GetMoveDefinition(arena::MoveId::Heavy);
    const arena::MoveDefinition& thunderBolt =
        arena::GetMoveDefinition(arena::MoveId::ThunderBolt);
    const arena::MoveDefinition& infernoBurst =
        arena::GetMoveDefinition(arena::MoveId::InfernoBurst);
    const arena::CombatRules& combatRules = arena::GetCombatRules();

    bool passed = true;
    passed &= Expect(game.state().simulationFrame ==
                         static_cast<std::uint64_t>(arena::kDefaultRoundIntroDurationTicks) &&
                         game.state().matchPhase == arena::MatchPhase::Fighting,
                     "default round intro lasts its exact configured ticks");
    passed &= Expect(Volt(game).id == arena::FighterId::Volt, "player one is Volt");
    passed &= Expect(Ember(game).id == arena::FighterId::Ember, "player two is Ember");
    passed &= Expect(Volt(game).grounded && Ember(game).grounded, "fighters start grounded");
    passed &= Expect(Volt(game).facing == arena::FacingDirection::Right,
                     "Volt initially faces Ember");
    passed &= Expect(Ember(game).facing == arena::FacingDirection::Left,
                     "Ember initially faces Volt");
    passed &= Expect(Volt(game).health == 100 && Ember(game).health == 100,
                     "fighters start at 100 health");
    passed &= Expect(Volt(game).energy == 0 && Ember(game).energy == 0 &&
                         Volt(game).maximumEnergy == 100 && Ember(game).maximumEnergy == 100,
                     "fighters start with empty 100-point energy meters");
    passed &= Expect(Volt(game).combatState == arena::CombatState::Available &&
                         Ember(game).combatState == arena::CombatState::Available,
                     "fighters start combat-available");
    passed &= Expect(light.damage == 6 && light.startupFrames == 5 &&
                         light.activeFrames == 3 && light.recoveryFrames == 10,
                     "light move data matches Phase 1C values");
    passed &= Expect(heavy.damage == 14 && heavy.startupFrames == 12 &&
                         heavy.activeFrames == 4 && heavy.recoveryFrames == 17,
                     "heavy move data matches Phase 1C values");
    passed &= Expect(combatRules.lightHitEnergyGain == 8 &&
                         combatRules.heavyHitEnergyGain == 12 &&
                         combatRules.blockedAttackerEnergyGain == 4 &&
                         combatRules.blockedDefenderEnergyGain == 6 &&
                         combatRules.specialEnergyCost == 25 &&
                         combatRules.minimumChargeFrames == 18 &&
                         combatRules.chargeLevel2Frames == 36 &&
                         combatRules.chargeLevel3Frames == 60 &&
                         combatRules.maximumChargeFrames == 72,
                     "energy and charge tuning is centralized");
    passed &= Expect(thunderBolt.owner == arena::MoveOwner::Volt &&
                         thunderBolt.behavior == arena::MoveBehavior::Projectile &&
                         thunderBolt.energyCost == 25 && thunderBolt.damage == 16 &&
                         thunderBolt.startupFrames == 12 && thunderBolt.activeFrames == 1 &&
                         thunderBolt.recoveryFrames == 20 &&
                         NearlyEqual(thunderBolt.projectileSpeed, 650.0F) &&
                         thunderBolt.projectileLifetimeFrames == 90,
                     "Thunder Bolt data matches Phase 1D-B values");
    passed &= Expect(infernoBurst.owner == arena::MoveOwner::Ember &&
                         infernoBurst.behavior == arena::MoveBehavior::Melee &&
                         infernoBurst.energyCost == 25 && infernoBurst.damage == 20 &&
                         infernoBurst.startupFrames == 16 && infernoBurst.activeFrames == 5 &&
                         infernoBurst.recoveryFrames == 24 &&
                         infernoBurst.knockbackStrength > thunderBolt.knockbackStrength,
                     "Inferno Burst data matches Phase 1D-B values");
    return passed;
}

bool TestHorizontalMovementAndOpposingInput() {
    arena::Game game;
    game.initialize();
    AdvanceToFighting(game);
    const float initialVoltX = Volt(game).position.x;

    arena::FrameCommands commands;
    commands.fighters[0].moveRightHeld = true;
    for (int tick = 0; tick < arena::kSimulationRate; ++tick) {
        game.advanceSimulation(commands);
    }

    bool passed = true;
    passed &= Expect(NearlyEqual(Volt(game).position.x,
                                 initialVoltX + Volt(game).movementSpeed),
                     "one second of right input uses Volt's movement speed");

    const float positionBeforeOpposingInput = Volt(game).position.x;
    commands.fighters[0].moveLeftHeld = true;
    game.advanceSimulation(commands);
    passed &= Expect(NearlyEqual(Volt(game).position.x, positionBeforeOpposingInput),
                     "simultaneous opposing directions cancel");
    passed &= Expect(NearlyEqual(Volt(game).velocity.x, 0.0F),
                     "opposing directions produce no horizontal velocity");
    return passed;
}

bool TestJumpGravityLandingAndHeldJump() {
    arena::Game game;
    game.initialize();
    AdvanceToFighting(game);

    arena::FrameCommands commands;
    commands.fighters[0].jumpHeld = true;
    commands.fighters[0].jumpPressed = true;
    game.advanceSimulation(commands);

    bool passed = true;
    passed &= Expect(!Volt(game).grounded, "a fresh jump press leaves the ground");
    passed &= Expect(Volt(game).position.y < arena::kArenaGroundY,
                     "jump initiation moves upward");
    passed &= Expect(Volt(game).velocity.y < 0.0F, "jump initiation has upward velocity");

    const float velocityAfterJump = Volt(game).velocity.y;
    commands.fighters[0].jumpPressed = true;
    game.advanceSimulation(commands);
    passed &= Expect(Volt(game).velocity.y > velocityAfterJump,
                     "an airborne jump press cannot reset upward velocity");
    commands.fighters[0].jumpPressed = false;

    for (int tick = 0; tick < arena::kSimulationRate * 2; ++tick) {
        game.advanceSimulation(commands);
    }
    passed &= Expect(Volt(game).grounded, "fighter lands after a jump");
    passed &= Expect(NearlyEqual(Volt(game).position.y, arena::kArenaGroundY),
                     "landing snaps to the ground plane");
    passed &= Expect(NearlyEqual(Volt(game).velocity.y, 0.0F),
                     "landing clears vertical velocity");
    passed &= Expect(Volt(game).grounded,
                     "holding jump without another press does not automatically jump again");

    commands.fighters[0].jumpPressed = true;
    game.advanceSimulation(commands);
    passed &= Expect(!Volt(game).grounded, "a new jump press works after landing");
    return passed;
}

bool TestStageBoundaries() {
    arena::Game game;
    game.initialize();
    AdvanceToFighting(game);

    arena::FrameCommands commands;
    commands.fighters[0].moveLeftHeld = true;
    commands.fighters[1].moveRightHeld = true;
    for (int tick = 0; tick < arena::kSimulationRate * 5; ++tick) {
        game.advanceSimulation(commands);
    }

    const float voltMinimum = arena::kArenaLeftBoundary + Volt(game).collisionWidth * 0.5F;
    const float emberMaximum = arena::kArenaRightBoundary - Ember(game).collisionWidth * 0.5F;
    bool passed = true;
    passed &= Expect(NearlyEqual(Volt(game).position.x, voltMinimum),
                     "Volt cannot leave the left stage boundary");
    passed &= Expect(NearlyEqual(Ember(game).position.x, emberMaximum),
                     "Ember cannot leave the right stage boundary");
    return passed;
}

bool TestSimultaneousMovementAndCollision() {
    arena::Game game;
    game.initialize();
    AdvanceToFighting(game);

    arena::FrameCommands commands;
    commands.fighters[0].moveRightHeld = true;
    commands.fighters[1].moveLeftHeld = true;
    game.advanceSimulation(commands);

    bool passed = true;
    passed &= Expect(Volt(game).position.x > 360.0F, "Volt moves during simultaneous input");
    passed &= Expect(Ember(game).position.x < 920.0F, "Ember moves during simultaneous input");

    for (int tick = 0; tick < arena::kSimulationRate * 2; ++tick) {
        game.advanceSimulation(commands);
    }

    const float requiredSeparation =
        (Volt(game).collisionWidth + Ember(game).collisionWidth) * 0.5F;
    passed &= Expect(Volt(game).position.x < Ember(game).position.x,
                     "grounded fighters retain their left-right order");
    passed &= Expect(Ember(game).position.x - Volt(game).position.x + kTolerance >=
                         requiredSeparation,
                     "grounded fighter bodies do not overlap");
    passed &= Expect(NearlyEqual(Volt(game).velocity.x, 0.0F) &&
                         NearlyEqual(Ember(game).velocity.x, 0.0F),
                     "inward velocity stops at body contact");
    return passed;
}

bool TestFacingUpdatesAfterAirborneCrossOver() {
    arena::Game game;
    game.initialize();
    AdvanceToFighting(game);

    arena::FrameCommands approach;
    approach.fighters[0].moveRightHeld = true;
    approach.fighters[1].moveLeftHeld = true;
    for (int tick = 0; tick < arena::kSimulationRate; ++tick) {
        game.advanceSimulation(approach);
    }

    arena::FrameCommands crossOver;
    crossOver.fighters[0].moveRightHeld = true;
    crossOver.fighters[0].jumpPressed = true;
    crossOver.fighters[0].jumpHeld = true;
    game.advanceSimulation(crossOver);
    crossOver.fighters[0].jumpPressed = false;
    for (int tick = 0; tick < 30; ++tick) {
        game.advanceSimulation(crossOver);
    }

    bool passed = true;
    passed &= Expect(Volt(game).position.x > Ember(game).position.x,
                     "an airborne fighter can cross over the opponent");
    passed &= Expect(Volt(game).facing == arena::FacingDirection::Left,
                     "Volt turns toward Ember after crossing over");
    passed &= Expect(Ember(game).facing == arena::FacingDirection::Right,
                     "Ember turns toward Volt after crossing over");
    return passed;
}

bool TestAttackLifecycle(arena::MoveId moveId, const char* moveName) {
    arena::Game game;
    game.initialize();
    AdvanceToFighting(game);
    const arena::MoveDefinition& move = arena::GetMoveDefinition(moveId);

    PressMove(game, 0, moveId);
    bool passed = true;
    passed &= Expect(Volt(game).combatState == arena::CombatState::Attacking &&
                         Volt(game).attackPhase == arena::AttackPhase::Startup &&
                         Volt(game).combatStateFrame == 1,
                     moveName);

    AdvanceEmpty(game, move.startupFrames - 1);
    passed &= Expect(Volt(game).attackPhase == arena::AttackPhase::Startup &&
                         Volt(game).combatStateFrame == move.startupFrames,
                     "startup lasts its exact configured frame count");
    AdvanceEmpty(game, 1);
    passed &= Expect(Volt(game).attackPhase == arena::AttackPhase::Active &&
                         Volt(game).combatStateFrame == 1,
                     "active begins on the tick after startup completes");
    AdvanceEmpty(game, move.activeFrames - 1);
    passed &= Expect(Volt(game).attackPhase == arena::AttackPhase::Active &&
                         Volt(game).combatStateFrame == move.activeFrames,
                     "active lasts its exact configured frame count");
    AdvanceEmpty(game, 1);
    passed &= Expect(Volt(game).attackPhase == arena::AttackPhase::Recovery &&
                         Volt(game).combatStateFrame == 1,
                     "recovery begins on the tick after active completes");
    AdvanceEmpty(game, move.recoveryFrames - 1);
    passed &= Expect(Volt(game).attackPhase == arena::AttackPhase::Recovery &&
                         Volt(game).combatStateFrame == move.recoveryFrames,
                     "recovery lasts its exact configured frame count");
    AdvanceEmpty(game, 1);
    passed &= Expect(Volt(game).combatState == arena::CombatState::Available &&
                         Volt(game).attackPhase == arena::AttackPhase::None,
                     "fighter is available on the tick after recovery completes");
    return passed;
}

bool TestDamageOneHitAndHitStunLockout() {
    arena::Game game;
    game.initialize();
    AdvanceToFighting(game);
    ApproachToBodyContact(game);
    AdvanceToFirstActiveFrame(game, 0, arena::MoveId::Light);

    bool passed = true;
    passed &= Expect(Ember(game).health == 94, "light attack deals six damage");
    passed &= Expect(Volt(game).health == 100, "attack does not damage its owner");
    passed &= Expect(Volt(game).energy == 8 && Ember(game).energy == 0,
                     "successful light hit awards attacker energy");
    passed &= Expect(Ember(game).combatState == arena::CombatState::HitStun &&
                         Ember(game).hitStunFrames == 12,
                     "unblocked light hit applies configured hit-stun");
    passed &= Expect(Ember(game).velocity.x > 0.0F, "hit applies facing-relative knockback");

    arena::FrameCommands counterAttack;
    counterAttack.fighters[1].heavyAttackPressed = true;
    game.advanceSimulation(counterAttack);
    passed &= Expect(Ember(game).combatState == arena::CombatState::HitStun &&
                         Ember(game).currentMove == arena::MoveId::None,
                     "fighter cannot attack during hit-stun");

    const int healthAfterFirstActiveFrame = Ember(game).health;
    const int energyAfterFirstActiveFrame = Volt(game).energy;
    AdvanceEmpty(game, 4);
    passed &= Expect(Ember(game).health == healthAfterFirstActiveFrame,
                     "one activation cannot hit again on later active frames");
    passed &= Expect(Volt(game).energy == energyAfterFirstActiveFrame,
                     "one activation cannot award energy again on later active frames");
    return passed;
}

bool TestBlocking() {
    arena::Game game;
    game.initialize();
    AdvanceToFighting(game);
    ApproachToBodyContact(game);
    AdvanceToFirstActiveFrame(game, 0, arena::MoveId::Heavy, true);

    bool passed = true;
    passed &= Expect(Ember(game).health == 100, "grounded front block prevents all damage");
    passed &= Expect(Ember(game).combatState == arena::CombatState::Blocking,
                     "held block remains active after guarding");
    passed &= Expect(Ember(game).blockFlashFrames > 0, "blocked hit emits block feedback");
    passed &= Expect(Volt(game).energy == 4 && Ember(game).energy == 6,
                     "blocked attack awards both attacker and defender energy once");

    arena::FrameCommands blockAndAttack;
    blockAndAttack.fighters[1].blockHeld = true;
    blockAndAttack.fighters[1].lightAttackPressed = true;
    game.advanceSimulation(blockAndAttack);
    passed &= Expect(Ember(game).combatState == arena::CombatState::Blocking &&
                         Ember(game).currentMove == arena::MoveId::None,
                     "blocking cannot attack simultaneously");
    AdvanceEmpty(game, 4);
    passed &= Expect(Volt(game).energy == 4 && Ember(game).energy == 6,
                     "blocked activation cannot award repeated energy");
    return passed;
}

bool TestInvalidAttackDuringRecovery() {
    arena::Game game;
    game.initialize();
    AdvanceToFighting(game);
    const arena::MoveDefinition& light = arena::GetMoveDefinition(arena::MoveId::Light);
    PressMove(game, 0, arena::MoveId::Light);
    AdvanceEmpty(game, light.startupFrames + light.activeFrames);

    bool passed = true;
    passed &= Expect(Volt(game).attackPhase == arena::AttackPhase::Recovery,
                     "test reaches light recovery");
    const int recoveryFrame = Volt(game).combatStateFrame;
    arena::FrameCommands commands;
    commands.fighters[0].heavyAttackPressed = true;
    game.advanceSimulation(commands);
    passed &= Expect(Volt(game).currentMove == arena::MoveId::Light &&
                         Volt(game).attackPhase == arena::AttackPhase::Recovery &&
                         Volt(game).combatStateFrame == recoveryFrame + 1,
                     "new attack is rejected during recovery");
    return passed;
}

bool TestHorizontalAndVerticalMisses() {
    arena::Game horizontalGame;
    horizontalGame.initialize();
    AdvanceToFighting(horizontalGame);
    AdvanceToFirstActiveFrame(horizontalGame, 0, arena::MoveId::Heavy);
    bool passed = true;
    passed &= Expect(Ember(horizontalGame).health == 100,
                     "attack misses outside horizontal range");

    arena::Game verticalGame;
    verticalGame.initialize();
    AdvanceToFighting(verticalGame);
    ApproachToBodyContact(verticalGame);
    arena::FrameCommands jump;
    jump.fighters[1].jumpPressed = true;
    jump.fighters[1].jumpHeld = true;
    verticalGame.advanceSimulation(jump);
    jump.fighters[1].jumpPressed = false;
    for (int tick = 0; tick < 7; ++tick) {
        verticalGame.advanceSimulation(jump);
    }
    AdvanceToFirstActiveFrame(verticalGame, 0, arena::MoveId::Light);
    passed &= Expect(Ember(verticalGame).health == 100,
                     "attack misses without vertical hurtbox overlap");
    return passed;
}

bool TestHitboxOrientation() {
    arena::Game game;
    game.initialize();
    AdvanceToFighting(game);
    arena::Fighter rightFacing = Volt(game);
    arena::Fighter leftFacing = Volt(game);
    rightFacing.facing = arena::FacingDirection::Right;
    leftFacing.facing = arena::FacingDirection::Left;
    const arena::MoveDefinition& light = arena::GetMoveDefinition(arena::MoveId::Light);
    const arena::Rectanglef rightBox = arena::GetAttackHitbox(rightFacing, light);
    const arena::Rectanglef leftBox = arena::GetAttackHitbox(leftFacing, light);

    bool passed = true;
    passed &= Expect(rightBox.x >= rightFacing.position.x + rightFacing.collisionWidth * 0.5F,
                     "right-facing hitbox starts beyond the right body edge");
    passed &= Expect(leftBox.x + leftBox.width <=
                         leftFacing.position.x - leftFacing.collisionWidth * 0.5F,
                     "left-facing hitbox ends before the left body edge");
    passed &= Expect(arena::IsAttackInFrontOfDefender(Volt(game), Ember(game)) &&
                         arena::IsAttackInFrontOfDefender(Ember(game), Volt(game)),
                     "initial facing treats the opponent as in front for both fighters");
    arena::Fighter attackerBehind = Volt(game);
    attackerBehind.position.x = Ember(game).position.x + 100.0F;
    passed &= Expect(!arena::IsAttackInFrontOfDefender(attackerBehind, Ember(game)),
                     "an attacker behind the defender is outside the directional guard");
    return passed;
}

bool TestSimultaneousTrade() {
    arena::Game game;
    game.initialize();
    AdvanceToFighting(game);
    ApproachToBodyContact(game);

    arena::FrameCommands attacks;
    attacks.fighters[0].lightAttackPressed = true;
    attacks.fighters[1].lightAttackPressed = true;
    game.advanceSimulation(attacks);
    AdvanceEmpty(game, arena::GetMoveDefinition(arena::MoveId::Light).startupFrames);

    bool passed = true;
    passed &= Expect(Volt(game).health == 94 && Ember(game).health == 94,
                     "same-tick valid attacks both deal damage");
    passed &= Expect(Volt(game).energy == 8 && Ember(game).energy == 8,
                     "same-tick trade awards both attackers without order bias");
    passed &= Expect(Volt(game).combatState == arena::CombatState::HitStun &&
                         Ember(game).combatState == arena::CombatState::HitStun,
                     "both fighters enter hit-stun after a trade");
    passed &= Expect(!Volt(game).attackHasConnected && !Ember(game).attackHasConnected,
                     "trade interruption leaves symmetric transient attack state");
    return passed;
}

bool TestKnockbackAtStageBoundary() {
    arena::Game game;
    game.initialize();
    AdvanceToFighting(game);
    arena::FrameCommands moveRight;
    moveRight.fighters[0].moveRightHeld = true;
    moveRight.fighters[1].moveRightHeld = true;
    for (int tick = 0; tick < arena::kSimulationRate * 4; ++tick) {
        game.advanceSimulation(moveRight);
    }

    const float maximumX = arena::kArenaRightBoundary - Ember(game).collisionWidth * 0.5F;
    AdvanceToFirstActiveFrame(game, 0, arena::MoveId::Heavy);
    const float impactVelocity = Ember(game).velocity.x;
    AdvanceEmpty(game, 5);

    bool passed = true;
    passed &= Expect(impactVelocity > 0.0F, "heavy hit applies positive knockback at right wall");
    passed &= Expect(Ember(game).position.x <= maximumX + kTolerance,
                     "knockback cannot move a fighter beyond the stage boundary");
    passed &= Expect(NearlyEqual(Ember(game).position.x, maximumX),
                     "wall-constrained knockback leaves fighter at the legal edge");
    return passed;
}

bool TestHeldAttackDoesNotRepeat() {
    arena::Game game;
    game.initialize();
    AdvanceToFighting(game);
    arena::FrameCommands commands;
    commands.fighters[0].lightAttackHeld = true;
    commands.fighters[0].lightAttackPressed = true;
    game.advanceSimulation(commands);
    commands.fighters[0].lightAttackPressed = false;
    const arena::MoveDefinition& light = arena::GetMoveDefinition(arena::MoveId::Light);
    for (int tick = 0; tick < light.startupFrames + light.activeFrames +
                               light.recoveryFrames + 20;
         ++tick) {
        game.advanceSimulation(commands);
    }

    return Expect(Volt(game).combatState == arena::CombatState::Available &&
                      Volt(game).currentMove == arena::MoveId::None,
                  "held attack without a new press does not restart");
}

bool TestEnergyRulesAndSpecialFoundation() {
    arena::Game game;
    game.initialize();
    AdvanceToFighting(game);
    ApproachToBodyContact(game);
    AdvanceToFirstActiveFrame(game, 0, arena::MoveId::Heavy);

    bool passed = true;
    passed &= Expect(Volt(game).energy == 12,
                     "successful heavy hit awards twelve attacker energy");

    arena::Fighter fighter = Volt(game);
    fighter.combatState = arena::CombatState::Available;
    arena::GainEnergy(fighter, 500);
    passed &= Expect(fighter.energy == fighter.maximumEnergy,
                     "energy gain clamps at maximum energy");
    arena::GainEnergy(fighter, -20);
    passed &= Expect(fighter.energy == fighter.maximumEnergy,
                     "negative energy awards cannot reduce energy");
    passed &= Expect(arena::SpendEnergyForSpecial(fighter) && fighter.energy == 75,
                     "future special spending atomically deducts the configured cost");

    arena::Fighter emptyFighter = Ember(game);
    emptyFighter.combatState = arena::CombatState::Available;
    emptyFighter.energy = 0;
    passed &= Expect(!arena::CanActivateSpecial(emptyFighter) &&
                         !arena::SpendEnergyForSpecial(emptyFighter) &&
                         emptyFighter.energy == 0,
                     "insufficient energy rejects future special activation without underflow");
    emptyFighter.energy = arena::GetCombatRules().specialEnergyCost;
    emptyFighter.combatState = arena::CombatState::Charging;
    passed &= Expect(!arena::CanActivateSpecial(emptyFighter),
                     "future special activation is locked out while charging");
    return passed;
}

bool TestFrameCommandEdgeBuffer() {
    arena::FrameCommandBuffer buffer;
    arena::FrameCommands pressed;
    pressed.fighters[0].specialAttackHeld = true;
    pressed.fighters[0].specialAttackPressed = true;
    pressed.fighters[1].chargedAttackHeld = true;
    pressed.fighters[1].chargedAttackPressed = true;
    pressed.restartPressed = true;
    buffer.sample(pressed);

    arena::FrameCommands released;
    released.fighters[0].specialAttackReleased = true;
    released.fighters[1].chargedAttackReleased = true;
    buffer.sample(released);

    const arena::FrameCommands firstTick = buffer.consumeForSimulationTick();
    const arena::FrameCommands catchUpTick = buffer.consumeForSimulationTick();
    bool passed = true;
    passed &= Expect(firstTick.fighters[0].specialAttackPressed &&
                         firstTick.fighters[0].specialAttackReleased &&
                         !firstTick.fighters[0].specialAttackHeld,
                     "special press and release survive render frames until a simulation tick");
    passed &= Expect(firstTick.fighters[1].chargedAttackPressed &&
                         firstTick.fighters[1].chargedAttackReleased &&
                         !firstTick.fighters[1].chargedAttackHeld,
                     "charge press and release survive render frames until a simulation tick");
    passed &= Expect(firstTick.restartPressed && !catchUpTick.restartPressed,
                     "restart is buffered and consumed as a one-shot command");
    passed &= Expect(!catchUpTick.fighters[0].specialAttackPressed &&
                         !catchUpTick.fighters[0].specialAttackReleased &&
                         !catchUpTick.fighters[1].chargedAttackPressed &&
                         !catchUpTick.fighters[1].chargedAttackReleased,
                     "one-shot edges are not repeated on a catch-up tick");

    arena::Game edgeGame;
    edgeGame.initialize();
    AdvanceToFighting(edgeGame);
    edgeGame.advanceSimulation(firstTick);
    passed &= Expect(Volt(edgeGame).energy == 0 &&
                         Volt(edgeGame).combatState == arena::CombatState::Available,
                     "insufficient-energy special input cannot spend or start a move");
    passed &= Expect(Ember(edgeGame).chargeReleaseOutcome ==
                         arena::ChargeReleaseOutcome::Cancelled,
                     "a buffered charge release is not lost when press and release share a tick");

    arena::FrameCommands held;
    held.fighters[0].chargedAttackHeld = true;
    buffer.sample(held);
    const arena::FrameCommands heldTick = buffer.consumeForSimulationTick();
    const arena::FrameCommands secondHeldTick = buffer.consumeForSimulationTick();
    passed &= Expect(heldTick.fighters[0].chargedAttackHeld &&
                         secondHeldTick.fighters[0].chargedAttackHeld,
                     "held input remains available to every catch-up tick");
    return passed;
}

void BeginCharge(arena::Game& game, std::size_t fighterIndex) {
    arena::FrameCommands commands;
    commands.fighters[fighterIndex].chargedAttackHeld = true;
    commands.fighters[fighterIndex].chargedAttackPressed = true;
    game.advanceSimulation(commands);
}

void HoldCharge(arena::Game& game, std::size_t fighterIndex, int tickCount) {
    arena::FrameCommands commands;
    commands.fighters[fighterIndex].chargedAttackHeld = true;
    for (int tick = 0; tick < tickCount; ++tick) {
        game.advanceSimulation(commands);
    }
}

void ReleaseCharge(arena::Game& game, std::size_t fighterIndex) {
    arena::FrameCommands commands;
    commands.fighters[fighterIndex].chargedAttackReleased = true;
    game.advanceSimulation(commands);
}

void ChargeAndRelease(arena::Game& game, std::size_t fighterIndex, int chargeFrames) {
    BeginCharge(game, fighterIndex);
    HoldCharge(game, fighterIndex, chargeFrames - 1);
    ReleaseCharge(game, fighterIndex);
}

void KnockOutWithChargedAttacks(arena::Game& game, std::size_t attackerIndex) {
    const arena::MoveId moveId = attackerIndex == 0
                                     ? arena::MoveId::ThunderStrikeLevel3
                                     : arena::MoveId::FlameSmashLevel3;
    for (int attack = 0; attack < 4; ++attack) {
        AdvanceUntilBothAvailable(game);
        ApproachToBodyContact(game);
        ChargeAndRelease(game, attackerIndex, 60);
        AdvanceEmpty(game, arena::GetMoveDefinition(moveId).startupFrames);
    }
}

void TradeLevelThreeChargedAttacks(arena::Game& game) {
    AdvanceUntilBothAvailable(game);
    ApproachToBodyContact(game);
    arena::FrameCommands bothStart;
    bothStart.fighters[0].chargedAttackPressed = true;
    bothStart.fighters[0].chargedAttackHeld = true;
    bothStart.fighters[1].chargedAttackPressed = true;
    bothStart.fighters[1].chargedAttackHeld = true;
    game.advanceSimulation(bothStart);

    arena::FrameCommands bothHeld;
    bothHeld.fighters[0].chargedAttackHeld = true;
    bothHeld.fighters[1].chargedAttackHeld = true;
    for (int tick = 0; tick < 59; ++tick) {
        game.advanceSimulation(bothHeld);
    }

    arena::FrameCommands emberRelease;
    emberRelease.fighters[0].chargedAttackHeld = true;
    emberRelease.fighters[1].chargedAttackReleased = true;
    game.advanceSimulation(emberRelease);
    HoldCharge(game, 0, 4);
    ReleaseCharge(game, 0);
    AdvanceEmpty(game, 8);
}

bool TestChargeThresholdsAndMaximum() {
    const arena::CombatRules& rules = arena::GetCombatRules();
    arena::Game earlyRelease;
    earlyRelease.initialize();
    AdvanceToFighting(earlyRelease);
    BeginCharge(earlyRelease, 0);
    HoldCharge(earlyRelease, 0, rules.minimumChargeFrames - 2);

    bool passed = true;
    passed &= Expect(Volt(earlyRelease).chargeFrames == rules.minimumChargeFrames - 1,
                     "charge timer counts the accepted press tick as frame one");
    ReleaseCharge(earlyRelease, 0);
    passed &= Expect(Volt(earlyRelease).combatState == arena::CombatState::Available &&
                         Volt(earlyRelease).chargeReleaseOutcome ==
                             arena::ChargeReleaseOutcome::Cancelled,
                     "release one tick before minimum charge cancels");

    arena::Game exactRelease;
    exactRelease.initialize();
    AdvanceToFighting(exactRelease);
    BeginCharge(exactRelease, 0);
    HoldCharge(exactRelease, 0, rules.minimumChargeFrames - 1);
    passed &= Expect(Volt(exactRelease).chargeFrames == rules.minimumChargeFrames,
                     "minimum charge threshold occurs on the exact configured tick");
    ReleaseCharge(exactRelease, 0);
    passed &= Expect(Volt(exactRelease).combatState == arena::CombatState::Attacking &&
                         Volt(exactRelease).currentMove ==
                             arena::MoveId::ThunderStrikeLevel1 &&
                         Volt(exactRelease).attackPhase == arena::AttackPhase::Startup &&
                         Volt(exactRelease).chargeReleaseOutcome ==
                             arena::ChargeReleaseOutcome::Ready,
                     "release at minimum charge starts Thunder Strike startup");
    exactRelease.advanceSimulation({});
    passed &= Expect(Volt(exactRelease).chargeReleaseOutcome ==
                         arena::ChargeReleaseOutcome::None,
                     "charge release outcome lasts one simulation tick");

    arena::Game maximumCharge;
    maximumCharge.initialize();
    AdvanceToFighting(maximumCharge);
    BeginCharge(maximumCharge, 0);
    HoldCharge(maximumCharge, 0, rules.maximumChargeFrames + 20);
    passed &= Expect(Volt(maximumCharge).combatState == arena::CombatState::Charging &&
                         Volt(maximumCharge).chargeFrames == rules.maximumChargeFrames,
                     "charge duration remains capped while the button stays held");
    return passed;
}

bool TestChargedMoveDataAndLevelBoundaries() {
    struct BoundaryCase {
        int chargeFrames;
        int level;
    };
    constexpr std::array<BoundaryCase, 7> kCases = {{{17, 0}, {18, 1}, {35, 1}, {36, 2},
                                                     {59, 2}, {60, 3}, {72, 3}}};

    bool passed = true;
    for (const BoundaryCase& testCase : kCases) {
        arena::Game game;
        game.initialize();
        AdvanceToFighting(game);
        BeginCharge(game, 0);
        HoldCharge(game, 0, testCase.chargeFrames - 1);
        passed &= Expect(Volt(game).chargeFrames == testCase.chargeFrames,
                         "charge reaches each exact level boundary");
        ReleaseCharge(game, 0);
        if (testCase.level == 0) {
            passed &= Expect(Volt(game).combatState == arena::CombatState::Available &&
                                 Volt(game).chargeReleaseOutcome ==
                                     arena::ChargeReleaseOutcome::Cancelled &&
                                 Volt(game).chargeFrames == 0,
                             "a 17-tick release cancels and clears charge state");
        } else {
            passed &= Expect(Volt(game).combatState == arena::CombatState::Attacking &&
                                 arena::GetMoveChargeLevel(Volt(game).currentMove) ==
                                     testCase.level &&
                                 Volt(game).chargeReleaseOutcome ==
                                     arena::ChargeReleaseOutcome::Ready &&
                                 Volt(game).chargeFrames == 0,
                             "charged release selects the exact configured level once");
        }
    }

    const arena::MoveDefinition& thunder1 =
        arena::GetMoveDefinition(arena::MoveId::ThunderStrikeLevel1);
    const arena::MoveDefinition& thunder2 =
        arena::GetMoveDefinition(arena::MoveId::ThunderStrikeLevel2);
    const arena::MoveDefinition& thunder3 =
        arena::GetMoveDefinition(arena::MoveId::ThunderStrikeLevel3);
    const arena::MoveDefinition& flame1 =
        arena::GetMoveDefinition(arena::MoveId::FlameSmashLevel1);
    const arena::MoveDefinition& flame2 =
        arena::GetMoveDefinition(arena::MoveId::FlameSmashLevel2);
    const arena::MoveDefinition& flame3 =
        arena::GetMoveDefinition(arena::MoveId::FlameSmashLevel3);
    passed &= Expect(thunder1.damage == 16 && thunder2.damage == 23 &&
                         thunder3.damage == 30 && flame1.damage == 16 &&
                         flame2.damage == 23 && flame3.damage == 30,
                     "charged damage is centralized at 16, 23, and 30 by level");
    passed &= Expect(thunder1.owner == arena::MoveOwner::Volt &&
                         flame1.owner == arena::MoveOwner::Ember &&
                         thunder1.startupFrames < flame1.startupFrames &&
                         thunder1.hitbox.width > flame1.hitbox.width &&
                         flame1.knockbackStrength > thunder1.knockbackStrength &&
                         flame2.knockbackStrength > thunder2.knockbackStrength &&
                         flame3.knockbackStrength > thunder3.knockbackStrength,
                     "charged move data preserves VOLT speed/range and EMBER knockback identity");

    arena::Game capped;
    capped.initialize();
    AdvanceToFighting(capped);
    BeginCharge(capped, 1);
    HoldCharge(capped, 1, 100);
    passed &= Expect(Ember(capped).chargeFrames == 72,
                     "charging beyond 72 ticks remains capped");
    ReleaseCharge(capped, 1);
    passed &= Expect(Ember(capped).currentMove == arena::MoveId::FlameSmashLevel3,
                     "a capped EMBER release selects Flame Smash level three");
    return passed;
}

bool TestChargedDamageTimingAndEconomy() {
    constexpr std::array<int, 3> kChargeFrames = {18, 36, 60};
    constexpr std::array<int, 3> kDamage = {16, 23, 30};
    bool passed = true;

    for (std::size_t fighterIndex = 0; fighterIndex < 2; ++fighterIndex) {
        for (std::size_t levelIndex = 0; levelIndex < kChargeFrames.size(); ++levelIndex) {
            arena::Game game;
            game.initialize();
            AdvanceToFighting(game);
            ApproachToBodyContact(game);
            const int defenderHealth = game.state().fighters[1U - fighterIndex].health;
            const int attackerEnergy = game.state().fighters[fighterIndex].energy;
            const int defenderEnergy = game.state().fighters[1U - fighterIndex].energy;
            ChargeAndRelease(game, fighterIndex, kChargeFrames[levelIndex]);
            const arena::MoveDefinition& move =
                arena::GetMoveDefinition(game.state().fighters[fighterIndex].currentMove);
            AdvanceEmpty(game, move.startupFrames - 1);
            passed &= Expect(game.state().fighters[1U - fighterIndex].health == defenderHealth &&
                                 game.state().fighters[fighterIndex].attackPhase ==
                                     arena::AttackPhase::Startup,
                             "charged attacks do not damage during startup");
            AdvanceEmpty(game, 1);
            passed &= Expect(game.state().fighters[1U - fighterIndex].health ==
                                 defenderHealth - kDamage[levelIndex] &&
                                 game.state().fighters[fighterIndex].attackHasConnected,
                             "each charged level deals its configured damage on activation");
            const int healthAfterHit = game.state().fighters[1U - fighterIndex].health;
            AdvanceEmpty(game, move.activeFrames - 1);
            passed &= Expect(game.state().fighters[1U - fighterIndex].health == healthAfterHit,
                             "a charged activation connects only once");
            passed &= Expect(game.state().fighters[fighterIndex].energy == attackerEnergy &&
                                 game.state().fighters[1U - fighterIndex].energy ==
                                     defenderEnergy,
                             "charged hits neither spend nor award energy");
        }
    }

    arena::Game lifecycle;
    lifecycle.initialize();
    AdvanceToFighting(lifecycle);
    ChargeAndRelease(lifecycle, 0, 36);
    const arena::MoveDefinition& move =
        arena::GetMoveDefinition(arena::MoveId::ThunderStrikeLevel2);
    passed &= Expect(Volt(lifecycle).combatStateFrame == 1 &&
                         Volt(lifecycle).attackPhase == arena::AttackPhase::Startup,
                     "charged release tick is startup frame one");
    AdvanceEmpty(lifecycle, move.startupFrames - 1);
    passed &= Expect(Volt(lifecycle).combatStateFrame == move.startupFrames &&
                         Volt(lifecycle).attackPhase == arena::AttackPhase::Startup,
                     "charged startup lasts its exact configured ticks");
    AdvanceEmpty(lifecycle, 1);
    passed &= Expect(Volt(lifecycle).attackPhase == arena::AttackPhase::Active &&
                         Volt(lifecycle).combatStateFrame == 1,
                     "charged active begins after startup");
    AdvanceEmpty(lifecycle, move.activeFrames);
    passed &= Expect(Volt(lifecycle).attackPhase == arena::AttackPhase::Recovery &&
                         Volt(lifecycle).combatStateFrame == 1,
                     "charged recovery follows the exact active duration");
    AdvanceEmpty(lifecycle, move.recoveryFrames);
    passed &= Expect(Volt(lifecycle).combatState == arena::CombatState::Available &&
                         Volt(lifecycle).currentMove == arena::MoveId::None,
                     "charged recovery returns to available on schedule");
    return passed;
}

bool TestChargedBlockMissFacingAndLockouts() {
    bool passed = true;

    arena::Game blocked;
    blocked.initialize();
    AdvanceToFighting(blocked);
    ApproachToBodyContact(blocked);
    ChargeAndRelease(blocked, 0, 60);
    const arena::MoveDefinition& thunder =
        arena::GetMoveDefinition(arena::MoveId::ThunderStrikeLevel3);
    arena::FrameCommands guard;
    guard.fighters[1].blockHeld = true;
    for (int tick = 0; tick < thunder.startupFrames; ++tick) {
        blocked.advanceSimulation(guard);
    }
    passed &= Expect(Ember(blocked).health == 100 && Ember(blocked).blockFlashFrames > 0 &&
                         Volt(blocked).attackHasConnected && Volt(blocked).energy == 0 &&
                         Ember(blocked).energy == 0,
                     "a facing block consumes a charged hit with no chip or energy gain");

    arena::Game miss;
    miss.initialize();
    AdvanceToFighting(miss);
    ChargeAndRelease(miss, 0, 60);
    AdvanceEmpty(miss, thunder.startupFrames + thunder.activeFrames);
    passed &= Expect(Ember(miss).health == 100 && !Volt(miss).attackHasConnected,
                     "an out-of-range charged attack misses");

    arena::Fighter rightFacing = Volt(miss);
    rightFacing.position.x = 500.0F;
    rightFacing.facing = arena::FacingDirection::Right;
    arena::Fighter leftFacing = rightFacing;
    leftFacing.facing = arena::FacingDirection::Left;
    const arena::Rectanglef rightHitbox = arena::GetAttackHitbox(rightFacing, thunder);
    const arena::Rectanglef leftHitbox = arena::GetAttackHitbox(leftFacing, thunder);
    passed &= Expect(rightHitbox.x > rightFacing.position.x &&
                         leftHitbox.x + leftHitbox.width < leftFacing.position.x,
                     "charged hitboxes mirror with facing direction");

    arena::Game recovery;
    recovery.initialize();
    AdvanceToFighting(recovery);
    ChargeAndRelease(recovery, 0, 18);
    AdvanceEmpty(recovery, arena::GetMoveDefinition(Volt(recovery).currentMove).startupFrames +
                               arena::GetMoveDefinition(Volt(recovery).currentMove).activeFrames);
    arena::FrameCommands rejectedCharge;
    rejectedCharge.fighters[0].chargedAttackPressed = true;
    rejectedCharge.fighters[0].chargedAttackHeld = true;
    recovery.advanceSimulation(rejectedCharge);
    passed &= Expect(Volt(recovery).combatState == arena::CombatState::Attacking &&
                         Volt(recovery).attackPhase == arena::AttackPhase::Recovery,
                     "charging cannot start during charged recovery");

    arena::Game interrupted;
    interrupted.initialize();
    AdvanceToFighting(interrupted);
    ApproachToBodyContact(interrupted);
    arena::FrameCommands startLightAndCharge;
    startLightAndCharge.fighters[0].lightAttackPressed = true;
    startLightAndCharge.fighters[1].chargedAttackPressed = true;
    startLightAndCharge.fighters[1].chargedAttackHeld = true;
    interrupted.advanceSimulation(startLightAndCharge);
    arena::FrameCommands keepCharging;
    keepCharging.fighters[1].chargedAttackHeld = true;
    for (int tick = 0; tick < arena::GetMoveDefinition(arena::MoveId::Light).startupFrames;
         ++tick) {
        interrupted.advanceSimulation(keepCharging);
    }
    passed &= Expect(Ember(interrupted).combatState == arena::CombatState::HitStun &&
                         Ember(interrupted).chargeReleaseOutcome ==
                             arena::ChargeReleaseOutcome::Interrupted &&
                         Ember(interrupted).chargeFrames == 0,
                     "an unblocked hit cancels accumulated charge");
    arena::FrameCommands staleRelease;
    staleRelease.fighters[1].chargedAttackReleased = true;
    interrupted.advanceSimulation(staleRelease);
    passed &= Expect(Ember(interrupted).combatState == arena::CombatState::HitStun &&
                         Ember(interrupted).currentMove == arena::MoveId::None,
                     "a stale release after interruption cannot start a charged attack");
    return passed;
}

bool TestChargedTradesAndOtherMoveInteractions() {
    bool passed = true;

    arena::Game chargedTrade;
    chargedTrade.initialize();
    AdvanceToFighting(chargedTrade);
    ApproachToBodyContact(chargedTrade);
    arena::FrameCommands bothStart;
    bothStart.fighters[0].chargedAttackPressed = true;
    bothStart.fighters[0].chargedAttackHeld = true;
    bothStart.fighters[1].chargedAttackPressed = true;
    bothStart.fighters[1].chargedAttackHeld = true;
    chargedTrade.advanceSimulation(bothStart);
    arena::FrameCommands bothHeld;
    bothHeld.fighters[0].chargedAttackHeld = true;
    bothHeld.fighters[1].chargedAttackHeld = true;
    for (int tick = 0; tick < 58; ++tick) {
        chargedTrade.advanceSimulation(bothHeld);
    }
    arena::FrameCommands emberRelease;
    emberRelease.fighters[0].chargedAttackHeld = true;
    emberRelease.fighters[1].chargedAttackReleased = true;
    chargedTrade.advanceSimulation(emberRelease);
    for (int tick = 0; tick < 4; ++tick) {
        arena::FrameCommands voltHeld;
        voltHeld.fighters[0].chargedAttackHeld = true;
        chargedTrade.advanceSimulation(voltHeld);
    }
    ReleaseCharge(chargedTrade, 0);
    AdvanceEmpty(chargedTrade, 8);
    passed &= Expect(Volt(chargedTrade).health == 77 && Ember(chargedTrade).health == 70 &&
                         Volt(chargedTrade).combatState == arena::CombatState::HitStun &&
                         Ember(chargedTrade).combatState == arena::CombatState::HitStun,
                     "different charged startups still resolve a fair simultaneous trade");

    arena::Game normalInterrupt;
    normalInterrupt.initialize();
    AdvanceToFighting(normalInterrupt);
    ApproachToBodyContact(normalInterrupt);
    ChargeAndRelease(normalInterrupt, 0, 18);
    PressMove(normalInterrupt, 1, arena::MoveId::Light);
    AdvanceEmpty(normalInterrupt,
                 arena::GetMoveDefinition(arena::MoveId::Light).startupFrames);
    passed &= Expect(Volt(normalInterrupt).health == 94 && Ember(normalInterrupt).health == 100 &&
                         Volt(normalInterrupt).combatState == arena::CombatState::HitStun,
                     "a normal attack can interrupt charged startup before activation");

    arena::Game specialTrade;
    specialTrade.initialize();
    AdvanceToFighting(specialTrade);
    EarnEnergy(specialTrade, 1, 25);
    ApproachToBodyContact(specialTrade);
    const int voltHealthBeforeSpecialTrade = Volt(specialTrade).health;
    const int emberHealthBeforeSpecialTrade = Ember(specialTrade).health;
    BeginCharge(specialTrade, 0);
    HoldCharge(specialTrade, 0, 20);
    arena::FrameCommands specialAndHold;
    specialAndHold.fighters[0].chargedAttackHeld = true;
    specialAndHold.fighters[1].specialAttackPressed = true;
    specialTrade.advanceSimulation(specialAndHold);
    HoldCharge(specialTrade, 0, 7);
    ReleaseCharge(specialTrade, 0);
    AdvanceEmpty(specialTrade, 8);
    passed &= Expect(Volt(specialTrade).health == voltHealthBeforeSpecialTrade - 20 &&
                         Ember(specialTrade).health == emberHealthBeforeSpecialTrade - 16 &&
                         Volt(specialTrade).combatState == arena::CombatState::HitStun &&
                         Ember(specialTrade).combatState == arena::CombatState::HitStun,
                     "charged and special active contacts resolve in the same snapshot");
    return passed;
}

bool TestChargedKoLockoutAndReplayConsistency() {
    arena::Game koGame;
    koGame.initialize();
    AdvanceToFighting(koGame);
    ApproachToBodyContact(koGame);
    for (int attack = 0; attack < 4 && Ember(koGame).health > 0; ++attack) {
        AdvanceUntilBothAvailable(koGame);
        ApproachToBodyContact(koGame);
        ChargeAndRelease(koGame, 0, 60);
        AdvanceEmpty(koGame,
                     arena::GetMoveDefinition(arena::MoveId::ThunderStrikeLevel3)
                         .startupFrames);
    }
    const int koHealth = Ember(koGame).health;
    BeginCharge(koGame, 1);
    bool passed = Expect(koHealth == 0 &&
                             Ember(koGame).combatState == arena::CombatState::KnockedOut &&
                             Ember(koGame).currentMove == arena::MoveId::None,
                         "KO rejects new charge activation");

    arena::Game first;
    arena::Game second;
    first.initialize();
    AdvanceToFighting(first);
    second.initialize();
    AdvanceToFighting(second);
    for (int tick = 0; tick < 150; ++tick) {
        arena::FrameCommands commands;
        commands.fighters[0].chargedAttackPressed = tick == 0;
        commands.fighters[0].chargedAttackHeld = tick < 60;
        commands.fighters[0].chargedAttackReleased = tick == 60;
        commands.fighters[1].chargedAttackPressed = tick == 10;
        commands.fighters[1].chargedAttackHeld = tick >= 10 && tick < 46;
        commands.fighters[1].chargedAttackReleased = tick == 46;
        first.advanceSimulation(commands);
        second.advanceSimulation(commands);
        passed &= Expect(CompleteStatesEqual(first, second),
                         "charged command replay reproduces complete state each tick");
        if (!passed) {
            return false;
        }
    }
    return passed;
}

bool TestChargeInterruptionAndCombatLockouts() {
    arena::Game lockoutGame;
    lockoutGame.initialize();
    AdvanceToFighting(lockoutGame);
    arena::FrameCommands competingCommands;
    competingCommands.fighters[0].lightAttackPressed = true;
    competingCommands.fighters[0].chargedAttackPressed = true;
    competingCommands.fighters[0].chargedAttackHeld = true;
    lockoutGame.advanceSimulation(competingCommands);

    bool passed = true;
    passed &= Expect(Volt(lockoutGame).combatState == arena::CombatState::Attacking &&
                         Volt(lockoutGame).currentMove == arena::MoveId::Light,
                     "existing normal-attack priority wins over charge on the same tick");

    arena::Game chargingGame;
    chargingGame.initialize();
    AdvanceToFighting(chargingGame);
    BeginCharge(chargingGame, 0);
    const float initialX = Volt(chargingGame).position.x;
    arena::FrameCommands rejectedActions;
    rejectedActions.fighters[0].chargedAttackHeld = true;
    rejectedActions.fighters[0].moveRightHeld = true;
    rejectedActions.fighters[0].heavyAttackPressed = true;
    rejectedActions.fighters[0].jumpPressed = true;
    chargingGame.advanceSimulation(rejectedActions);
    passed &= Expect(Volt(chargingGame).combatState == arena::CombatState::Charging &&
                         Volt(chargingGame).currentMove == arena::MoveId::None &&
                         NearlyEqual(Volt(chargingGame).position.x, initialX) &&
                         Volt(chargingGame).grounded,
                     "charging locks out movement, jump, and normal attacks");

    arena::Game interruptedGame;
    interruptedGame.initialize();
    AdvanceToFighting(interruptedGame);
    ApproachToBodyContact(interruptedGame);
    arena::FrameCommands start;
    start.fighters[0].lightAttackPressed = true;
    start.fighters[1].chargedAttackPressed = true;
    start.fighters[1].chargedAttackHeld = true;
    interruptedGame.advanceSimulation(start);
    arena::FrameCommands hold;
    hold.fighters[1].chargedAttackHeld = true;
    for (int tick = 0; tick < arena::GetMoveDefinition(arena::MoveId::Light).startupFrames;
         ++tick) {
        interruptedGame.advanceSimulation(hold);
    }
    passed &= Expect(Ember(interruptedGame).combatState == arena::CombatState::HitStun &&
                         Ember(interruptedGame).chargeReleaseOutcome ==
                             arena::ChargeReleaseOutcome::Interrupted,
                     "a valid unblocked hit interrupts charging");
    return passed;
}

bool TestSpecialActivationEnergyAndLifecycle() {
    arena::Game insufficient;
    insufficient.initialize();
    AdvanceToFighting(insufficient);
    PressSpecial(insufficient, 0);

    bool passed = true;
    passed &= Expect(Volt(insufficient).combatState == arena::CombatState::Available &&
                         Volt(insufficient).energy == 0,
                     "insufficient energy rejects Thunder Bolt without spending");

    arena::Game thunderGame;
    thunderGame.initialize();
    AdvanceToFighting(thunderGame);
    EarnEnergy(thunderGame, 0, 25);
    const int thunderEnergyBefore = Volt(thunderGame).energy;
    const arena::MoveDefinition& thunder =
        arena::GetMoveDefinition(arena::MoveId::ThunderBolt);
    PressSpecial(thunderGame, 0);
    passed &= Expect(Volt(thunderGame).combatState == arena::CombatState::Attacking &&
                         Volt(thunderGame).currentMove == arena::MoveId::ThunderBolt &&
                         Volt(thunderGame).attackPhase == arena::AttackPhase::Startup &&
                         Volt(thunderGame).combatStateFrame == 1 &&
                         Volt(thunderGame).energy == thunderEnergyBefore - thunder.energyCost,
                     "valid VOLT special starts startup frame one and spends exactly once");
    AdvanceEmpty(thunderGame, thunder.startupFrames - 1);
    passed &= Expect(Volt(thunderGame).attackPhase == arena::AttackPhase::Startup &&
                         Volt(thunderGame).combatStateFrame == thunder.startupFrames,
                     "Thunder Bolt startup lasts exactly twelve ticks");
    AdvanceEmpty(thunderGame, 1);
    passed &= Expect(Volt(thunderGame).attackPhase == arena::AttackPhase::Active &&
                         Volt(thunderGame).combatStateFrame == 1,
                     "Thunder Bolt active spawn frame follows startup");
    AdvanceEmpty(thunderGame, 1);
    passed &= Expect(Volt(thunderGame).attackPhase == arena::AttackPhase::Recovery &&
                         Volt(thunderGame).combatStateFrame == 1,
                     "Thunder Bolt enters recovery after its one active tick");
    const int thunderEnergyAfterStart = Volt(thunderGame).energy;
    arena::FrameCommands heldSpecial;
    heldSpecial.fighters[0].specialAttackHeld = true;
    for (int tick = 0; tick < thunder.recoveryFrames + 8; ++tick) {
        thunderGame.advanceSimulation(heldSpecial);
    }
    passed &= Expect(Volt(thunderGame).combatState == arena::CombatState::Available &&
                         Volt(thunderGame).energy == thunderEnergyAfterStart,
                     "held special input neither spends again nor restarts Thunder Bolt");

    arena::Game infernoGame;
    infernoGame.initialize();
    AdvanceToFighting(infernoGame);
    EarnEnergy(infernoGame, 1, 25);
    const int infernoEnergyBefore = Ember(infernoGame).energy;
    const arena::MoveDefinition& inferno =
        arena::GetMoveDefinition(arena::MoveId::InfernoBurst);
    PressSpecial(infernoGame, 1);
    AdvanceEmpty(infernoGame, inferno.startupFrames - 1);
    passed &= Expect(Ember(infernoGame).attackPhase == arena::AttackPhase::Startup &&
                         Ember(infernoGame).combatStateFrame == inferno.startupFrames &&
                         Ember(infernoGame).energy == infernoEnergyBefore - inferno.energyCost,
                     "Inferno Burst spends once and keeps exact startup timing");
    AdvanceEmpty(infernoGame, 1);
    passed &= Expect(Ember(infernoGame).attackPhase == arena::AttackPhase::Active &&
                         Ember(infernoGame).combatStateFrame == 1,
                     "Inferno Burst active phase begins after sixteen startup ticks");
    AdvanceEmpty(infernoGame, inferno.activeFrames - 1);
    passed &= Expect(Ember(infernoGame).attackPhase == arena::AttackPhase::Active &&
                         Ember(infernoGame).combatStateFrame == inferno.activeFrames,
                     "Inferno Burst remains active for exactly five ticks");
    AdvanceEmpty(infernoGame, 1);
    passed &= Expect(Ember(infernoGame).attackPhase == arena::AttackPhase::Recovery &&
                         Ember(infernoGame).combatStateFrame == 1,
                     "Inferno Burst recovery follows its final active tick");
    AdvanceEmpty(infernoGame, inferno.recoveryFrames);
    passed &= Expect(Ember(infernoGame).combatState == arena::CombatState::Available,
                     "Inferno Burst returns to available after exact recovery");
    return passed;
}

bool TestSpecialSpendingLockoutsAndInterruption() {
    bool passed = true;

    arena::Game recoveryGame;
    recoveryGame.initialize();
    AdvanceToFighting(recoveryGame);
    EarnEnergy(recoveryGame, 0, 50);
    PressSpecial(recoveryGame, 0);
    const arena::MoveDefinition& thunder =
        arena::GetMoveDefinition(arena::MoveId::ThunderBolt);
    AdvanceEmpty(recoveryGame, thunder.startupFrames + thunder.activeFrames);
    const int recoveryEnergy = Volt(recoveryGame).energy;
    PressSpecial(recoveryGame, 0);
    passed &= Expect(Volt(recoveryGame).attackPhase == arena::AttackPhase::Recovery &&
                         Volt(recoveryGame).energy == recoveryEnergy,
                     "special input during recovery cannot spend energy");

    arena::Game stunGame;
    stunGame.initialize();
    AdvanceToFighting(stunGame);
    EarnEnergy(stunGame, 0, 25);
    ApproachToBodyContact(stunGame);
    AdvanceToFirstActiveFrame(stunGame, 1, arena::MoveId::Light);
    const int stunEnergy = Volt(stunGame).energy;
    PressSpecial(stunGame, 0);
    passed &= Expect(Volt(stunGame).combatState == arena::CombatState::HitStun &&
                         Volt(stunGame).energy == stunEnergy,
                     "special input during hit-stun cannot spend energy");

    arena::Game chargingGame;
    chargingGame.initialize();
    AdvanceToFighting(chargingGame);
    EarnEnergy(chargingGame, 0, 25);
    BeginCharge(chargingGame, 0);
    const int chargingEnergy = Volt(chargingGame).energy;
    arena::FrameCommands chargeAndSpecial;
    chargeAndSpecial.fighters[0].chargedAttackHeld = true;
    chargeAndSpecial.fighters[0].specialAttackPressed = true;
    chargingGame.advanceSimulation(chargeAndSpecial);
    passed &= Expect(Volt(chargingGame).combatState == arena::CombatState::Charging &&
                         Volt(chargingGame).energy == chargingEnergy,
                     "special input while charging cannot spend energy");

    arena::Game interruptedGame;
    interruptedGame.initialize();
    AdvanceToFighting(interruptedGame);
    EarnEnergy(interruptedGame, 0, 25);
    ApproachToBodyContact(interruptedGame);
    const int activationEnergy = Volt(interruptedGame).energy;
    arena::FrameCommands simultaneousStart;
    simultaneousStart.fighters[0].specialAttackPressed = true;
    simultaneousStart.fighters[1].lightAttackPressed = true;
    interruptedGame.advanceSimulation(simultaneousStart);
    AdvanceEmpty(interruptedGame,
                 arena::GetMoveDefinition(arena::MoveId::Light).startupFrames);
    passed &= Expect(Volt(interruptedGame).combatState == arena::CombatState::HitStun &&
                         Volt(interruptedGame).energy ==
                             activationEnergy - thunder.energyCost &&
                         ActiveProjectileCount(interruptedGame) == 0,
                     "startup interruption cancels the special without refund or projectile");

    arena::Game koGame;
    koGame.initialize();
    AdvanceToFighting(koGame);
    EarnEnergy(koGame, 0, 25);
    for (int attack = 0; attack < 8 &&
                         Volt(koGame).combatState != arena::CombatState::KnockedOut;
         ++attack) {
        AdvanceUntilBothAvailable(koGame);
        ApproachToBodyContact(koGame);
        AdvanceToFirstActiveFrame(koGame, 1, arena::MoveId::Heavy);
    }
    const int koEnergy = Volt(koGame).energy;
    PressSpecial(koGame, 0);
    passed &= Expect(Volt(koGame).combatState == arena::CombatState::KnockedOut &&
                         Volt(koGame).energy == koEnergy,
                     "special input while knocked out cannot spend energy");
    return passed;
}

bool TestThunderBoltProjectileStateAndDamage() {
    arena::Game game;
    game.initialize();
    AdvanceToFighting(game);
    EarnEnergy(game, 0, 25);

    arena::FrameCommands separate;
    separate.fighters[0].moveLeftHeld = true;
    separate.fighters[1].moveRightHeld = true;
    for (int tick = 0; tick < 40; ++tick) {
        game.advanceSimulation(separate);
    }
    AdvanceUntilBothAvailable(game);

    const int voltHealthBefore = Volt(game).health;
    const int emberHealthBefore = Ember(game).health;
    PressSpecial(game, 0);
    const arena::MoveDefinition& thunder =
        arena::GetMoveDefinition(arena::MoveId::ThunderBolt);
    AdvanceEmpty(game, thunder.startupFrames);
    const arena::Projectile* projectile = FirstActiveProjectile(game);

    bool passed = true;
    passed &= Expect(projectile != nullptr && ActiveProjectileCount(game) == 1,
                     "Thunder Bolt spawns exactly one projectile on its active frame");
    if (projectile == nullptr) {
        return false;
    }
    passed &= Expect(projectile->owner == arena::FighterId::Volt &&
                         projectile->sourceMove == arena::MoveId::ThunderBolt &&
                         projectile->direction == arena::FacingDirection::Right &&
                         projectile->velocity.x > 0.0F,
                     "projectile stores VOLT ownership and captured facing direction");
    const float projectileX = projectile->position.x;
    const float projectileVelocityX = projectile->velocity.x;
    game.advanceSimulation({});
    projectile = FirstActiveProjectile(game);
    passed &= Expect(projectile != nullptr &&
                         NearlyEqual(projectile->position.x,
                                     projectileX + projectileVelocityX *
                                                       static_cast<float>(arena::kFixedDeltaSeconds)),
                     "projectile movement uses fixed simulation delta");

    arena::GameState copiedState = game.state();
    copiedState.fighters[0].facing = arena::FacingDirection::Left;
    passed &= Expect(copiedState.projectiles[0].direction == arena::FacingDirection::Right &&
                         copiedState.projectiles[0].velocity.x == projectileVelocityX,
                     "projectile direction is value state independent of later owner facing");

    for (int tick = 0; tick < thunder.projectileLifetimeFrames &&
                         ActiveProjectileCount(game) > 0;
         ++tick) {
        game.advanceSimulation({});
    }
    passed &= Expect(Volt(game).health == voltHealthBefore,
                     "Thunder Bolt never damages its owner");
    passed &= Expect(Ember(game).health == emberHealthBefore - thunder.damage &&
                         Ember(game).combatState == arena::CombatState::HitStun &&
                         Ember(game).hitStunFrames == thunder.hitStunFrames &&
                         ActiveProjectileCount(game) == 0,
                     "Thunder Bolt damages once, applies hit-stun, and is removed on contact");
    passed &= Expect(Volt(game).energy == 11 && Ember(game).energy == 0,
                     "special projectile contact grants no energy to either fighter");
    return passed;
}

bool TestThunderBoltBlockingAndLifetime() {
    arena::Game blockGame;
    blockGame.initialize();
    AdvanceToFighting(blockGame);
    EarnEnergy(blockGame, 0, 25);
    arena::FrameCommands separate;
    separate.fighters[0].moveLeftHeld = true;
    separate.fighters[1].moveRightHeld = true;
    for (int tick = 0; tick < 35; ++tick) {
        blockGame.advanceSimulation(separate);
    }
    AdvanceUntilBothAvailable(blockGame);
    const int healthBeforeBlock = Ember(blockGame).health;
    PressSpecial(blockGame, 0);
    const arena::MoveDefinition& thunder =
        arena::GetMoveDefinition(arena::MoveId::ThunderBolt);
    AdvanceEmpty(blockGame, thunder.startupFrames);
    arena::FrameCommands block;
    block.fighters[1].blockHeld = true;
    for (int tick = 0; tick < thunder.projectileLifetimeFrames &&
                         ActiveProjectileCount(blockGame) > 0;
         ++tick) {
        blockGame.advanceSimulation(block);
    }

    bool passed = true;
    passed &= Expect(Ember(blockGame).health == healthBeforeBlock &&
                         Ember(blockGame).blockFlashFrames > 0 &&
                         ActiveProjectileCount(blockGame) == 0,
                     "grounded facing block consumes Thunder Bolt without damage");
    passed &= Expect(Volt(blockGame).energy == 11 && Ember(blockGame).energy == 0,
                     "blocked special grants no energy and cannot refund its cost");

    arena::Game lifetimeGame;
    lifetimeGame.initialize();
    AdvanceToFighting(lifetimeGame);
    EarnEnergy(lifetimeGame, 0, 25);
    arena::FrameCommands moveLeft;
    moveLeft.fighters[0].moveLeftHeld = true;
    moveLeft.fighters[1].moveLeftHeld = true;
    for (int tick = 0; tick < arena::kSimulationRate * 4; ++tick) {
        lifetimeGame.advanceSimulation(moveLeft);
    }
    arena::FrameCommands moveEmberRight;
    moveEmberRight.fighters[1].moveRightHeld = true;
    for (int tick = 0; tick < arena::kSimulationRate * 4; ++tick) {
        lifetimeGame.advanceSimulation(moveEmberRight);
    }
    AdvanceUntilBothAvailable(lifetimeGame);
    const int healthBeforeMiss = Ember(lifetimeGame).health;
    PressSpecial(lifetimeGame, 0);
    AdvanceEmpty(lifetimeGame, thunder.startupFrames);
    AdvanceEmpty(lifetimeGame, 68);
    arena::FrameCommands jump;
    jump.fighters[1].jumpPressed = true;
    jump.fighters[1].jumpHeld = true;
    lifetimeGame.advanceSimulation(jump);
    jump.fighters[1].jumpPressed = false;
    for (int tick = 0; tick < 19; ++tick) {
        lifetimeGame.advanceSimulation(jump);
    }
    passed &= Expect(ActiveProjectileCount(lifetimeGame) == 1,
                     "missed Thunder Bolt remains active through lifetime tick 89");
    lifetimeGame.advanceSimulation(jump);
    passed &= Expect(ActiveProjectileCount(lifetimeGame) == 0 &&
                         Ember(lifetimeGame).health == healthBeforeMiss,
                     "missed Thunder Bolt expires exactly after 90 fixed ticks without damage");
    return passed;
}

bool TestProjectilePersistsAfterOwnerHit() {
    arena::Game game;
    game.initialize();
    AdvanceToFighting(game);
    EarnEnergy(game, 0, 25);
    ApproachToBodyContact(game);
    arena::FrameCommands createGap;
    createGap.fighters[1].moveRightHeld = true;
    for (int tick = 0; tick < 13; ++tick) {
        game.advanceSimulation(createGap);
    }

    const int emberHealthBefore = Ember(game).health;
    arena::FrameCommands attacks;
    attacks.fighters[0].specialAttackPressed = true;
    attacks.fighters[1].heavyAttackPressed = true;
    game.advanceSimulation(attacks);
    AdvanceEmpty(game, arena::GetMoveDefinition(arena::MoveId::ThunderBolt).startupFrames);

    bool passed = true;
    passed &= Expect(Volt(game).combatState == arena::CombatState::HitStun &&
                         ActiveProjectileCount(game) == 1,
                     "Thunder Bolt projectile survives its owner's same-tick interruption");
    for (int tick = 0; tick < 8 && ActiveProjectileCount(game) > 0; ++tick) {
        game.advanceSimulation({});
    }
    passed &= Expect(Ember(game).health == emberHealthBefore - 16 &&
                         ActiveProjectileCount(game) == 0,
                     "projectile can connect after its owner has entered hit-stun");
    return passed;
}

bool TestInfernoBurstHitMissAndFacing() {
    const arena::MoveDefinition& inferno =
        arena::GetMoveDefinition(arena::MoveId::InfernoBurst);
    arena::Game hitGame;
    hitGame.initialize();
    AdvanceToFighting(hitGame);
    EarnEnergy(hitGame, 1, 25);
    ApproachToBodyContact(hitGame);
    const int voltHealthBefore = Volt(hitGame).health;
    PressSpecial(hitGame, 1);
    AdvanceEmpty(hitGame, inferno.startupFrames);

    bool passed = true;
    passed &= Expect(Volt(hitGame).health == voltHealthBefore - inferno.damage &&
                         Volt(hitGame).hitStunFrames == inferno.hitStunFrames &&
                         Volt(hitGame).velocity.x < 0.0F,
                     "left-facing Inferno Burst deals damage, stun, and stronger knockback");
    const int healthAfterHit = Volt(hitGame).health;
    AdvanceEmpty(hitGame, inferno.activeFrames + 2);
    passed &= Expect(Volt(hitGame).health == healthAfterHit,
                     "Inferno Burst connects only once during its active duration");

    arena::Fighter rightFacing = Ember(hitGame);
    rightFacing.facing = arena::FacingDirection::Right;
    const arena::Rectanglef leftBox = arena::GetAttackHitbox(
        Ember(hitGame), inferno);
    const arena::Rectanglef rightBox = arena::GetAttackHitbox(rightFacing, inferno);
    passed &= Expect(leftBox.x + leftBox.width <= Ember(hitGame).position.x + 16.0F &&
                         rightBox.x >= rightFacing.position.x - 16.0F,
                     "Inferno Burst hitbox mirrors with fighter facing");

    arena::Game missGame;
    missGame.initialize();
    AdvanceToFighting(missGame);
    EarnEnergy(missGame, 1, 25);
    arena::FrameCommands separate;
    separate.fighters[0].moveLeftHeld = true;
    separate.fighters[1].moveRightHeld = true;
    for (int tick = 0; tick < 45; ++tick) {
        missGame.advanceSimulation(separate);
    }
    AdvanceUntilBothAvailable(missGame);
    const int healthBeforeMiss = Volt(missGame).health;
    PressSpecial(missGame, 1);
    AdvanceEmpty(missGame, inferno.startupFrames + inferno.activeFrames);
    passed &= Expect(Volt(missGame).health == healthBeforeMiss,
                     "Inferno Burst misses outside its short facing-aware range");

    arena::Game blockGame;
    blockGame.initialize();
    AdvanceToFighting(blockGame);
    EarnEnergy(blockGame, 1, 25);
    ApproachToBodyContact(blockGame);
    const int healthBeforeBlock = Volt(blockGame).health;
    arena::FrameCommands startBlockedBurst;
    startBlockedBurst.fighters[0].blockHeld = true;
    startBlockedBurst.fighters[1].specialAttackPressed = true;
    blockGame.advanceSimulation(startBlockedBurst);
    arena::FrameCommands holdBlock;
    holdBlock.fighters[0].blockHeld = true;
    for (int tick = 0; tick < inferno.startupFrames; ++tick) {
        blockGame.advanceSimulation(holdBlock);
    }
    passed &= Expect(Volt(blockGame).health == healthBeforeBlock &&
                         Volt(blockGame).blockFlashFrames > 0,
                     "grounded facing block prevents Inferno Burst damage");
    return passed;
}

bool TestSpecialTradeAndKo() {
    arena::Game tradeGame;
    tradeGame.initialize();
    AdvanceToFighting(tradeGame);
    EarnEnergy(tradeGame, 1, 25);
    ApproachToBodyContact(tradeGame);
    const int voltHealthBeforeTrade = Volt(tradeGame).health;
    const int emberHealthBeforeTrade = Ember(tradeGame).health;
    PressSpecial(tradeGame, 1);
    AdvanceEmpty(tradeGame, 10);
    arena::FrameCommands light;
    light.fighters[0].lightAttackPressed = true;
    tradeGame.advanceSimulation(light);
    AdvanceEmpty(tradeGame, 5);

    bool passed = true;
    passed &= Expect(Volt(tradeGame).combatState == arena::CombatState::HitStun &&
                         Ember(tradeGame).combatState == arena::CombatState::HitStun,
                     "same-tick normal and Inferno Burst contacts both resolve as a trade");
    passed &= Expect(Volt(tradeGame).health == voltHealthBeforeTrade - 20 &&
                         Ember(tradeGame).health == emberHealthBeforeTrade - 6,
                     "trade applies both special and normal damage from the snapshot");

    arena::Game koGame;
    koGame.initialize();
    AdvanceToFighting(koGame);
    EarnEnergy(koGame, 1, 72);
    ApproachToBodyContact(koGame);
    const int healthBeforeSpecial = Volt(koGame).health;
    PressSpecial(koGame, 1);
    AdvanceEmpty(koGame,
                 arena::GetMoveDefinition(arena::MoveId::InfernoBurst).startupFrames);
    passed &= Expect(healthBeforeSpecial <= 20 && Volt(koGame).health == 0 &&
                         Volt(koGame).combatState == arena::CombatState::KnockedOut,
                     "Inferno Burst can clamp health to zero and cause KO");
    return passed;
}

bool TestHealthClampKoAndKoLockout() {
    arena::Game game;
    game.initialize();
    AdvanceToFighting(game);
    for (int attack = 0; attack < 8; ++attack) {
        int safetyTicks = 0;
        while ((Volt(game).combatState != arena::CombatState::Available ||
                Ember(game).combatState != arena::CombatState::Available) &&
               Ember(game).combatState != arena::CombatState::KnockedOut && safetyTicks < 120) {
            game.advanceSimulation({});
            ++safetyTicks;
        }
        ApproachToBodyContact(game);
        AdvanceToFirstActiveFrame(game, 0, arena::MoveId::Heavy);
    }

    bool passed = true;
    passed &= Expect(Ember(game).health == 0, "health clamps at zero after lethal damage");
    passed &= Expect(Ember(game).combatState == arena::CombatState::KnockedOut,
                     "zero health enters KO state");
    arena::FrameCommands koAttack;
    koAttack.fighters[1].lightAttackPressed = true;
    game.advanceSimulation(koAttack);
    passed &= Expect(Ember(game).combatState == arena::CombatState::KnockedOut &&
                         Ember(game).currentMove == arena::MoveId::None,
                     "KO fighter cannot attack");
    return passed;
}

bool TestMatchTimerAndTimeoutRules() {
    arena::Game game({5, 2, 3});
    game.initialize();

    bool passed = true;
    passed &= Expect(game.state().matchPhase == arena::MatchPhase::RoundIntro &&
                         game.state().roundTimerTicksRemaining == 5 &&
                         game.state().phaseTicksRemaining == 2,
                     "a new match begins in a fixed-tick round intro");

    arena::FrameCommands staleAttack;
    staleAttack.fighters[0].lightAttackPressed = true;
    staleAttack.fighters[0].lightAttackHeld = true;
    game.advanceSimulation(staleAttack);
    passed &= Expect(game.state().matchPhase == arena::MatchPhase::RoundIntro &&
                         game.state().roundTimerTicksRemaining == 5,
                     "the round timer does not decrement during intro");
    staleAttack.fighters[0].lightAttackPressed = false;
    game.advanceSimulation(staleAttack);
    passed &= Expect(game.state().matchPhase == arena::MatchPhase::Fighting &&
                         game.state().roundTimerTicksRemaining == 5,
                     "fighting begins only after the exact intro duration");
    game.advanceSimulation(staleAttack);
    passed &= Expect(Volt(game).combatState == arena::CombatState::Available &&
                         game.state().roundTimerTicksRemaining == 4,
                     "an intro attack edge cannot leak into fighting");

    AdvanceEmpty(game, 4);
    passed &= Expect(game.state().matchPhase == arena::MatchPhase::RoundOver &&
                         game.state().roundTimerTicksRemaining == 0 &&
                         game.state().roundEndReason == arena::RoundEndReason::TimeOver &&
                         game.state().roundResult == arena::RoundResult::Draw &&
                         game.state().roundWins == std::array<int, 2>{0, 0},
                     "equal health at the exact timer boundary produces an unscored draw");
    game.advanceSimulation({});
    passed &= Expect(game.state().roundTimerTicksRemaining == 0 &&
                         game.state().roundWins == std::array<int, 2>{0, 0},
                     "the timer cannot become negative and a result is not scored twice");
    AdvanceEmpty(game, 2);
    passed &= Expect(game.state().matchPhase == arena::MatchPhase::RoundIntro &&
                         game.state().roundNumber == 2 &&
                         game.state().roundTimerTicksRemaining == 5,
                     "a drawn round advances to a fresh replay round without awarding a win");

    arena::Game healthLead({200, 0, 2});
    healthLead.initialize();
    AdvanceToFighting(healthLead);
    ApproachToBodyContact(healthLead);
    AdvanceToFirstActiveFrame(healthLead, 0, arena::MoveId::Light);
    const int remainingTicks = healthLead.state().roundTimerTicksRemaining;
    AdvanceEmpty(healthLead, remainingTicks);
    passed &= Expect(healthLead.state().matchPhase == arena::MatchPhase::RoundOver &&
                         healthLead.state().roundResult == arena::RoundResult::VoltWins &&
                         healthLead.state().roundWins == std::array<int, 2>{1, 0},
                     "higher remaining health wins on timeout exactly once");
    return passed;
}

bool TestKoDoubleKoAndTimerBoundary() {
    arena::Game singleKo({2000, 0, 4});
    singleKo.initialize();
    AdvanceToFighting(singleKo);
    KnockOutWithChargedAttacks(singleKo, 0);

    bool passed = true;
    passed &= Expect(singleKo.state().matchPhase == arena::MatchPhase::RoundOver &&
                         Ember(singleKo).health == 0 &&
                         singleKo.state().roundResult == arena::RoundResult::VoltWins &&
                         singleKo.state().roundEndReason == arena::RoundEndReason::Knockout &&
                         singleKo.state().roundWins == std::array<int, 2>{1, 0},
                     "a single KO freezes combat and awards its opponent one round");
    const int voltHealthAfterKo = Volt(singleKo).health;
    arena::FrameCommands attackAfterKo;
    attackAfterKo.fighters[0].lightAttackPressed = true;
    singleKo.advanceSimulation(attackAfterKo);
    passed &= Expect(Volt(singleKo).health == voltHealthAfterKo &&
                         singleKo.state().roundWins == std::array<int, 2>{1, 0},
                     "combat is frozen and scoring is stable after round completion");

    arena::Game doubleKo({3000, 0, 4});
    doubleKo.initialize();
    AdvanceToFighting(doubleKo);
    for (int trade = 0; trade < 4; ++trade) {
        TradeLevelThreeChargedAttacks(doubleKo);
    }
    passed &= Expect(doubleKo.state().matchPhase == arena::MatchPhase::RoundOver &&
                         Volt(doubleKo).health == 0 && Ember(doubleKo).health == 0 &&
                         doubleKo.state().roundResult == arena::RoundResult::Draw &&
                         doubleKo.state().roundEndReason ==
                             arena::RoundEndReason::DoubleKnockout &&
                         doubleKo.state().roundWins == std::array<int, 2>{0, 0},
                     "simultaneous final contacts produce an unscored double KO");

    arena::Game boundaryKo({2000, 0, 4});
    boundaryKo.initialize();
    AdvanceToFighting(boundaryKo);
    for (int attack = 0; attack < 3; ++attack) {
        AdvanceUntilBothAvailable(boundaryKo);
        ApproachToBodyContact(boundaryKo);
        ChargeAndRelease(boundaryKo, 0, 60);
        AdvanceEmpty(boundaryKo,
                     arena::GetMoveDefinition(arena::MoveId::ThunderStrikeLevel3)
                         .startupFrames);
    }
    AdvanceUntilBothAvailable(boundaryKo);
    ApproachToBodyContact(boundaryKo);
    constexpr int kFinalAttackTicks = 69;
    AdvanceEmpty(boundaryKo,
                 boundaryKo.state().roundTimerTicksRemaining - kFinalAttackTicks);
    ChargeAndRelease(boundaryKo, 0, 60);
    AdvanceEmpty(boundaryKo,
                 arena::GetMoveDefinition(arena::MoveId::ThunderStrikeLevel3)
                     .startupFrames);
    passed &= Expect(boundaryKo.state().roundTimerTicksRemaining == 0 &&
                         Ember(boundaryKo).health == 0 &&
                         boundaryKo.state().roundEndReason ==
                             arena::RoundEndReason::Knockout &&
                         boundaryKo.state().roundResult == arena::RoundResult::VoltWins,
                     "KO resolution takes precedence on the timer-expiration tick");
    return passed;
}

bool TestRoundResetScoringMatchOverAndRestart() {
    arena::Game game({4000, 1, 3});
    game.initialize();
    AdvanceToFighting(game);
    KnockOutWithChargedAttacks(game, 0);

    bool passed = true;
    passed &= Expect(game.state().roundWins == std::array<int, 2>{1, 0} &&
                         game.state().roundNumber == 1,
                     "VOLT receives the first round win");
    game.advanceSimulation({});
    game.advanceSimulation({});
    passed &= Expect(game.state().roundWins == std::array<int, 2>{1, 0},
                     "round-over ticks do not repeat the score award");

    arena::FrameCommands staleEdge;
    staleEdge.fighters[1].lightAttackPressed = true;
    staleEdge.fighters[1].lightAttackHeld = true;
    game.advanceSimulation(staleEdge);
    passed &= Expect(game.state().matchPhase == arena::MatchPhase::RoundIntro &&
                         game.state().roundNumber == 2 && Volt(game).health == 100 &&
                         Ember(game).health == 100 && Volt(game).energy == 0 &&
                         Ember(game).energy == 0 && NearlyEqual(Volt(game).position.x, 360.0F) &&
                         NearlyEqual(Ember(game).position.x, 920.0F) &&
                         NearlyEqual(Volt(game).velocity.x, 0.0F) &&
                         NearlyEqual(Ember(game).velocity.x, 0.0F) &&
                         Volt(game).facing == arena::FacingDirection::Right &&
                         Ember(game).facing == arena::FacingDirection::Left &&
                         Volt(game).combatState == arena::CombatState::Available &&
                         Ember(game).combatState == arena::CombatState::Available &&
                         Volt(game).currentMove == arena::MoveId::None &&
                         Ember(game).currentMove == arena::MoveId::None &&
                         Volt(game).attackPhase == arena::AttackPhase::None &&
                         Ember(game).attackPhase == arena::AttackPhase::None &&
                         Volt(game).hitStunFrames == 0 && Ember(game).hitStunFrames == 0 &&
                         Volt(game).chargeFrames == 0 && Ember(game).chargeFrames == 0 &&
                         Volt(game).hitFlashFrames == 0 && Ember(game).hitFlashFrames == 0 &&
                         ActiveProjectileCount(game) == 0,
                     "round reset restores fighters, transients, positions, facing, and projectiles");
    staleEdge.fighters[1].lightAttackPressed = false;
    game.advanceSimulation(staleEdge);
    game.advanceSimulation(staleEdge);
    passed &= Expect(game.state().matchPhase == arena::MatchPhase::Fighting &&
                         Ember(game).combatState == arena::CombatState::Available,
                     "a held attack from round-over cannot become a new-round press");

    KnockOutWithChargedAttacks(game, 1);
    passed &= Expect(game.state().roundWins == std::array<int, 2>{1, 1},
                     "EMBER can equalize the round score");
    AdvanceEmpty(game, 3);
    AdvanceToFighting(game);
    passed &= Expect(game.state().roundNumber == 3,
                     "one win each advances to a deciding round");

    KnockOutWithChargedAttacks(game, 0);
    const int timerAtMatchPoint = game.state().roundTimerTicksRemaining;
    AdvanceEmpty(game, 3);
    passed &= Expect(game.state().matchPhase == arena::MatchPhase::MatchOver &&
                         game.state().matchWinner == arena::RoundResult::VoltWins &&
                         game.state().roundWins == std::array<int, 2>{2, 1},
                     "the first fighter to two wins enters match over with a winner");
    arena::FrameCommands rejectedAttack;
    rejectedAttack.fighters[1].heavyAttackPressed = true;
    game.advanceSimulation(rejectedAttack);
    passed &= Expect(game.state().matchPhase == arena::MatchPhase::MatchOver &&
                         game.state().roundTimerTicksRemaining == timerAtMatchPoint &&
                         Ember(game).combatState == arena::CombatState::KnockedOut,
                     "match over freezes attacks and the round timer");

    arena::FrameCommands restart;
    restart.restartPressed = true;
    game.advanceSimulation(restart);
    passed &= Expect(game.state().matchPhase == arena::MatchPhase::RoundIntro &&
                         game.state().matchWinner == arena::RoundResult::None &&
                         game.state().roundWins == std::array<int, 2>{0, 0} &&
                         game.state().roundNumber == 1 && Volt(game).health == 100 &&
                         Ember(game).health == 100 && Volt(game).energy == 0 &&
                         Ember(game).energy == 0 &&
                         game.state().roundTimerTicksRemaining == 4000,
                     "R or Enter command starts one fully reset new match");
    game.advanceSimulation({});
    passed &= Expect(game.state().matchPhase == arena::MatchPhase::Fighting &&
                         game.state().roundWins == std::array<int, 2>{0, 0},
                     "the consumed restart command does not trigger again");
    return passed;
}

bool TestProjectileClearedAtRoundBoundary() {
    arena::Game game({1200, 0, 2});
    game.initialize();
    AdvanceToFighting(game);
    EarnEnergy(game, 0, 25);

    arena::FrameCommands separate;
    separate.fighters[0].moveLeftHeld = true;
    separate.fighters[1].moveRightHeld = true;
    for (int tick = 0; tick < 30; ++tick) {
        game.advanceSimulation(separate);
    }
    AdvanceEmpty(game, game.state().roundTimerTicksRemaining - 13);
    PressSpecial(game, 0);
    AdvanceEmpty(game, arena::GetMoveDefinition(arena::MoveId::ThunderBolt).startupFrames);

    return Expect(game.state().matchPhase == arena::MatchPhase::RoundOver &&
                      game.state().roundEndReason == arena::RoundEndReason::TimeOver &&
                      ActiveProjectileCount(game) == 0,
                  "timeout clears a projectile created on the final fighting tick");
}

bool FightersEqual(const arena::Fighter& first, const arena::Fighter& second) {
    return first.id == second.id && first.position.x == second.position.x &&
           first.position.y == second.position.y && first.velocity.x == second.velocity.x &&
           first.velocity.y == second.velocity.y && first.movementSpeed == second.movementSpeed &&
           first.jumpVelocity == second.jumpVelocity && first.grounded == second.grounded &&
           first.facing == second.facing && first.collisionWidth == second.collisionWidth &&
           first.collisionHeight == second.collisionHeight && first.health == second.health &&
           first.maximumHealth == second.maximumHealth && first.energy == second.energy &&
           first.maximumEnergy == second.maximumEnergy &&
           first.combatState == second.combatState && first.currentMove == second.currentMove &&
           first.attackPhase == second.attackPhase &&
           first.combatStateFrame == second.combatStateFrame &&
           first.hitStunFrames == second.hitStunFrames &&
           first.chargeFrames == second.chargeFrames &&
           first.chargeReleaseOutcome == second.chargeReleaseOutcome &&
           first.attackHasConnected == second.attackHasConnected &&
           first.moveEffectCreated == second.moveEffectCreated &&
           first.hitFlashFrames == second.hitFlashFrames &&
           first.blockFlashFrames == second.blockFlashFrames &&
           first.specialImpactMove == second.specialImpactMove &&
           first.specialImpactFrames == second.specialImpactFrames;
}

bool ProjectilesEqual(const arena::Projectile& first, const arena::Projectile& second) {
    return first.active == second.active && first.owner == second.owner &&
           first.sourceMove == second.sourceMove && first.position.x == second.position.x &&
           first.position.y == second.position.y && first.velocity.x == second.velocity.x &&
           first.velocity.y == second.velocity.y && first.direction == second.direction &&
           first.remainingLifetimeFrames == second.remainingLifetimeFrames &&
           first.hitbox.x == second.hitbox.x && first.hitbox.y == second.hitbox.y &&
           first.hitbox.width == second.hitbox.width &&
           first.hitbox.height == second.hitbox.height;
}

bool CompleteStatesEqual(const arena::Game& first, const arena::Game& second) {
    if (!FightersEqual(Volt(first), Volt(second)) ||
        !FightersEqual(Ember(first), Ember(second)) ||
        first.state().roundWins != second.state().roundWins ||
        first.state().matchPhase != second.state().matchPhase ||
        first.state().roundResult != second.state().roundResult ||
        first.state().matchWinner != second.state().matchWinner ||
        first.state().roundEndReason != second.state().roundEndReason ||
        first.state().roundNumber != second.state().roundNumber ||
        first.state().roundTimerTicksRemaining !=
            second.state().roundTimerTicksRemaining ||
        first.state().phaseTicksRemaining != second.state().phaseTicksRemaining ||
        first.state().simulationFrame != second.state().simulationFrame) {
        return false;
    }
    for (std::size_t index = 0; index < first.state().projectiles.size(); ++index) {
        if (!ProjectilesEqual(first.state().projectiles[index],
                              second.state().projectiles[index])) {
            return false;
        }
    }
    return true;
}

bool TestMatchTransitionReplayConsistency() {
    arena::Game first({80, 1, 1});
    arena::Game second({80, 1, 1});
    first.initialize();
    second.initialize();

    bool passed = true;
    bool restartSent = false;
    bool sawMatchOver = false;
    int fightingTick = 0;
    arena::MatchPhase previousPhase = first.state().matchPhase;
    for (int tick = 0; tick < 260; ++tick) {
        if (first.state().matchPhase != previousPhase) {
            if (first.state().matchPhase == arena::MatchPhase::Fighting) {
                fightingTick = 0;
            }
            previousPhase = first.state().matchPhase;
        }

        arena::FrameCommands commands;
        if (first.state().matchPhase == arena::MatchPhase::Fighting) {
            commands.fighters[0].moveRightHeld = fightingTick < 60;
            commands.fighters[1].moveLeftHeld = fightingTick < 60;
            commands.fighters[0].lightAttackPressed = fightingTick == 60;
            commands.fighters[0].lightAttackHeld =
                fightingTick >= 60 && fightingTick < 66;
            ++fightingTick;
        } else if (first.state().matchPhase == arena::MatchPhase::MatchOver &&
                   !restartSent) {
            commands.restartPressed = true;
            restartSent = true;
            sawMatchOver = true;
        }

        first.advanceSimulation(commands);
        second.advanceSimulation(commands);
        passed &= Expect(CompleteStatesEqual(first, second),
                         "match command replay reproduces every transition state");
        if (!passed) {
            return false;
        }
    }

    passed &= Expect(sawMatchOver && restartSent,
                     "match replay fixture includes match over and rematch transitions");
    return passed;
}

bool TestFixedTickReplayConsistency() {
    arena::Game first;
    arena::Game second;
    first.initialize();
    AdvanceToFighting(first);
    second.initialize();
    AdvanceToFighting(second);

    bool passed = true;
    for (int tick = 0; tick < arena::kSimulationRate * 5; ++tick) {
        arena::FrameCommands commands;
        commands.fighters[0].moveRightHeld = tick < 60 || (tick >= 150 && tick < 190);
        commands.fighters[1].moveLeftHeld = tick >= 20 && tick < 80;
        commands.fighters[0].jumpPressed = tick == 20 || tick == 180;
        commands.fighters[0].jumpHeld = (tick >= 20 && tick < 45) ||
                                        (tick >= 180 && tick < 205);
        commands.fighters[1].jumpPressed = tick == 95;
        commands.fighters[1].jumpHeld = tick >= 95 && tick < 120;
        commands.fighters[0].lightAttackPressed = tick == 70 || tick == 220;
        commands.fighters[0].lightAttackHeld = tick >= 70 && tick < 76;
        commands.fighters[1].heavyAttackPressed = tick == 135;
        commands.fighters[1].heavyAttackHeld = tick >= 135 && tick < 150;
        commands.fighters[1].blockHeld = tick >= 215 && tick < 240;
        commands.fighters[0].chargedAttackPressed = tick == 250;
        commands.fighters[0].chargedAttackHeld = tick >= 250 && tick < 270;
        commands.fighters[0].chargedAttackReleased = tick == 270;
        commands.fighters[1].specialAttackPressed = tick == 280;
        commands.fighters[1].specialAttackHeld = tick >= 280 && tick < 283;
        commands.fighters[1].specialAttackReleased = tick == 283;

        first.advanceSimulation(commands);
        second.advanceSimulation(commands);
        passed &= Expect(FightersEqual(Volt(first), Volt(second)) &&
                             FightersEqual(Ember(first), Ember(second)),
                         "identical per-tick commands reproduce all fighter state");
        if (!passed) {
            return false;
        }
    }

    passed &= Expect(first.state().simulationFrame ==
                         arena::kDefaultRoundIntroDurationTicks +
                             arena::kSimulationRate * 5,
                     "five fighting seconds follow the fixed-tick round intro");
    passed &= Expect(first.state().simulationFrame == second.state().simulationFrame,
                     "matching replays retain the same frame number");
    return passed;
}

bool TestSpecialReplayConsistency() {
    arena::Game first;
    arena::Game second;
    first.initialize();
    AdvanceToFighting(first);
    second.initialize();
    AdvanceToFighting(second);
    EarnEnergy(first, 0, 25);
    EarnEnergy(first, 1, 25);
    EarnEnergy(second, 0, 25);
    EarnEnergy(second, 1, 25);

    arena::FrameCommands separate;
    separate.fighters[0].moveLeftHeld = true;
    separate.fighters[1].moveRightHeld = true;
    for (int tick = 0; tick < 40; ++tick) {
        first.advanceSimulation(separate);
        second.advanceSimulation(separate);
    }

    bool passed = true;
    for (int tick = 0; tick < 140; ++tick) {
        arena::FrameCommands commands;
        commands.fighters[0].specialAttackPressed = tick == 0;
        commands.fighters[0].specialAttackHeld = tick >= 0 && tick < 4;
        commands.fighters[0].specialAttackReleased = tick == 4;
        commands.fighters[1].specialAttackPressed = tick == 0;
        commands.fighters[1].specialAttackHeld = tick >= 0 && tick < 4;
        commands.fighters[1].specialAttackReleased = tick == 4;
        commands.fighters[1].blockHeld = tick >= 45 && tick < 100;

        first.advanceSimulation(commands);
        second.advanceSimulation(commands);
        passed &= Expect(CompleteStatesEqual(first, second),
                         "special replay reproduces fighter and projectile state each tick");
        if (!passed) {
            return false;
        }
        if (tick == 0) {
            passed &= Expect(Volt(first).currentMove == arena::MoveId::ThunderBolt &&
                                 Ember(first).currentMove == arena::MoveId::InfernoBurst &&
                                 Volt(first).energy == 11 && Ember(first).energy == 11,
                             "simultaneous specials both activate and spend without bias");
        }
    }
    return passed;
}

}  // namespace

int main() {
    bool passed = true;
    passed &= TestInitialStateAndMoveData();
    passed &= TestHorizontalMovementAndOpposingInput();
    passed &= TestJumpGravityLandingAndHeldJump();
    passed &= TestStageBoundaries();
    passed &= TestSimultaneousMovementAndCollision();
    passed &= TestFacingUpdatesAfterAirborneCrossOver();
    passed &= TestAttackLifecycle(arena::MoveId::Light,
                                  "light press begins startup frame one");
    passed &= TestAttackLifecycle(arena::MoveId::Heavy,
                                  "heavy press begins startup frame one");
    passed &= TestDamageOneHitAndHitStunLockout();
    passed &= TestBlocking();
    passed &= TestInvalidAttackDuringRecovery();
    passed &= TestHorizontalAndVerticalMisses();
    passed &= TestHitboxOrientation();
    passed &= TestSimultaneousTrade();
    passed &= TestKnockbackAtStageBoundary();
    passed &= TestHeldAttackDoesNotRepeat();
    passed &= TestEnergyRulesAndSpecialFoundation();
    passed &= TestFrameCommandEdgeBuffer();
    passed &= TestChargeThresholdsAndMaximum();
    passed &= TestChargedMoveDataAndLevelBoundaries();
    passed &= TestChargedDamageTimingAndEconomy();
    passed &= TestChargedBlockMissFacingAndLockouts();
    passed &= TestChargedTradesAndOtherMoveInteractions();
    passed &= TestChargedKoLockoutAndReplayConsistency();
    passed &= TestChargeInterruptionAndCombatLockouts();
    passed &= TestSpecialActivationEnergyAndLifecycle();
    passed &= TestSpecialSpendingLockoutsAndInterruption();
    passed &= TestThunderBoltProjectileStateAndDamage();
    passed &= TestThunderBoltBlockingAndLifetime();
    passed &= TestProjectilePersistsAfterOwnerHit();
    passed &= TestInfernoBurstHitMissAndFacing();
    passed &= TestSpecialTradeAndKo();
    passed &= TestHealthClampKoAndKoLockout();
    passed &= TestMatchTimerAndTimeoutRules();
    passed &= TestKoDoubleKoAndTimerBoundary();
    passed &= TestRoundResetScoringMatchOverAndRestart();
    passed &= TestProjectileClearedAtRoundBoundary();
    passed &= TestMatchTransitionReplayConsistency();
    passed &= TestFixedTickReplayConsistency();
    passed &= TestSpecialReplayConsistency();
    return passed ? 0 : 1;
}
