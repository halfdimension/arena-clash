# Project Status

**Current phase:** Phase 1A — Native and browser window setup (complete)

**Phase state:** Complete; ready to begin Phase 1B

**Last updated:** 2026-10-19

This file is the source of truth for the current verified state. Roadmap and game-design documents may describe work that does not exist yet.

## Implemented components

- Top-level CMake project using C++17 and separate application/core targets.
- CMake FetchContent acquisition of the official raylib 6.2 tag archive, pinned by SHA-256; dependency source remains inside each ignored build directory.
- Platform-neutral `Game` state with Volt and Ember placeholder records and a monotonically increasing simulation-frame counter.
- Fixed-step application loop at 60 Hz with render/simulation separation, an accumulator, and a 250 ms frame-delta clamp (at most 15 catch-up ticks per rendered frame).
- Native lifecycle using a conventional window loop and Emscripten lifecycle using a registered browser callback rather than a blocking loop.
- Browser scheduling avoids raylib's synchronous `WindowShouldClose` and target-FPS wait paths; requestAnimationFrame is the sole web frame scheduler.
- `GameRenderer` with a 1280×720 logical render target, aspect-preserving scaling, primitive arena art, distinct placeholder fighters, player names, health bars, round display, FPS/tick data, and platform identification.
- Custom browser shell and native/browser run scripts using isolated `build/desktop` and `build/web` directories. The web server binds to `127.0.0.1`, atomically requests an OS-assigned port by default, and accepts an optional `PORT` override.
- Headless core-state smoke test that validates initialization and fixed-tick frame advancement.

Movement, jumping, combat, input handling, audio, AI, and networking are not implemented.

## Verified functionality

- Confirmed the official raylib `6.2` Git tag resolves to commit `02566453880e06bec97f5b8160fa6fe1e98397ca`.
- Downloaded the official tag archive and verified the pinned SHA-256 value used by CMake.
- Strict C++17 syntax checks passed for all project translation units against raylib 6.2 headers using `-Wall -Wextra -Wpedantic`.
- Configured and built the platform-neutral core and its test with CMake/GNU 11.4.0.
- `ctest --test-dir build/tests --output-on-failure` passed 1 of 1 tests.
- Configured and built the native Linux Release target both directly and through `scripts/run-desktop.sh`, using CMake 3.22.1, GNU 11.4.0, and raylib 6.2 with its X11/OpenGL 3.3 build path.
- Launched the native executable in the Linux X11 session. raylib initialized the Intel/Mesa OpenGL 4.6 context and the 1280×720 render texture, and the first-frame diagnostic completed.
- Captured and visually inspected the live native window: the arena, Volt, Ember, health bars, round display, diagnostics, and `Linux desktop` platform label rendered correctly.
- Closed the native window through `WM_DELETE_WINDOW`; the process exited with code 0 after unloading its framebuffer, texture, shader, and window resources.
- `ctest --test-dir build/desktop --output-on-failure` passed 1 of 1 tests against the native build tree.
- Configured and built `arena-clash.html`, `arena-clash.js`, and `arena-clash.wasm` with Emscripten 6.0.12 and raylib 6.2.
- Reproduced the original black frame in Google Chrome 153 and captured the runtime abort caused by calling raylib's `WindowShouldClose()` without Asyncify.
- Verified the corrected build in headless Google Chrome 153 on Linux: WebGL initialized, the 1280×720 render texture was valid, the first-frame diagnostic fired, and a captured screenshot showed the arena, Volt, Ember, health bars, round display, debug information, and platform label.
- Browser console inspection found no JavaScript, WebAssembly, or functional WebGL errors after the fix. Chrome's software WebGL backend reported only screenshot-readback performance warnings.
- Verified the web helper with both an OS-assigned port and `PORT=39123`; HTML, JavaScript, and Wasm responses returned HTTP 200 with appropriate MIME types, and Ctrl+C stopped the server cleanly.

## Unavailable verification

- Windows and macOS builds were not attempted because those toolchains/platforms are unavailable.
- An interactive manual browser session was not performed; rendering was inspected through an automated headless Chrome screenshot and captured console output.

## Known issues and limitations

- The current health bars and round data are static presentation placeholders; no match or combat rules exist.
- There is not yet a device-to-command input layer or serializable state snapshot implementation; the current boundaries leave space for both.
- Cross-platform determinism has not been tested and is not claimed.

## Next recommended task

Begin Phase 1B with a compact per-frame input command, data-backed mutable fighter state, fixed-tick walking/jumping/gravity/stage bounds, and replayable headless movement tests. Do not expand into Phase 1C combat while establishing those movement boundaries.
