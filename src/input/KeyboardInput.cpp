#include "input/KeyboardInput.hpp"

#include "raylib.h"

namespace arena {

void KeyboardInput::sample() {
    debugHitboxTogglePending_ = debugHitboxTogglePending_ || IsKeyPressed(KEY_F1);

    FrameCommands sampledCommands;
    sampledCommands.restartPressed = IsKeyPressed(KEY_R) || IsKeyPressed(KEY_ENTER);
    FighterCommand& volt = sampledCommands.fighters[0];
    volt.moveLeftHeld = IsKeyDown(KEY_A);
    volt.moveRightHeld = IsKeyDown(KEY_D);
    volt.jumpHeld = IsKeyDown(KEY_W);
    volt.jumpPressed = IsKeyPressed(KEY_W);
    volt.blockHeld = IsKeyDown(KEY_S);
    volt.lightAttackHeld = IsKeyDown(KEY_F);
    volt.lightAttackPressed = IsKeyPressed(KEY_F);
    volt.heavyAttackHeld = IsKeyDown(KEY_G);
    volt.heavyAttackPressed = IsKeyPressed(KEY_G);
    volt.specialAttackHeld = IsKeyDown(KEY_H);
    volt.specialAttackPressed = IsKeyPressed(KEY_H);
    volt.specialAttackReleased = IsKeyReleased(KEY_H);
    volt.chargedAttackHeld = IsKeyDown(KEY_J);
    volt.chargedAttackPressed = IsKeyPressed(KEY_J);
    volt.chargedAttackReleased = IsKeyReleased(KEY_J);

    FighterCommand& ember = sampledCommands.fighters[1];
    ember.moveLeftHeld = IsKeyDown(KEY_LEFT);
    ember.moveRightHeld = IsKeyDown(KEY_RIGHT);
    ember.jumpHeld = IsKeyDown(KEY_UP);
    ember.jumpPressed = IsKeyPressed(KEY_UP);
    ember.blockHeld = IsKeyDown(KEY_DOWN);
    ember.lightAttackHeld = IsKeyDown(KEY_K);
    ember.lightAttackPressed = IsKeyPressed(KEY_K);
    ember.heavyAttackHeld = IsKeyDown(KEY_L);
    ember.heavyAttackPressed = IsKeyPressed(KEY_L);
    ember.specialAttackHeld = IsKeyDown(KEY_SEMICOLON);
    ember.specialAttackPressed = IsKeyPressed(KEY_SEMICOLON);
    ember.specialAttackReleased = IsKeyReleased(KEY_SEMICOLON);
    ember.chargedAttackHeld = IsKeyDown(KEY_APOSTROPHE);
    ember.chargedAttackPressed = IsKeyPressed(KEY_APOSTROPHE);
    ember.chargedAttackReleased = IsKeyReleased(KEY_APOSTROPHE);

    commandBuffer_.sample(sampledCommands);
}

FrameCommands KeyboardInput::consumeForSimulationTick() {
    return commandBuffer_.consumeForSimulationTick();
}

bool KeyboardInput::consumeDebugHitboxToggle() {
    const bool toggle = debugHitboxTogglePending_;
    debugHitboxTogglePending_ = false;
    return toggle;
}

}  // namespace arena
