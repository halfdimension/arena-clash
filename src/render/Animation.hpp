#pragma once

#include "game/Fighter.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace arena {

enum class AnimationId {
    Idle,
    Walking,
    Jumping,
    Blocking,
    LightAttack,
    HeavyAttack,
    HitReaction,
    Charging,
    SpecialAttack,
    ChargedAttack,
    Knockout,
    Count,
};

enum class AnimationPlaybackMode {
    Looping,
    OneShot,
};

struct AnimationFrameDefinition {
    std::string texturePath;
    int durationTicks;
    int widthPixels;
    int heightPixels;
    Vector2f groundPivotPixels;
    AttackPhase attackPhase = AttackPhase::None;
};

struct AnimationClipDefinition {
    AnimationId id;
    AnimationPlaybackMode playbackMode;
    std::vector<AnimationFrameDefinition> frames;
};

struct FighterAnimationSet {
    std::array<AnimationClipDefinition, static_cast<std::size_t>(AnimationId::Count)> clips;
    AnimationFrameDefinition fallbackFrame;
    float scale;
};

struct AnimationFrameSelection {
    std::size_t frameIndex = 0;
    bool completed = false;
};

struct AnimationPlaybackState {
    AnimationId animationId = AnimationId::Idle;
    std::uint64_t elapsedTicks = 0;
    std::uint64_t lastSimulationFrame = 0;
    bool initialized = false;
};

struct SpriteDrawGeometry {
    float sourceWidth;
    float sourceHeight;
    float destinationWidth;
    float destinationHeight;
    float originX;
    float originY;
};

const FighterAnimationSet& GetFighterAnimationSet(FighterId fighterId);
const AnimationClipDefinition& GetAnimationClip(const FighterAnimationSet& animationSet,
                                                AnimationId animationId);
AnimationId SelectFighterAnimation(const Fighter& fighter);
void UpdateAnimationPlayback(AnimationPlaybackState& playback, AnimationId animationId,
                             std::uint64_t simulationFrame);
AnimationFrameSelection SelectAnimationFrame(const AnimationClipDefinition& clip,
                                             std::uint64_t elapsedTicks);
AnimationFrameSelection SelectFighterAnimationFrame(const AnimationClipDefinition& clip,
                                                    const Fighter& fighter,
                                                    std::uint64_t elapsedTicks);
const AnimationFrameDefinition& ResolveAnimationFrame(
    const AnimationFrameDefinition& authoredFrame,
    const AnimationFrameDefinition& fallbackFrame, bool authoredFrameAvailable);
SpriteDrawGeometry CalculateSpriteDrawGeometry(const AnimationFrameDefinition& frame,
                                               float scaleX, float scaleY,
                                               bool facingRight);

}  // namespace arena
