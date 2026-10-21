#pragma once

#include "input/FrameCommandBuffer.hpp"

namespace arena {

class KeyboardInput {
public:
    void sample();
    FrameCommands consumeForSimulationTick();
    bool consumeDebugHitboxToggle();

private:
    FrameCommandBuffer commandBuffer_{};
    bool debugHitboxTogglePending_ = false;
};

}  // namespace arena
