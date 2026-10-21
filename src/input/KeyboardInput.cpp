#include "input/KeyboardInput.hpp"

#include "raylib.h"

namespace arena {

void KeyboardInput::sample() {
    debugHitboxTogglePending_ = debugHitboxTogglePending_ || IsKeyPressed(KEY_F1);

    FighterCommand& volt = pendingCommands_.fighters[0];
    volt.moveLeftHeld = IsKeyDown(KEY_A);
    volt.moveRightHeld = IsKeyDown(KEY_D);
    volt.jumpHeld = IsKeyDown(KEY_W);
    volt.jumpPressed = volt.jumpPressed || IsKeyPressed(KEY_W);
    volt.blockHeld = IsKeyDown(KEY_S);
    volt.lightAttackHeld = IsKeyDown(KEY_F);
    volt.lightAttackPressed = volt.lightAttackPressed || IsKeyPressed(KEY_F);
    volt.heavyAttackHeld = IsKeyDown(KEY_G);
    volt.heavyAttackPressed = volt.heavyAttackPressed || IsKeyPressed(KEY_G);

    FighterCommand& ember = pendingCommands_.fighters[1];
    ember.moveLeftHeld = IsKeyDown(KEY_LEFT);
    ember.moveRightHeld = IsKeyDown(KEY_RIGHT);
    ember.jumpHeld = IsKeyDown(KEY_UP);
    ember.jumpPressed = ember.jumpPressed || IsKeyPressed(KEY_UP);
    ember.blockHeld = IsKeyDown(KEY_DOWN);
    ember.lightAttackHeld = IsKeyDown(KEY_K);
    ember.lightAttackPressed = ember.lightAttackPressed || IsKeyPressed(KEY_K);
    ember.heavyAttackHeld = IsKeyDown(KEY_L);
    ember.heavyAttackPressed = ember.heavyAttackPressed || IsKeyPressed(KEY_L);
}

FrameCommands KeyboardInput::consumeForSimulationTick() {
    const FrameCommands commands = pendingCommands_;
    pendingCommands_.fighters[0].jumpPressed = false;
    pendingCommands_.fighters[1].jumpPressed = false;
    pendingCommands_.fighters[0].lightAttackPressed = false;
    pendingCommands_.fighters[1].lightAttackPressed = false;
    pendingCommands_.fighters[0].heavyAttackPressed = false;
    pendingCommands_.fighters[1].heavyAttackPressed = false;
    return commands;
}

bool KeyboardInput::consumeDebugHitboxToggle() {
    const bool toggle = debugHitboxTogglePending_;
    debugHitboxTogglePending_ = false;
    return toggle;
}

}  // namespace arena
