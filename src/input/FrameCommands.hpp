#pragma once

#include <array>

namespace arena {

struct FighterCommand {
    bool moveLeftHeld = false;
    bool moveRightHeld = false;
    bool jumpHeld = false;
    bool jumpPressed = false;
};

struct FrameCommands {
    std::array<FighterCommand, 2> fighters{};
};

}  // namespace arena
