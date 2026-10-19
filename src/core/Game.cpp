#include "core/Game.hpp"

#include <cassert>

namespace arena {

void Game::initialize() {
    state_ = {
        {{{FighterId::Volt, 360.0F, 552.0F, 1000, 1000},
          {FighterId::Ember, 920.0F, 552.0F, 1000, 1000}}},
        0,
    };
}

void Game::advanceSimulation(double fixedDeltaSeconds) {
    // A strict tick duration keeps future gameplay rules independent of render timing.
    assert(fixedDeltaSeconds == kFixedDeltaSeconds);
    (void)fixedDeltaSeconds;
    ++state_.simulationFrame;
}

const GameState& Game::state() const {
    return state_;
}

}  // namespace arena
