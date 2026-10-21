#include "combat/Combat.hpp"
#include "core/Game.hpp"

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

bool TestInitialStateAndMoveData() {
    arena::Game game;
    game.initialize();
    const arena::MoveDefinition& light = arena::GetMoveDefinition(arena::MoveId::Light);
    const arena::MoveDefinition& heavy = arena::GetMoveDefinition(arena::MoveId::Heavy);

    bool passed = true;
    passed &= Expect(game.state().simulationFrame == 0, "initial frame is zero");
    passed &= Expect(Volt(game).id == arena::FighterId::Volt, "player one is Volt");
    passed &= Expect(Ember(game).id == arena::FighterId::Ember, "player two is Ember");
    passed &= Expect(Volt(game).grounded && Ember(game).grounded, "fighters start grounded");
    passed &= Expect(Volt(game).facing == arena::FacingDirection::Right,
                     "Volt initially faces Ember");
    passed &= Expect(Ember(game).facing == arena::FacingDirection::Left,
                     "Ember initially faces Volt");
    passed &= Expect(Volt(game).health == 100 && Ember(game).health == 100,
                     "fighters start at 100 health");
    passed &= Expect(Volt(game).combatState == arena::CombatState::Available &&
                         Ember(game).combatState == arena::CombatState::Available,
                     "fighters start combat-available");
    passed &= Expect(light.damage == 6 && light.startupFrames == 5 &&
                         light.activeFrames == 3 && light.recoveryFrames == 10,
                     "light move data matches Phase 1C values");
    passed &= Expect(heavy.damage == 14 && heavy.startupFrames == 12 &&
                         heavy.activeFrames == 4 && heavy.recoveryFrames == 17,
                     "heavy move data matches Phase 1C values");
    return passed;
}

bool TestHorizontalMovementAndOpposingInput() {
    arena::Game game;
    game.initialize();
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
    ApproachToBodyContact(game);
    AdvanceToFirstActiveFrame(game, 0, arena::MoveId::Light);

    bool passed = true;
    passed &= Expect(Ember(game).health == 94, "light attack deals six damage");
    passed &= Expect(Volt(game).health == 100, "attack does not damage its owner");
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
    AdvanceEmpty(game, 4);
    passed &= Expect(Ember(game).health == healthAfterFirstActiveFrame,
                     "one activation cannot hit again on later active frames");
    return passed;
}

bool TestBlocking() {
    arena::Game game;
    game.initialize();
    ApproachToBodyContact(game);
    AdvanceToFirstActiveFrame(game, 0, arena::MoveId::Heavy, true);

    bool passed = true;
    passed &= Expect(Ember(game).health == 100, "grounded front block prevents all damage");
    passed &= Expect(Ember(game).combatState == arena::CombatState::Blocking,
                     "held block remains active after guarding");
    passed &= Expect(Ember(game).blockFlashFrames > 0, "blocked hit emits block feedback");

    arena::FrameCommands blockAndAttack;
    blockAndAttack.fighters[1].blockHeld = true;
    blockAndAttack.fighters[1].lightAttackPressed = true;
    game.advanceSimulation(blockAndAttack);
    passed &= Expect(Ember(game).combatState == arena::CombatState::Blocking &&
                         Ember(game).currentMove == arena::MoveId::None,
                     "blocking cannot attack simultaneously");
    return passed;
}

bool TestInvalidAttackDuringRecovery() {
    arena::Game game;
    game.initialize();
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
    AdvanceToFirstActiveFrame(horizontalGame, 0, arena::MoveId::Heavy);
    bool passed = true;
    passed &= Expect(Ember(horizontalGame).health == 100,
                     "attack misses outside horizontal range");

    arena::Game verticalGame;
    verticalGame.initialize();
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
    ApproachToBodyContact(game);

    arena::FrameCommands attacks;
    attacks.fighters[0].lightAttackPressed = true;
    attacks.fighters[1].lightAttackPressed = true;
    game.advanceSimulation(attacks);
    AdvanceEmpty(game, arena::GetMoveDefinition(arena::MoveId::Light).startupFrames);

    bool passed = true;
    passed &= Expect(Volt(game).health == 94 && Ember(game).health == 94,
                     "same-tick valid attacks both deal damage");
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

bool TestHealthClampKoAndKoLockout() {
    arena::Game game;
    game.initialize();
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

bool FightersEqual(const arena::Fighter& first, const arena::Fighter& second) {
    return first.id == second.id && first.position.x == second.position.x &&
           first.position.y == second.position.y && first.velocity.x == second.velocity.x &&
           first.velocity.y == second.velocity.y && first.movementSpeed == second.movementSpeed &&
           first.jumpVelocity == second.jumpVelocity && first.grounded == second.grounded &&
           first.facing == second.facing && first.collisionWidth == second.collisionWidth &&
           first.collisionHeight == second.collisionHeight && first.health == second.health &&
           first.maximumHealth == second.maximumHealth &&
           first.combatState == second.combatState && first.currentMove == second.currentMove &&
           first.attackPhase == second.attackPhase &&
           first.combatStateFrame == second.combatStateFrame &&
           first.hitStunFrames == second.hitStunFrames &&
           first.attackHasConnected == second.attackHasConnected &&
           first.hitFlashFrames == second.hitFlashFrames &&
           first.blockFlashFrames == second.blockFlashFrames;
}

bool TestFixedTickReplayConsistency() {
    arena::Game first;
    arena::Game second;
    first.initialize();
    second.initialize();

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

        first.advanceSimulation(commands);
        second.advanceSimulation(commands);
        passed &= Expect(FightersEqual(Volt(first), Volt(second)) &&
                             FightersEqual(Ember(first), Ember(second)),
                         "identical per-tick commands reproduce all fighter state");
        if (!passed) {
            return false;
        }
    }

    passed &= Expect(first.state().simulationFrame == arena::kSimulationRate * 5,
                     "five seconds advance exactly 300 fixed ticks");
    passed &= Expect(first.state().simulationFrame == second.state().simulationFrame,
                     "matching replays retain the same frame number");
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
    passed &= TestHealthClampKoAndKoLockout();
    passed &= TestFixedTickReplayConsistency();
    return passed ? 0 : 1;
}
