# Project Status

**Current phase:** Phase 1B — Two fighters and movement (complete)

**Phase state:** Complete; ready to begin Phase 1C

**Last updated:** 2026-10-19

This file is the source of truth for the current verified state. Roadmap and game-design documents may describe work that does not exist yet.

## Implemented components

- Top-level CMake project using C++17 and separate application/core targets, with raylib 6.2 fetched from its SHA-256-pinned official tag archive.
- Fixed 60 Hz simulation with an accumulator and 250 ms render-delta clamp, allowing at most 15 catch-up ticks per rendered frame.
- Native lifecycle using a conventional window loop and Emscripten lifecycle using a registered requestAnimationFrame callback without `WindowShouldClose()`, target-FPS sleeping, or Asyncify in the browser callback.
- Platform-neutral `FighterCommand` and `FrameCommands` values with distinct held and just-pressed states. Commands contain no raylib types and can be supplied directly by tests or future replay/network sources.
- `KeyboardInput` mappings for VOLT (`A`, `D`, `W`) and EMBER (Left, Right, Up). Held directions remain available to every simulation tick; jump presses are buffered until a tick and consumed once.
- Composed `Fighter` state containing identity, position, velocity, movement speed, jump velocity, grounded state, facing, collision dimensions, and placeholder health.
- Fixed-tick walking, 75% horizontal air control, 2,200 units/s² gravity, per-fighter jump tuning, landing, no midair jump, stage bounds, and opposing-direction cancellation.
- Axis-aligned movement body collision. Overlap is separated horizontally and symmetrically in prior-frame left/right order, with wall-aware residual correction and inward velocity cancellation. Vertically clear fighters can cross over.
- Automatic facing based on final post-collision positions each tick.
- `GameRenderer` driven by fighter position/facing state, preserving the 1280×720 arena and HUD while adding an in-game controls overlay.
- Headless tests for initialization, horizontal movement, conflicting input, jump initiation, gravity, midair-jump rejection, held-jump behavior, landing, stage bounds, simultaneous movement, body collision, cross-over facing, fixed tick counts, and repeated command-state consistency.
- Native/browser run scripts using isolated `build/desktop` and `build/web` directories; the browser helper uses an OS-assigned loopback port by default and accepts `PORT` as an override.

Combat, attacks, damage, mutable health, blocking, audio, AI, and networking are not implemented.

## Verified functionality

- Configured and built the native Linux Release target with CMake 3.22.1, GNU 11.4.0, and raylib 6.2 through `scripts/run-desktop.sh`.
- `ctest --test-dir build/desktop --output-on-failure` passed 1 of 1 test executables. The executable exercises the Phase 1B movement and repeatability cases listed above.
- Launched the native executable in Linux X11 with Intel/Mesa OpenGL 4.6. Automated X11 key input and captured frames confirmed simultaneous player movement, simultaneous jumping, landing without a held-key re-jump, the controls overlay, and state-driven rendering.
- Closed the native window through `WM_DELETE_WINDOW`; it exited with code 0 after unloading framebuffer, texture, shader, and window resources.
- Configured and built `arena-clash.html`, `arena-clash.js`, and `arena-clash.wasm` with Emscripten 6.0.12 and raylib 6.2.
- Served the browser build through `scripts/run-web.sh`; HTML, JavaScript, and Wasm requests returned HTTP 200 from the printed OS-assigned loopback URL.
- Used Chrome 153 DevTools input automation to hold VOLT right and EMBER left simultaneously, then press and hold both jump controls. Captured frames showed both fighters move, jump, and land without auto-jumping again while the keys remained held.
- Chrome initialized WebGL 1, the 1280×720 logical render texture, and the requestAnimationFrame loop. The inspected WebGL error state was `0`, and no JavaScript or WebAssembly runtime exception occurred.
- Chrome requested an absent `/favicon.ico`, producing a harmless HTTP 404; all application resources loaded successfully.

## Unavailable verification

- Windows and macOS builds were not attempted because those toolchains/platforms are unavailable.
- No human-operated playtest was performed; native and browser controls were exercised with automated key input and the captured frames were visually inspected.
- Cross-platform or cross-compiler deterministic equivalence was not tested. Repeatability is verified only for two simulations replaying the same command sequence in the current Linux build.

## Known issues and limitations

- Health bars, round number, and timer remain static presentation placeholders; there are no combat or match rules.
- Movement body boxes are deliberately simple and are not future attack hitboxes or hurtboxes.
- Keyboard ghosting varies by hardware and was not exhaustively tested despite simultaneous software input verification.
- Controller input, rebinding, replay storage, state serialization, and interpolation are not implemented.
- Cross-platform determinism is not claimed.

## Next recommended task

Begin Phase 1C by defining tested combat-state/tick ordering and data-backed light/heavy moves, then add hitboxes, hurtboxes, one-hit-per-activation tracking, blocking, damage, stun, knockback, and mutable health through the existing command/simulation boundary. Do not begin special, charged, or ultimate attacks until the Phase 1C acceptance tests pass.
