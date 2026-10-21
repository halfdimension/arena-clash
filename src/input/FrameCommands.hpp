#pragma once

#include <array>

namespace arena {

struct FighterCommand {
    bool moveLeftHeld = false;
    bool moveRightHeld = false;
    bool jumpHeld = false;
    bool jumpPressed = false;
    bool blockHeld = false;
    bool lightAttackHeld = false;
    bool lightAttackPressed = false;
    bool heavyAttackHeld = false;
    bool heavyAttackPressed = false;
    bool specialAttackHeld = false;
    bool specialAttackPressed = false;
    bool specialAttackReleased = false;
    bool chargedAttackHeld = false;
    bool chargedAttackPressed = false;
    bool chargedAttackReleased = false;
};

struct FrameCommands {
    std::array<FighterCommand, 2> fighters{};
    bool restartPressed = false;
};

}  // namespace arena
