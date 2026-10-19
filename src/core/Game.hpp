#pragma once

#include "game/Fighter.hpp"
#include "input/FrameCommands.hpp"

#include <array>
#include <cstdint>

namespace arena {

inline constexpr int kLogicalWidth = 1280;
inline constexpr int kLogicalHeight = 720;
inline constexpr int kSimulationRate = 60;
inline constexpr double kFixedDeltaSeconds = 1.0 / static_cast<double>(kSimulationRate);
inline constexpr float kArenaLeftBoundary = 80.0F;
inline constexpr float kArenaRightBoundary = 1200.0F;
inline constexpr float kArenaGroundY = 564.0F;
inline constexpr float kGravity = 2200.0F;
inline constexpr float kAirControlFactor = 0.75F;

struct GameState {
    std::array<Fighter, 2> fighters;
    std::uint64_t simulationFrame;
};

class Game {
public:
    void initialize();
    void advanceSimulation(const FrameCommands& commands);

    const GameState& state() const;

private:
    GameState state_{};
};

}  // namespace arena
