#pragma once

#include "input/FrameCommands.hpp"

namespace arena {

class KeyboardInput {
public:
    void sample();
    FrameCommands consumeForSimulationTick();

private:
    FrameCommands pendingCommands_{};
};

}  // namespace arena
