#include "core/Game.hpp"
#include "render/Animation.hpp"

#include <cmath>
#include <iostream>

namespace {

constexpr float kTolerance = 0.001F;

bool Expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        return false;
    }
    return true;
}

bool NearlyEqual(float first, float second) {
    return std::fabs(first - second) <= kTolerance;
}

arena::Fighter MakeVolt() {
    arena::Game game;
    game.initialize();
    return game.state().fighters[0];
}

bool TestClipDefinitionsAndStateMapping() {
    const arena::FighterAnimationSet& animations =
        arena::GetFighterAnimationSet(arena::FighterId::Volt);
    bool passed = true;
    for (std::size_t index = 0; index < animations.clips.size(); ++index) {
        const auto animationId = static_cast<arena::AnimationId>(index);
        const arena::AnimationClipDefinition& clip =
            arena::GetAnimationClip(animations, animationId);
        passed &= Expect(clip.id == animationId && !clip.frames.empty(),
                         "every supported animation has an ordered clip definition");
        for (const arena::AnimationFrameDefinition& frame : clip.frames) {
            passed &= Expect(frame.durationTicks > 0 && frame.widthPixels > 0 &&
                                 frame.heightPixels > 0,
                             "every animation frame has duration and dimensions");
        }
    }

    arena::Fighter fighter = MakeVolt();
    passed &= Expect(arena::SelectFighterAnimation(fighter) == arena::AnimationId::Idle,
                     "stationary available fighter selects idle");
    fighter.velocity.x = 20.0F;
    passed &= Expect(arena::SelectFighterAnimation(fighter) == arena::AnimationId::Walking,
                     "moving grounded fighter selects walking");
    fighter.grounded = false;
    passed &= Expect(arena::SelectFighterAnimation(fighter) == arena::AnimationId::Jumping,
                     "airborne fighter selects jumping");
    fighter.grounded = true;
    fighter.combatState = arena::CombatState::Blocking;
    passed &= Expect(arena::SelectFighterAnimation(fighter) == arena::AnimationId::Blocking,
                     "blocking state selects blocking");
    fighter.combatState = arena::CombatState::Charging;
    passed &= Expect(arena::SelectFighterAnimation(fighter) == arena::AnimationId::Charging,
                     "charging state selects charging");
    fighter.combatState = arena::CombatState::HitStun;
    passed &= Expect(arena::SelectFighterAnimation(fighter) == arena::AnimationId::HitReaction,
                     "hit stun selects hit reaction");
    fighter.combatState = arena::CombatState::KnockedOut;
    passed &= Expect(arena::SelectFighterAnimation(fighter) == arena::AnimationId::Knockout,
                     "knockout state selects knockout");
    fighter.combatState = arena::CombatState::Attacking;
    fighter.currentMove = arena::MoveId::Light;
    passed &= Expect(arena::SelectFighterAnimation(fighter) == arena::AnimationId::LightAttack,
                     "light move selects light attack");
    fighter.currentMove = arena::MoveId::Heavy;
    passed &= Expect(arena::SelectFighterAnimation(fighter) == arena::AnimationId::HeavyAttack,
                     "heavy move selects heavy attack");
    fighter.currentMove = arena::MoveId::ThunderBolt;
    passed &= Expect(arena::SelectFighterAnimation(fighter) == arena::AnimationId::SpecialAttack,
                     "special move selects special attack");
    fighter.currentMove = arena::MoveId::ThunderStrikeLevel3;
    passed &= Expect(arena::SelectFighterAnimation(fighter) == arena::AnimationId::ChargedAttack,
                     "charged move selects charged attack");
    return passed;
}

bool TestFrameSelectionAndCompletion() {
    const arena::AnimationFrameDefinition first = {
        "first.png", 2, 100, 80, {30.0F, 75.0F}, arena::AttackPhase::None};
    const arena::AnimationFrameDefinition second = {
        "second.png", 3, 100, 80, {31.0F, 75.0F}, arena::AttackPhase::None};
    const arena::AnimationClipDefinition looping = {
        arena::AnimationId::Walking, arena::AnimationPlaybackMode::Looping, {first, second}};
    const arena::AnimationClipDefinition oneShot = {
        arena::AnimationId::Jumping, arena::AnimationPlaybackMode::OneShot, {first, second}};

    bool passed = true;
    passed &= Expect(arena::SelectAnimationFrame(looping, 0).frameIndex == 0 &&
                         arena::SelectAnimationFrame(looping, 1).frameIndex == 0 &&
                         arena::SelectAnimationFrame(looping, 2).frameIndex == 1 &&
                         arena::SelectAnimationFrame(looping, 4).frameIndex == 1 &&
                         arena::SelectAnimationFrame(looping, 5).frameIndex == 0,
                     "looping frame durations wrap on their exact tick boundary");
    passed &= Expect(arena::SelectAnimationFrame(oneShot, 4).frameIndex == 1 &&
                         !arena::SelectAnimationFrame(oneShot, 4).completed,
                     "one-shot displays its final frame for its full duration");
    passed &= Expect(arena::SelectAnimationFrame(oneShot, 5).frameIndex == 1 &&
                         arena::SelectAnimationFrame(oneShot, 5).completed &&
                         arena::SelectAnimationFrame(oneShot, 100).completed,
                     "one-shot clamps to its final frame and reports completion");

    arena::AnimationPlaybackState playback;
    arena::UpdateAnimationPlayback(playback, arena::AnimationId::Idle, 20);
    arena::UpdateAnimationPlayback(playback, arena::AnimationId::Idle, 24);
    passed &= Expect(playback.elapsedTicks == 4,
                     "presentation playback follows fixed simulation tick deltas");
    arena::UpdateAnimationPlayback(playback, arena::AnimationId::Walking, 25);
    passed &= Expect(playback.elapsedTicks == 0,
                     "changing animation resets presentation playback");
    return passed;
}

