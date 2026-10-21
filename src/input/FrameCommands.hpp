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
};

struct FrameCommands {
    std::array<FighterCommand, 2> fighters{};
};

}  // namespace arena
