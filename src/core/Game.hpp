#pragma once

#include <array>
#include <cstdint>

namespace arena {

inline constexpr int kLogicalWidth = 1280;
inline constexpr int kLogicalHeight = 720;
inline constexpr int kSimulationRate = 60;
inline constexpr double kFixedDeltaSeconds = 1.0 / static_cast<double>(kSimulationRate);

enum class FighterId {
    Volt,
    Ember,
};

struct FighterState {
    FighterId id;
    float positionX;
    float positionY;
    int health;
    int maximumHealth;
};

struct GameState {
    std::array<FighterState, 2> fighters;
    std::uint64_t simulationFrame;
};

class Game {
public:
    void initialize();
    void advanceSimulation(double fixedDeltaSeconds);

    const GameState& state() const;

private:
    GameState state_{};
};

}  // namespace arena
