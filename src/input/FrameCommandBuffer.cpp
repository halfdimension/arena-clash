#include "input/FrameCommandBuffer.hpp"

#include <cstddef>

namespace arena {
namespace {

void AccumulateCommand(FighterCommand& pending, const FighterCommand& sampled) {
    pending.moveLeftHeld = sampled.moveLeftHeld;
    pending.moveRightHeld = sampled.moveRightHeld;
    pending.jumpHeld = sampled.jumpHeld;
    pending.blockHeld = sampled.blockHeld;
    pending.lightAttackHeld = sampled.lightAttackHeld;
    pending.heavyAttackHeld = sampled.heavyAttackHeld;
    pending.specialAttackHeld = sampled.specialAttackHeld;
    pending.chargedAttackHeld = sampled.chargedAttackHeld;

    pending.jumpPressed = pending.jumpPressed || sampled.jumpPressed;
    pending.lightAttackPressed = pending.lightAttackPressed || sampled.lightAttackPressed;
    pending.heavyAttackPressed = pending.heavyAttackPressed || sampled.heavyAttackPressed;
    pending.specialAttackPressed = pending.specialAttackPressed || sampled.specialAttackPressed;
    pending.specialAttackReleased =
        pending.specialAttackReleased || sampled.specialAttackReleased;
    pending.chargedAttackPressed = pending.chargedAttackPressed || sampled.chargedAttackPressed;
    pending.chargedAttackReleased =
        pending.chargedAttackReleased || sampled.chargedAttackReleased;
}

void ClearEdges(FighterCommand& command) {
    command.jumpPressed = false;
    command.lightAttackPressed = false;
    command.heavyAttackPressed = false;
    command.specialAttackPressed = false;
    command.specialAttackReleased = false;
    command.chargedAttackPressed = false;
    command.chargedAttackReleased = false;
}

}  // namespace

void FrameCommandBuffer::sample(const FrameCommands& sampledCommands) {
    for (std::size_t fighterIndex = 0; fighterIndex < pendingCommands_.fighters.size();
         ++fighterIndex) {
        AccumulateCommand(pendingCommands_.fighters[fighterIndex],
                          sampledCommands.fighters[fighterIndex]);
    }
    pendingCommands_.restartPressed =
        pendingCommands_.restartPressed || sampledCommands.restartPressed;
}

FrameCommands FrameCommandBuffer::consumeForSimulationTick() {
    const FrameCommands commands = pendingCommands_;
    for (FighterCommand& command : pendingCommands_.fighters) {
        ClearEdges(command);
    }
    pendingCommands_.restartPressed = false;
    return commands;
}

}  // namespace arena
