#include "input/KeyboardInput.hpp"

#include "raylib.h"

namespace arena {

void KeyboardInput::sample() {
    FighterCommand& volt = pendingCommands_.fighters[0];
    volt.moveLeftHeld = IsKeyDown(KEY_A);
    volt.moveRightHeld = IsKeyDown(KEY_D);
    volt.jumpHeld = IsKeyDown(KEY_W);
    volt.jumpPressed = volt.jumpPressed || IsKeyPressed(KEY_W);

    FighterCommand& ember = pendingCommands_.fighters[1];
    ember.moveLeftHeld = IsKeyDown(KEY_LEFT);
    ember.moveRightHeld = IsKeyDown(KEY_RIGHT);
    ember.jumpHeld = IsKeyDown(KEY_UP);
    ember.jumpPressed = ember.jumpPressed || IsKeyPressed(KEY_UP);
}

FrameCommands KeyboardInput::consumeForSimulationTick() {
    const FrameCommands commands = pendingCommands_;
    pendingCommands_.fighters[0].jumpPressed = false;
    pendingCommands_.fighters[1].jumpPressed = false;
    return commands;
}

}  // namespace arena
