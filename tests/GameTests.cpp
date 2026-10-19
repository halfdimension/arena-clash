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

bool TestInitialState() {
    arena::Game game;
    game.initialize();

    bool passed = true;
    passed &= Expect(game.state().simulationFrame == 0, "initial frame is zero");
    passed &= Expect(Volt(game).id == arena::FighterId::Volt, "player one is Volt");
    passed &= Expect(Ember(game).id == arena::FighterId::Ember, "player two is Ember");
    passed &= Expect(Volt(game).grounded && Ember(game).grounded, "fighters start grounded");
    passed &= Expect(Volt(game).facing == arena::FacingDirection::Right,
                     "Volt initially faces Ember");
    passed &= Expect(Ember(game).facing == arena::FacingDirection::Left,
                     "Ember initially faces Volt");
    passed &= Expect(Volt(game).health == Volt(game).maximumHealth, "Volt starts at full health");
    passed &= Expect(Ember(game).health == Ember(game).maximumHealth,
                     "Ember starts at full health");
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

bool FightersEqual(const arena::Fighter& first, const arena::Fighter& second) {
    return first.id == second.id && first.position.x == second.position.x &&
           first.position.y == second.position.y && first.velocity.x == second.velocity.x &&
           first.velocity.y == second.velocity.y && first.movementSpeed == second.movementSpeed &&
           first.jumpVelocity == second.jumpVelocity && first.grounded == second.grounded &&
           first.facing == second.facing && first.collisionWidth == second.collisionWidth &&
           first.collisionHeight == second.collisionHeight && first.health == second.health &&
           first.maximumHealth == second.maximumHealth;
}

bool TestFixedTickConsistency() {
    arena::Game first;
    arena::Game second;
    first.initialize();
    second.initialize();

    bool passed = true;
    for (int tick = 0; tick < arena::kSimulationRate * 4; ++tick) {
        arena::FrameCommands commands;
        commands.fighters[0].moveRightHeld = tick < 80;
        commands.fighters[1].moveLeftHeld = tick >= 30 && tick < 110;
        commands.fighters[0].jumpPressed = tick == 20 || tick == 100;
        commands.fighters[0].jumpHeld = tick >= 20 && tick < 45;
        commands.fighters[1].jumpPressed = tick == 65;
        commands.fighters[1].jumpHeld = tick >= 65 && tick < 90;

        first.advanceSimulation(commands);
        second.advanceSimulation(commands);
        passed &= Expect(FightersEqual(Volt(first), Volt(second)) &&
                             FightersEqual(Ember(first), Ember(second)),
                         "identical commands produce identical fighter state per tick");
        if (!passed) {
            return false;
        }
    }

    passed &= Expect(first.state().simulationFrame == arena::kSimulationRate * 4,
                     "four seconds advance exactly 240 fixed ticks");
    passed &= Expect(first.state().simulationFrame == second.state().simulationFrame,
                     "matching simulations retain the same frame number");
    return passed;
}

}  // namespace

int main() {
    bool passed = true;
    passed &= TestInitialState();
    passed &= TestHorizontalMovementAndOpposingInput();
    passed &= TestJumpGravityLandingAndHeldJump();
    passed &= TestStageBoundaries();
    passed &= TestSimultaneousMovementAndCollision();
    passed &= TestFacingUpdatesAfterAirborneCrossOver();
    passed &= TestFixedTickConsistency();
    return passed ? 0 : 1;
}
