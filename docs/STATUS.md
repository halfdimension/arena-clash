# Project Status

**Current phase:** Phase 1C — Basic combat and health (complete)

**Phase state:** Complete; ready to begin Phase 1D

**Last updated:** 2026-10-21

This file is the source of truth for the current verified state. Roadmap and game-design documents may describe work that does not exist yet.

## Implemented components

- Top-level CMake project using C++17 and separate application/core targets, with raylib 6.2 fetched from its SHA-256-pinned official tag archive.
- Fixed 60 Hz simulation with an accumulator and 250 ms render-delta clamp, allowing at most 15 catch-up ticks per rendered frame.
- Native lifecycle using a conventional window loop and Emscripten lifecycle using a registered requestAnimationFrame callback without `WindowShouldClose()`, target-FPS sleeping, or Asyncify in the browser callback.
- Platform-neutral per-player commands for held movement, jump, block, and attack buttons plus buffered one-shot jump/light/heavy presses. VOLT uses `A`/`D`/`W`/`S`/`F`/`G`; EMBER uses Left/Right/Up/Down/`K`/`L`.
- Composed fighter state for identity, movement tuning, position/velocity, grounding, facing, body collision, 100 HP, combat state, current move/phase, stun, one-hit tracking, and short hit/block feedback.
- Phase 1B walking, jumping, gravity, stage/body collision, airborne cross-over, and automatic facing behavior preserved.
- Raylib-independent `src/combat/` move definitions and axis-aligned hitbox/hurtbox queries. Light is 6 damage with 5/3/10 startup/active/recovery frames; heavy is 14 damage with 12/4/17 frames.
- Explicit available, attacking, blocking, hit-stun, and KO states. Grounded attacks and blocks stop horizontal movement; airborne attacks retain momentum; grounded hit knockback is damped and constrained by stage/body collision.
- Grounded held blocking with no chip damage or block-stun. A guard is valid only when the attacker is on the side the defender faces, and block takes priority over an attack press on the same tick.
- Simultaneous-hit candidate collection before mutation, so valid same-tick attacks trade without player-order bias. One activation can connect only once, including against a block.
- Mutable health clamped to zero, hit-stun and facing-relative knockback, visible hit/block effects, light/heavy poses, numeric health HUD, KO labels, and `F1` debug hurtbox/active-hitbox rendering.
- Headless regression coverage for Phase 1B movement plus move data/timing, damage, guard behavior and direction, recovery/stun lockouts, one-hit activation, horizontal/vertical misses, orientation, trades, wall knockback, held attacks, health clamping, KO lockout, and same-build fixed-tick replay consistency.

## Exact frame and tick semantics

- The tick accepting an attack press is startup frame 1.
- A phase remains current for exactly its configured number of processed ticks. The next phase begins at the start of the following tick; a fighter becomes available at the start of the tick after the final recovery frame.
- Each fixed tick performs: completed state/phase transitions and command validation; movement/gravity integration; stage/body collision; facing update; active-hit candidate snapshot for both fighters; one-hit marking; attack/stun counter advancement; block/damage/stun/knockback/KO application from the snapshot; simulation-frame increment.
- A newly applied hit-stun duration begins after the impact tick and restricts the following configured number of ticks. Both candidate hits are captured before either outcome mutates health or state.

## Verified functionality

- Configured and built the native Linux Release target with CMake 3.22.1, GNU 11.4.0, and raylib 6.2.
- `ctest --test-dir build/tests --output-on-failure` and `ctest --test-dir build/desktop --output-on-failure` each passed 1 of 1 test executables. The executable covers all movement and combat cases listed above.
- Launched the native Linux/X11 application with Intel/Mesa OpenGL 4.6. Automated X11 input moved both fighters together and pressed both light attacks; a captured frame showed both health values at 94 with hit flashes. Escape closed the window normally with framebuffer, texture, shader, and window cleanup logs.
- Relinked `arena-clash.html`, `arena-clash.js`, and `arena-clash.wasm` with Emscripten 6.0.12 and raylib 6.2.
- Served the generated browser files over loopback and exercised the final build through Chrome 153 DevTools input. Captured frames confirmed the arena/HUD, simultaneous light trade at 94/94, held Down-arrow block preventing VOLT's light damage at 100/100 with guard feedback, VOLT heavy reducing EMBER to 86, and repeated heavies producing 0 HP plus KO indicators.
- Chrome reported WebGL error state `0` and no JavaScript or WebAssembly runtime exception. Its only page error was the unchanged harmless `/favicon.ico` HTTP 404.
- `git diff --check` is required again at final handoff after documentation edits.

## Unavailable verification

- Windows and macOS builds were not attempted because those toolchains/platforms are unavailable.
- No human-operated playtest was performed; native and browser combat controls were exercised with automated key input and captured frames were visually inspected.
- Cross-platform or cross-compiler deterministic equivalence was not tested. Repeatability is verified only for two simulations replaying the same command stream in the current Linux build.

## Known issues and limitations

- KO freezes combat actions but does not end/reset a round; the round label and 99-second timer remain presentation placeholders for Phase 1E.
- Phase 1C intentionally has no chip damage, block-stun, combos/cancels, energy, specials, charged attacks, ultimates, projectiles, audio, controller input, or AI.
- Guard uses a dedicated held button and requires the attacker to be on the defender's facing side. There is no crouch or high/low guard distinction.
- Movement bodies, hurtboxes, and move hitboxes are deliberately simple rectangles. Hit and block flashes are simulation counters used as short presentation cues; no general event/audio queue exists yet.
- Keyboard ghosting varies by hardware and was not exhaustively tested despite simultaneous software input verification.
- Replay storage, state serialization, rollback, interpolation, and cross-platform determinism are not implemented or claimed.

## Next recommended task

Begin Phase 1D by defining tested energy and charge rules before implementing VOLT/EMBER specials, projectiles, charged heavy levels, or ultimates. Preserve the Phase 1C command/tick boundary and add each new move through data-backed combat definitions rather than presentation code.
