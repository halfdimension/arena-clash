# Project Status

**Current phase:** Phase 2B-A — Procedural idle animation prototype (complete)

**Phase state:** Static neutral sprites and renderer-only idle breathing are verified in Linux builds and a Chrome/WebAssembly runtime; frame-based/skeletal animation is not implemented, and parent Phase 1D still lacks ultimates

**Last updated:** 2026-10-21

This file is the source of truth for the current verified state. Roadmap and game-design documents may describe work that does not exist yet.

## Implemented components

- All Phase 1A–1D-C movement, normal combat, directional guard, health/energy, specials, projectiles, charging, and charged attacks remain in the fixed 60 Hz simulation.
- VOLT's **Thunder Strike** retains 8/3/18 startup/active/recovery, a 96×60 hitbox, 16/23/30 level damage, and 440/500/560 knockback. EMBER's **Flame Smash** retains 13/4/23 timing, a 76×72 hitbox, 16/23/30 damage, and 560/680/800 knockback.
- `GameState` now owns `RoundIntro`, `Fighting`, `RoundOver`, and `MatchOver` phases; the current round/result/reason; round wins; match winner; and integer tick counters.
- Each round has a 180-tick intro and 5,400-tick (90-second) fighting timer. The timer decrements only in `Fighting`, never becomes negative, and is displayed using the authoritative tick count.
- KO is evaluated after every snapshotted contact resolves. A single KO awards the opponent; simultaneous KO is an unscored draw. KO takes precedence when the same tick also expires the timer.
- Timeout compares remaining health; equal health is an unscored draw. Drawn rounds increase the displayed round number and replay until one fighter reaches two wins, so the match is first-to-two rather than a strict three-round maximum.
- A 180-tick result hold freezes combat. The next round resets health, zero-start energy, position, velocity, facing, attack/charge/stun/feedback state, projectiles, and the timer while preserving scores.
- Match over freezes combat and exposes the winner in simulation state. Buffered `R` or Enter starts a new match and resets scores and all round state without leaking attack edges.
- The HUD displays authoritative time, round number, win pips, intro countdown, KO/double-KO/time-over and round-winner messages, plus a match-winner/rematch panel.
- `GameRenderer` loads one RGBA neutral texture for each fighter, draws it from character-specific bottom-center artwork pivots, mirrors it without moving that pivot, and falls back to the procedural body if an asset cannot be loaded.
- Fighter shadows and large special fields render behind the body; block, charge, attack, hit/impact, projectile, KO, debug, and HUD presentation remain separate from the sprite body. The same static neutral art is intentionally used for every combat state in this prototype.
- Native builds copy `assets/characters/` beside the executable. Emscripten packages that directory into `arena-clash.data` at `/assets/characters` without embedding image bytes in C++.
- Grounded, stationary fighters in `Available` state use a presentation-only three-second sine cycle: vertical scale varies by at most 0.6% and horizontal scale varies inversely by at most 0.3%. Both the image and its existing pivot are scaled together, so the world-space ground anchor remains fixed.
- VOLT and EMBER use quarter-cycle-separated breathing phases. Movement, jumping, block, attacks, charge, hit-stun, and KO all select the identity transform; no animation counter or transform is stored in authoritative state.

## Verified functionality

- `cmake --build build/tests --parallel` and `ctest --test-dir build/tests --output-on-failure` passed 1 of 1 test executables.
- `cmake --build build/desktop --parallel` completed the GNU/Linux raylib application, and its CTest run passed 1 of 1 test executables.
- `cmake --build build/web --parallel` completed with Emscripten 6.0.12, including the shared match simulation and renderer.
- The generated browser package contains `/assets/characters/volt/neutral.png` and `/assets/characters/ember/neutral.png`; Chrome loaded both textures from it without a fallback warning.
- Automated Chrome 153 screenshots verified both fighters at their expected starting anchors, undistorted alpha compositing, VOLT facing right, mirrored EMBER facing left, matched visual height and foot alignment, stable sprite/ground alignment during a jump, charging and blocking overlays, unobstructed HUD/control text, and unchanged `F1` hurtboxes.
- Twelve Chrome 153/WebAssembly idle samples measured subtle top-edge movement (three pixels for both fighters in the captured viewport) while the detected VOLT and EMBER foot rows remained fixed at y=449 and y=446. Separate captures verified that charging and jumping suppress the breathing transform.
- Headless coverage verifies exact timer duration/gating, timeout outcomes, timer clamping, single and simultaneous KO, KO-at-timeout precedence, score-once behavior, draws, both fighters winning, one win each, first-to-two, complete round reset, projectile clearing, stale-edge rejection, frozen match over, restart, and complete-state replay through match over/rematch.
- An automated Chrome 153/WebAssembly playthrough visually verified Round 1 countdown, the active timer, charged attack effects, KO, VOLT's win pip/result, round reset, Round 2, a 2–0 match victory panel, and `R` rematch to a clean Round 1. WebGL returned error state 0; HTML/JavaScript/Wasm returned HTTP 200/304.
- Phase 1D-C charged effects were exercised during that browser playthrough, completing their previously missing runtime visual check.
- `git diff --check` passed after the final source and documentation edits, and no generated build output is tracked.

## Unavailable verification

- The current native executable could not be runtime-tested because no X11 display is available. The native build and tests pass, and its character assets were copied beside the executable, but native sprite presentation, controls, and shutdown are unverified.
- Windows and macOS builds were not attempted because those platforms/toolchains are unavailable.
- No human-operated playtest was performed. Physical keyboard rollover/ghosting, non-US punctuation layouts, and controller input remain unverified.
- Cross-platform or cross-compiler deterministic equivalence was not tested. Replay consistency is same-build only.
- The Phase 2A Chrome run captured charge and block feedback plus debug boxes, but did not recapture the short attack-active, hit/impact, projectile, or KO effect windows. Their existing draw paths remain separate from and above the new sprite body, and prior milestones exercised them with procedural bodies.

## Known issues and limitations

- Ultimate abilities remain unimplemented, so parent Phase 1D is still in progress despite Phase 1E being complete.
- Authored frame-based or skeletal character animation, pause, audio, AI, networking, replay storage, and general state serialization remain unimplemented.
- Idle breathing deforms the complete neutral image by a very small amount. It does not articulate the chest or limbs, blend poses, or provide state-specific artwork.
- At minimum gameplay-body spacing, the broad neutral illustrations overlap visually because the required 64-unit simulation collision bodies were not changed. State-specific action art should reduce that issue in the animation milestone.
- The projectile pool has eight fixed slots and does not present a warning if exhausted.
- If several simultaneous hits apply different stun/knockback values to one defender, stable contact-array order determines the final stun/velocity while all damage still applies.
- The known missing `favicon.ico` request and headless software-WebGL readback/performance messages do not affect the browser game.

## Next recommended task

Define Phase 2B-B's minimal presentation-owned clip metadata and integrate a consistently authored multi-frame idle clip before extending the same selection seam to walk, jump, block, attack, hit, and KO. Frame selection must remain read-only and must not define combat timing. Phase 1D-D ultimate abilities also remain outstanding gameplay work.
