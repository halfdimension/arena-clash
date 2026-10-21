#pragma once

#include "input/FrameCommands.hpp"

namespace arena {

// Accumulates render-frame input edges until exactly one simulation tick consumes them.
class FrameCommandBuffer {
public:
    void sample(const FrameCommands& sampledCommands);
    FrameCommands consumeForSimulationTick();

private:
    FrameCommands pendingCommands_{};
};

}  // namespace arena
