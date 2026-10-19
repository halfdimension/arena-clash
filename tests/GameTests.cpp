#include "core/Game.hpp"

#include <iostream>

namespace {

bool Expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        return false;
    }
    return true;
}

}  // namespace

int main() {
    arena::Game game;
    game.initialize();

    bool passed = true;
    passed &= Expect(game.state().simulationFrame == 0, "initial frame is zero");
    passed &= Expect(game.state().fighters[0].id == arena::FighterId::Volt, "player one is Volt");
    passed &= Expect(game.state().fighters[1].id == arena::FighterId::Ember, "player two is Ember");
    passed &= Expect(game.state().fighters[0].health == game.state().fighters[0].maximumHealth,
                     "Volt starts at full health");
    passed &= Expect(game.state().fighters[1].health == game.state().fighters[1].maximumHealth,
                     "Ember starts at full health");

    for (int tick = 0; tick < arena::kSimulationRate * 2; ++tick) {
        game.advanceSimulation(arena::kFixedDeltaSeconds);
    }

    passed &= Expect(game.state().simulationFrame == 120, "two seconds advance 120 ticks");
    return passed ? 0 : 1;
}
