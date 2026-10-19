#include "core/Game.hpp"

#include <algorithm>
#include <array>

namespace arena {
namespace {

constexpr float kVoltMovementSpeed = 360.0F;
constexpr float kVoltJumpVelocity = 780.0F;
constexpr float kEmberMovementSpeed = 300.0F;
constexpr float kEmberJumpVelocity = 740.0F;
constexpr float kFighterCollisionWidth = 64.0F;
constexpr float kFighterCollisionHeight = 112.0F;

Fighter MakeFighter(FighterId id, float positionX, float movementSpeed, float jumpVelocity,
                    FacingDirection facing) {
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
        1000,
        1000,
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

void ConstrainToArena(Fighter& fighter) {
    const float unconstrainedX = fighter.position.x;
    fighter.position.x = std::clamp(
        fighter.position.x, MinimumPositionX(fighter), MaximumPositionX(fighter));

    if (fighter.position.x != unconstrainedX) {
        fighter.velocity.x = 0.0F;
    }
}

void AdvanceFighter(Fighter& fighter, const FighterCommand& command) {
    const int horizontalAxis = HorizontalAxis(command);
    const float controlFactor = fighter.grounded ? 1.0F : kAirControlFactor;
    fighter.velocity.x = static_cast<float>(horizontalAxis) * fighter.movementSpeed * controlFactor;

    if (command.jumpPressed && fighter.grounded) {
        fighter.velocity.y = -fighter.jumpVelocity;
        fighter.grounded = false;
    }

    fighter.position.x += fighter.velocity.x * static_cast<float>(kFixedDeltaSeconds);
    ConstrainToArena(fighter);

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

    AdvanceFighter(state_.fighters[0], commands.fighters[0]);
    AdvanceFighter(state_.fighters[1], commands.fighters[1]);
    ResolveFighterCollision(state_, previousPositionsX);
    UpdateFacingDirections(state_);
    ++state_.simulationFrame;
}

const GameState& Game::state() const {
    return state_;
}

}  // namespace arena
