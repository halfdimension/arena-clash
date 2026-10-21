# Project Status

**Current phase:** Phase 1E — Complete local matches (complete)

**Phase state:** Phase 1D-C and Phase 1E are complete and verified in automated Linux builds/tests and a Chrome/WebAssembly runtime; parent Phase 1D still lacks ultimates

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

## Verified functionality

- `cmake --build build/tests --parallel` and `ctest --test-dir build/tests --output-on-failure` passed 1 of 1 test executables.
- `cmake --build build/desktop --parallel` completed the GNU/Linux raylib application, and its CTest run passed 1 of 1 test executables.
- `cmake --build build/web --parallel` completed with Emscripten 6.0.12, including the shared match simulation and renderer.
- Headless coverage verifies exact timer duration/gating, timeout outcomes, timer clamping, single and simultaneous KO, KO-at-timeout precedence, score-once behavior, draws, both fighters winning, one win each, first-to-two, complete round reset, projectile clearing, stale-edge rejection, frozen match over, restart, and complete-state replay through match over/rematch.
- An automated Chrome 153/WebAssembly playthrough visually verified Round 1 countdown, the active timer, charged attack effects, KO, VOLT's win pip/result, round reset, Round 2, a 2–0 match victory panel, and `R` rematch to a clean Round 1. WebGL returned error state 0; HTML/JavaScript/Wasm returned HTTP 200/304.
- Phase 1D-C charged effects were exercised during that browser playthrough, completing their previously missing runtime visual check.
- `git diff --check` passed after the final source and documentation edits, and no generated build output is tracked.

## Unavailable verification

- The current native executable could not be runtime-tested because GLFW could not open X11 display `:0`; it exited before window creation. The native build and tests pass, but current native visuals, controls, and shutdown are unverified.
- Windows and macOS builds were not attempted because those platforms/toolchains are unavailable.
- No human-operated playtest was performed. Physical keyboard rollover/ghosting, non-US punctuation layouts, and controller input remain unverified.
- Cross-platform or cross-compiler deterministic equivalence was not tested. Replay consistency is same-build only.

## Known issues and limitations

- Ultimate abilities remain unimplemented, so parent Phase 1D is still in progress despite Phase 1E being complete.
- Pause, audio, AI, networking, replay storage, general state serialization, and advanced sprites remain unimplemented.
- The projectile pool has eight fixed slots and does not present a warning if exhausted.
- If several simultaneous hits apply different stun/knockback values to one defender, stable contact-array order determines the final stun/velocity while all damage still applies.
- The known missing `favicon.ico` request and headless software-WebGL readback/performance messages do not affect the browser game.

## Next recommended task

Specify and implement Phase 1D-D ultimate abilities through the existing move/contact pipeline, including per-fighter costs, frame data, collision rules, interruption, presentation, and tests, to close the remaining parent Phase 1D scope before Phase 2 polish.
