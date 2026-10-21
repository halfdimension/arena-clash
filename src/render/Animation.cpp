#include "render/Animation.hpp"

#include "combat/Combat.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <utility>

namespace arena {
namespace {

constexpr int kFrameWidth = 1254;
constexpr int kFrameHeight = 1254;
constexpr float kMovementThreshold = 0.01F;

std::size_t AnimationIndex(AnimationId animationId) {
    return static_cast<std::size_t>(animationId);
}

AnimationFrameDefinition MakeFrame(std::string texturePath, int durationTicks,
                                   Vector2f groundPivotPixels,
                                   AttackPhase attackPhase = AttackPhase::None) {
    return {std::move(texturePath), durationTicks, kFrameWidth, kFrameHeight,
            groundPivotPixels, attackPhase};
}

AnimationClipDefinition MakeSingleFrameClip(AnimationId id, AnimationPlaybackMode playbackMode,
                                            const std::string& fighterPath,
                                            const std::string& clipPath, int durationTicks,
                                            Vector2f groundPivotPixels) {
    return {id, playbackMode,
            {MakeFrame(fighterPath + "/" + clipPath + "/000.png", durationTicks,
                       groundPivotPixels)}};
}

AnimationClipDefinition MakeAttackClip(AnimationId id, const std::string& fighterPath,
                                       const std::string& clipPath, MoveId moveId,
                                       Vector2f groundPivotPixels) {
    const MoveDefinition& move = GetMoveDefinition(moveId);
    const std::string basePath = fighterPath + "/" + clipPath;
    return {
        id,
        AnimationPlaybackMode::OneShot,
        {
            MakeFrame(basePath + "/startup.png", move.startupFrames, groundPivotPixels,
                      AttackPhase::Startup),
            MakeFrame(basePath + "/active.png", move.activeFrames, groundPivotPixels,
                      AttackPhase::Active),
            MakeFrame(basePath + "/recovery.png", move.recoveryFrames, groundPivotPixels,
                      AttackPhase::Recovery),
        },
    };
}

FighterAnimationSet MakeAnimationSet(FighterId fighterId) {
    const bool isVolt = fighterId == FighterId::Volt;
    const std::string fighterPath =
        isVolt ? "assets/characters/volt" : "assets/characters/ember";
    const Vector2f groundPivot = isVolt ? Vector2f{708.0F, 1209.0F}
                                        : Vector2f{704.0F, 1221.0F};
    const MoveId specialMove = isVolt ? MoveId::ThunderBolt : MoveId::InfernoBurst;
    const MoveId chargedMove = isVolt ? MoveId::ThunderStrikeLevel1
                                      : MoveId::FlameSmashLevel1;

    FighterAnimationSet animationSet{
        {},
        MakeFrame(fighterPath + "/neutral.png", 1, groundPivot),
        isVolt ? 0.27F : 0.26F,
    };
    auto& clips = animationSet.clips;
    clips[AnimationIndex(AnimationId::Idle)] =
        MakeSingleFrameClip(AnimationId::Idle, AnimationPlaybackMode::Looping,
                            fighterPath, "idle", 6, groundPivot);
    clips[AnimationIndex(AnimationId::Walking)] =
        MakeSingleFrameClip(AnimationId::Walking, AnimationPlaybackMode::Looping,
                            fighterPath, "walking", 6, groundPivot);
    clips[AnimationIndex(AnimationId::Jumping)] =
        MakeSingleFrameClip(AnimationId::Jumping, AnimationPlaybackMode::OneShot,
                            fighterPath, "jumping", 6, groundPivot);
    clips[AnimationIndex(AnimationId::Blocking)] =
        MakeSingleFrameClip(AnimationId::Blocking, AnimationPlaybackMode::Looping,
                            fighterPath, "blocking", 6, groundPivot);
    clips[AnimationIndex(AnimationId::LightAttack)] =
        MakeAttackClip(AnimationId::LightAttack, fighterPath, "light_attack", MoveId::Light,
                       groundPivot);
    clips[AnimationIndex(AnimationId::HeavyAttack)] =
        MakeAttackClip(AnimationId::HeavyAttack, fighterPath, "heavy_attack", MoveId::Heavy,
                       groundPivot);
    clips[AnimationIndex(AnimationId::HitReaction)] =
        MakeSingleFrameClip(AnimationId::HitReaction, AnimationPlaybackMode::OneShot,
                            fighterPath, "hit_reaction", 5, groundPivot);
    clips[AnimationIndex(AnimationId::Charging)] =
        MakeSingleFrameClip(AnimationId::Charging, AnimationPlaybackMode::Looping,
                            fighterPath, "charging", 6, groundPivot);
    clips[AnimationIndex(AnimationId::SpecialAttack)] =
        MakeAttackClip(AnimationId::SpecialAttack, fighterPath, "special_attack", specialMove,
                       groundPivot);
    clips[AnimationIndex(AnimationId::ChargedAttack)] =
        MakeAttackClip(AnimationId::ChargedAttack, fighterPath, "charged_attack", chargedMove,
                       groundPivot);
    clips[AnimationIndex(AnimationId::Knockout)] =
        MakeSingleFrameClip(AnimationId::Knockout, AnimationPlaybackMode::OneShot,
                            fighterPath, "ko", 1, groundPivot);
    return animationSet;
}

AnimationFrameSelection SelectFramesInRange(const AnimationClipDefinition& clip,
                                             std::size_t firstFrame,
                                             std::size_t frameCount,
                                             std::uint64_t elapsedTicks,
                                             AnimationPlaybackMode playbackMode) {
    assert(frameCount > 0);
    std::uint64_t totalDuration = 0;
    for (std::size_t index = firstFrame; index < firstFrame + frameCount; ++index) {
        totalDuration += static_cast<std::uint64_t>(std::max(1, clip.frames[index].durationTicks));
    }

    const bool completed = playbackMode == AnimationPlaybackMode::OneShot &&
                           elapsedTicks >= totalDuration;
    std::uint64_t clipTick = elapsedTicks;
    if (playbackMode == AnimationPlaybackMode::Looping) {
        clipTick %= totalDuration;
    } else if (completed) {
        return {firstFrame + frameCount - 1, true};
    }

    for (std::size_t index = firstFrame; index < firstFrame + frameCount; ++index) {
        const std::uint64_t frameDuration =
            static_cast<std::uint64_t>(std::max(1, clip.frames[index].durationTicks));
        if (clipTick < frameDuration) {
            return {index, false};
        }
        clipTick -= frameDuration;
    }
    return {firstFrame + frameCount - 1, completed};
}

}  // namespace

const FighterAnimationSet& GetFighterAnimationSet(FighterId fighterId) {
    static const FighterAnimationSet voltAnimations = MakeAnimationSet(FighterId::Volt);
    static const FighterAnimationSet emberAnimations = MakeAnimationSet(FighterId::Ember);
    return fighterId == FighterId::Volt ? voltAnimations : emberAnimations;
}

const AnimationClipDefinition& GetAnimationClip(const FighterAnimationSet& animationSet,
                                                AnimationId animationId) {
    const AnimationClipDefinition& clip = animationSet.clips[AnimationIndex(animationId)];
    assert(clip.id == animationId);
    assert(!clip.frames.empty());
    return clip;
}

AnimationId SelectFighterAnimation(const Fighter& fighter) {
    switch (fighter.combatState) {
        case CombatState::Attacking:
            if (fighter.currentMove == MoveId::Light) {
                return AnimationId::LightAttack;
            }
            if (fighter.currentMove == MoveId::Heavy) {
                return AnimationId::HeavyAttack;
            }
            return IsChargedMove(fighter.currentMove) ? AnimationId::ChargedAttack
                                                      : AnimationId::SpecialAttack;
        case CombatState::Blocking:
            return AnimationId::Blocking;
        case CombatState::Charging:
            return AnimationId::Charging;
        case CombatState::HitStun:
            return AnimationId::HitReaction;
        case CombatState::KnockedOut:
            return AnimationId::Knockout;
        case CombatState::Available:
            if (!fighter.grounded) {
                return AnimationId::Jumping;
            }
            if (std::fabs(fighter.velocity.x) >= kMovementThreshold) {
                return AnimationId::Walking;
            }
            return AnimationId::Idle;
    }
    return AnimationId::Idle;
}

void UpdateAnimationPlayback(AnimationPlaybackState& playback, AnimationId animationId,
                             std::uint64_t simulationFrame) {
    if (!playback.initialized || playback.animationId != animationId ||
        simulationFrame < playback.lastSimulationFrame) {
        playback.animationId = animationId;
        playback.elapsedTicks = 0;
        playback.lastSimulationFrame = simulationFrame;
        playback.initialized = true;
        return;
    }

    playback.elapsedTicks += simulationFrame - playback.lastSimulationFrame;
    playback.lastSimulationFrame = simulationFrame;
}

AnimationFrameSelection SelectAnimationFrame(const AnimationClipDefinition& clip,
                                             std::uint64_t elapsedTicks) {
    assert(!clip.frames.empty());
    return SelectFramesInRange(clip, 0, clip.frames.size(), elapsedTicks,
                               clip.playbackMode);
}

AnimationFrameSelection SelectFighterAnimationFrame(const AnimationClipDefinition& clip,
                                                    const Fighter& fighter,
                                                    std::uint64_t elapsedTicks) {
    if (fighter.combatState != CombatState::Attacking ||
        fighter.attackPhase == AttackPhase::None) {
        return SelectAnimationFrame(clip, elapsedTicks);
    }

    std::size_t firstFrame = clip.frames.size();
    std::size_t frameCount = 0;
    for (std::size_t index = 0; index < clip.frames.size(); ++index) {
        if (clip.frames[index].attackPhase == fighter.attackPhase) {
            if (firstFrame == clip.frames.size()) {
                firstFrame = index;
            }
            ++frameCount;
        } else if (frameCount > 0) {
            break;
        }
    }

    if (frameCount == 0) {
        return SelectAnimationFrame(clip, elapsedTicks);
    }
    return SelectFramesInRange(clip, firstFrame, frameCount,
                               static_cast<std::uint64_t>(
                                   std::max(0, fighter.combatStateFrame - 1)),
                               AnimationPlaybackMode::OneShot);
}

const AnimationFrameDefinition& ResolveAnimationFrame(
    const AnimationFrameDefinition& authoredFrame,
    const AnimationFrameDefinition& fallbackFrame, bool authoredFrameAvailable) {
    return authoredFrameAvailable ? authoredFrame : fallbackFrame;
}

SpriteDrawGeometry CalculateSpriteDrawGeometry(const AnimationFrameDefinition& frame,
                                               float scaleX, float scaleY,
                                               bool facingRight) {
    const float width = static_cast<float>(frame.widthPixels);
    const float height = static_cast<float>(frame.heightPixels);
    const float pivotX = facingRight ? frame.groundPivotPixels.x
                                     : width - frame.groundPivotPixels.x;
    return {
        facingRight ? width : -width,
        height,
        width * scaleX,
        height * scaleY,
        pivotX * scaleX,
        frame.groundPivotPixels.y * scaleY,
    };
}

}  // namespace arena
