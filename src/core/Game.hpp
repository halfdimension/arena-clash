#pragma once

#include "game/Fighter.hpp"
#include "input/FrameCommands.hpp"

#include <array>
#include <cstddef>
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
inline constexpr std::size_t kMaximumProjectiles = 8;
inline constexpr int kDefaultRoundDurationTicks = 90 * kSimulationRate;
inline constexpr int kDefaultRoundIntroDurationTicks = 3 * kSimulationRate;
inline constexpr int kDefaultRoundOverDurationTicks = 3 * kSimulationRate;
inline constexpr int kRoundWinsRequired = 2;

enum class MatchPhase {
    RoundIntro,
    Fighting,
    RoundOver,
    MatchOver,
};

enum class RoundResult {
    None,
    VoltWins,
    EmberWins,
    Draw,
};

enum class RoundEndReason {
    None,
    Knockout,
    DoubleKnockout,
    TimeOver,
};

struct MatchRules {
    int roundDurationTicks = kDefaultRoundDurationTicks;
    int roundIntroDurationTicks = kDefaultRoundIntroDurationTicks;
    int roundOverDurationTicks = kDefaultRoundOverDurationTicks;
};

struct Projectile {
    bool active = false;
    FighterId owner = FighterId::Volt;
    MoveId sourceMove = MoveId::None;
    Vector2f position{};
    Vector2f velocity{};
    FacingDirection direction = FacingDirection::Right;
    int remainingLifetimeFrames = 0;
    Rectanglef hitbox{};
};

struct GameState {
    std::array<Fighter, 2> fighters;
    std::array<Projectile, kMaximumProjectiles> projectiles;
    std::array<int, 2> roundWins{};
    MatchPhase matchPhase = MatchPhase::RoundIntro;
    RoundResult roundResult = RoundResult::None;
    RoundResult matchWinner = RoundResult::None;
    RoundEndReason roundEndReason = RoundEndReason::None;
    int roundNumber = 1;
    int roundTimerTicksRemaining = kDefaultRoundDurationTicks;
    int phaseTicksRemaining = kDefaultRoundIntroDurationTicks;
    std::uint64_t simulationFrame = 0;
};

class Game {
public:
    Game() = default;
    explicit Game(MatchRules matchRules);

    void initialize();
    void advanceSimulation(const FrameCommands& commands);

    const GameState& state() const;

private:
    MatchRules matchRules_;
    GameState state_{};
};

}  // namespace arena
