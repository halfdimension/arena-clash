# Project Status

**Current phase:** Phase 1D-A — Energy system and special-move foundation (complete)

**Phase state:** Complete and verified; ready to begin Phase 1D-B

**Last updated:** 2026-10-21

This file is the source of truth for the current verified state. Roadmap and game-design documents may describe work that does not exist yet.

## Implemented components

- Top-level CMake project using C++17 and separate application/core targets, with raylib 6.2 fetched from its SHA-256-pinned official tag archive.
- Fixed 60 Hz simulation with an accumulator and 250 ms render-delta clamp, allowing at most 15 catch-up ticks per rendered frame.
- Native lifecycle using a conventional window loop and Emscripten lifecycle using a registered requestAnimationFrame callback.
- Platform-neutral held/pressed/released commands and a raylib-free render-frame buffer. Held values reach every catch-up tick; one-shot edges survive render frames with no tick and reach only the next tick.
- Existing movement, jumping, blocking, light/heavy attacks, health, stun, knockback, KO, one-hit tracking, and simultaneous trade behavior from Phases 1B–1C.
- Per-fighter energy initialized to 0 and clamped to 100. Light hits award 8, heavy hits award 12, and blocked attacks award 4 to the attacker plus 6 to the defender, once per activation.
- Central `CombatRules` tuning for energy gains/cap, the reserved 25-energy special cost, and 18/72-tick charge thresholds. Atomic affordability/spending helpers are ready for a real Phase 1D-B special activation; Phase 1D-A special input does not spend energy by itself.
- `Charging` combat state with a fixed-tick counter, a 72-tick cap, cancellation below 18 ticks, readiness at 18 or more ticks, interruption by an unblocked hit, and one-tick cancelled/ready/interrupted outcomes for later move activation or presentation.
- Normal attack and charge mutual exclusion: the established block/light/heavy priority is preserved when buttons conflict; charging locks movement, jump, normal attacks, block entry, and future special activation until release or interruption.
- Keyboard mappings extended with VOLT `H` special / `J` charge and EMBER semicolon special / apostrophe charge. raylib 6.2's Web backend maps DOM `Semicolon` and `Quote` codes and registers both key-down and key-up callbacks on the canvas.
- Read-only rendering of energy bars, numeric energy, charge percentage, charging pose/aura, and updated controls while preserving the arena and existing combat presentation.
- Headless regression coverage for all Phase 1A–1C behavior plus energy gain/clamping/spending, repeated rewards, blocked awards, simultaneous awards, command-edge buffering, exact charge thresholds/cap/interruption/lockouts, and same-build replay consistency including the new state.

## Exact energy, charge, and tick semantics

- The tick accepting a charge press counts as charge tick 1. A held charge increments once per fixed simulation tick and remains capped at 72.
- Release at 0–17 accumulated ticks cancels; release at 18–72 reports `Ready`. Phase 1D-A deliberately produces no charged hitbox or damage.
- Command validation occurs before movement and hit snapshotting. A release processed at the start of a tick completes/cancels charging before an impact later that tick; a fighter still charging when a valid unblocked hit resolves is interrupted.
- A hit or block marks the attack activation connected before outcomes apply. Energy is awarded from those snapshotted outcomes, so later active frames cannot reward again and simultaneous trades award both fighters without player-order bias.
- Special affordability requires the fighter to be `Available` with at least 25 energy. The spending helper deducts exactly 25 atomically and refuses without changing energy otherwise. No actual special currently invokes it.

## Verified functionality

- Configured and built the isolated native Linux Release core with CMake 3.22.1 and GNU 11.4.0. `ctest --test-dir build/tests --output-on-failure` passed 1 of 1 test executables.
- Configured and built the native Linux application with raylib 6.2. `ctest --test-dir build/desktop --output-on-failure` passed 1 of 1 test executables.
- Launched the native Linux/X11 application with Intel/Mesa OpenGL 4.6 and confirmed successful window, render-target, and first-frame initialization. The automated check ended it after five seconds rather than exercising normal shutdown or controls.
- Relinked `arena-clash.html`, `arena-clash.js`, and `arena-clash.wasm` with Emscripten 6.0.12 and raylib 6.2. The Web cross-build does not register a runnable CTest test.
- Served the generated browser files over loopback and exercised them in Chrome 153 through DevTools input. Captured frames confirmed the energy HUD, VOLT `J` charging, EMBER apostrophe/DOM `Quote` charging, charge percentage/cap feedback, simultaneous held movement, and a light hit producing 8 energy and 94 defender health.
- Chrome reported WebGL error state 0 and no JavaScript or WebAssembly runtime exception. The only network error was the unchanged harmless `/favicon.ico` HTTP 404; headless Chrome also reported screenshot/readback rendering warnings.
- Same-build complete-state replay checks passed for two simulations receiving the same 300-tick movement/combat/special/charge command stream. This is not a cross-platform determinism claim.

## Unavailable verification

- Windows and macOS builds were not attempted because those toolchains/platforms are unavailable.
- No human-operated playtest was performed. Native input/charge/energy behavior was not exercised; interactive checks were performed in Chrome with automated DevTools key events and visually inspected captures.
- Physical keyboard rollover/ghosting and non-US punctuation-key layouts were not exhaustively tested. Browser source inspection confirms physical DOM-code mappings for semicolon/quote, while actual Chrome input verified apostrophe/Quote.
- Cross-platform or cross-compiler deterministic equivalence was not tested.

## Known issues and limitations

- Phase 1D-A adds only the special/charge foundation. `H` and semicolon commands are transported and buffered, but do not activate or spend meter until Phase 1D-B adds real character-specific moves.
- A ready charge is a one-tick simulation outcome only; it has no damage, hitbox, move phases, or post-release pose yet.
- There are no lightning/fire attacks, projectiles, ultimates, AI, networking, audio, controller input, replay storage, or round-management changes.
- KO freezes combat actions but does not end/reset a round; the round label and 99-second timer remain presentation placeholders for Phase 1E.
- Guard still has no chip damage or block-stun. Movement bodies, hurtboxes, and normal hitboxes remain simple rectangles.

## Next recommended task

Begin Phase 1D-B by defining data-backed VOLT and EMBER special move phases and spending 25 energy only when a valid move actually starts. Then connect `ChargeReleaseOutcome::Ready` to an explicitly designed charged attack, add projectile ownership/lifetime/collision rules, and extend simultaneous-resolution tests before adding presentation effects.