bool TestFallbackAndPivotGeometry() {
    const arena::AnimationFrameDefinition authored = {
        "authored.png", 4, 100, 80, {30.0F, 75.0F}, arena::AttackPhase::None};
    const arena::AnimationFrameDefinition fallback = {
        "neutral.png", 1, 120, 90, {44.0F, 86.0F}, arena::AttackPhase::None};

    bool passed = true;
    passed &= Expect(&arena::ResolveAnimationFrame(authored, fallback, false) == &fallback,
                     "unavailable authored frame resolves to neutral fallback metadata");
    passed &= Expect(&arena::ResolveAnimationFrame(authored, fallback, true) == &authored,
                     "available authored frame retains its own metadata");

    const arena::SpriteDrawGeometry right =
        arena::CalculateSpriteDrawGeometry(authored, 0.5F, 0.25F, true);
    const arena::SpriteDrawGeometry left =
        arena::CalculateSpriteDrawGeometry(authored, 0.5F, 0.25F, false);
    passed &= Expect(NearlyEqual(right.sourceWidth, 100.0F) &&
                         NearlyEqual(left.sourceWidth, -100.0F) &&
                         NearlyEqual(right.destinationWidth, left.destinationWidth),
                     "facing changes only the signed source width");
    passed &= Expect(NearlyEqual(right.originX, 15.0F) &&
                         NearlyEqual(left.originX, 35.0F) &&
                         NearlyEqual(right.originY, 18.75F) &&
                         NearlyEqual(left.originY, 18.75F),
                     "mirrored pivot and scaled vertical pivot preserve the ground anchor");
    return passed;
}

bool TestCombatPhaseSynchronization() {
    const arena::AnimationClipDefinition phaseTimingClip = {
        arena::AnimationId::LightAttack,
        arena::AnimationPlaybackMode::OneShot,
        {
            {"startup_1.png", 2, 100, 80, {30.0F, 75.0F}, arena::AttackPhase::Startup},
            {"startup_2.png", 3, 100, 80, {30.0F, 75.0F}, arena::AttackPhase::Startup},
            {"active.png", 2, 100, 80, {30.0F, 75.0F}, arena::AttackPhase::Active},
            {"recovery.png", 2, 100, 80, {30.0F, 75.0F}, arena::AttackPhase::Recovery},
        },
    };
    arena::Fighter phaseFighter = MakeVolt();
    phaseFighter.combatState = arena::CombatState::Attacking;
    phaseFighter.attackPhase = arena::AttackPhase::Startup;
    phaseFighter.combatStateFrame = 1;
    bool passed = true;
    passed &= Expect(arena::SelectFighterAnimationFrame(phaseTimingClip, phaseFighter, 1000)
                             .frameIndex == 0,
                     "attack phase frame one starts at the first authored frame");
    phaseFighter.combatStateFrame = 2;
    passed &= Expect(arena::SelectFighterAnimationFrame(phaseTimingClip, phaseFighter, 1000)
                             .frameIndex == 0,
                     "attack frame duration includes its final authoritative phase tick");
    phaseFighter.combatStateFrame = 3;
    passed &= Expect(arena::SelectFighterAnimationFrame(phaseTimingClip, phaseFighter, 1000)
                             .frameIndex == 1,
                     "attack frame advances on the following authoritative phase tick");
    phaseFighter.attackPhase = arena::AttackPhase::Active;
    phaseFighter.combatStateFrame = 1;
    passed &= Expect(arena::SelectFighterAnimationFrame(phaseTimingClip, phaseFighter, 1000)
                             .frameIndex == 2,
                     "combat phase overrides unrelated presentation elapsed time");

    arena::Game game;
    game.initialize();
    while (game.state().matchPhase != arena::MatchPhase::Fighting) {
        game.advanceSimulation({});
    }

    arena::FrameCommands commands;
    commands.fighters[0].lightAttackPressed = true;
    commands.fighters[0].lightAttackHeld = true;
    game.advanceSimulation(commands);

    const arena::AnimationClipDefinition& clip = arena::GetAnimationClip(
        arena::GetFighterAnimationSet(arena::FighterId::Volt),
        arena::AnimationId::LightAttack);
    const arena::MoveDefinition& move = arena::GetMoveDefinition(arena::MoveId::Light);
    const int attackTicks = move.startupFrames + move.activeFrames + move.recoveryFrames;
    for (int tick = 0; tick < attackTicks; ++tick) {
        const arena::Fighter& fighter = game.state().fighters[0];
        if (fighter.combatState != arena::CombatState::Attacking) {
            break;
        }
        const arena::AnimationFrameSelection selection =
            arena::SelectFighterAnimationFrame(clip, fighter, 1000);
        passed &= Expect(clip.frames[selection.frameIndex].attackPhase == fighter.attackPhase,
                         "attack frame always belongs to the authoritative combat phase");
        game.advanceSimulation({});
    }
    passed &= Expect(game.state().fighters[0].combatState == arena::CombatState::Available,
                     "animation sampling does not alter the combat lifecycle");
    return passed;
}

}  // namespace

int main() {
    bool passed = true;
    passed &= TestClipDefinitionsAndStateMapping();
    passed &= TestFrameSelectionAndCompletion();
    passed &= TestFallbackAndPivotGeometry();
    passed &= TestCombatPhaseSynchronization();
    return passed ? 0 : 1;
}
