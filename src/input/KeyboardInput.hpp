#pragma once

#include "input/FrameCommands.hpp"

namespace arena {

class KeyboardInput {
public:
    void sample();
    FrameCommands consumeForSimulationTick();
    bool consumeDebugHitboxToggle();

private:
    FrameCommands pendingCommands_{};
    bool debugHitboxTogglePending_ = false;
};

}  // namespace arena
