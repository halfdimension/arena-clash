# Changelog

This file records meaningful verified repository changes. Planned features belong in `docs/ROADMAP.md` or `docs/GAME_DESIGN.md`, not in the completed-change record below.

The format is inspired by Keep a Changelog. The project does not yet have a released version.

## Unreleased

### Verified — 2026-10-19 (Phase 1A completion)

- Configured and compiled the native Linux Release target with CMake 3.22.1, GNU 11.4.0, and raylib 6.2 after the required X11 development headers became available.
- Launched and visually inspected the live 1280×720 X11/OpenGL window, confirming the arena, fighters, health bars, HUD, diagnostics, and Linux platform label render correctly.
- Closed the native window through its normal window-manager close path and confirmed exit code 0 plus framebuffer, texture, shader, and window cleanup logs.
- Passed the core-state test in both the native application build tree and the isolated headless test build tree.
- Completed the Phase 1A native and browser acceptance checks; Windows and macOS remain untested targets.

### Fixed — 2026-10-19 (Phase 1A browser rendering)

- Removed `WindowShouldClose()` from the Emscripten callback because raylib 6.2 implements it with `emscripten_sleep`, which aborted the non-Asyncify build before its first draw call.
- Kept target-FPS sleeping native-only so requestAnimationFrame remains the sole browser scheduler.
- Added focused window, render-texture, loop-registration, and first-frame diagnostics.
- Updated the web helper to bind `127.0.0.1`, let the OS atomically select an available port by default, print the live URL, support an explicit `PORT`, and stop cleanly.

### Verified — 2026-10-19 (Phase 1A browser rendering)

- Built the application with Emscripten 6.0.12 and raylib 6.2.
- Reproduced the pre-fix abort and black frame in headless Google Chrome 153.
- Captured and inspected the corrected browser frame showing the arena, Volt, Ember, health bars, round HUD, diagnostics, and Web platform label.
- Confirmed WebGL/render-texture initialization and first-frame execution without a JavaScript or WebAssembly runtime exception.
- Verified automatic and explicit-port web serving plus HTTP 200 responses for the HTML, JavaScript, and Wasm outputs.

### Added — 2026-10-19 (Phase 1A implementation)

- Added a C++17 CMake project with an isolated platform-neutral core and raylib application target.
- Pinned FetchContent to the official raylib 6.2 release archive and its SHA-256 without vendoring dependency sources.
- Added a fixed 60 Hz accumulator loop with a bounded catch-up delta, native lifecycle, and Emscripten callback lifecycle.
- Added a 1280×720 logical renderer with an atmospheric primitive arena, Volt and Ember placeholders, HUD, debug information, and platform identification.
- Added a browser shell, separate desktop/web run scripts, and a headless core-state smoke test.

### Verified — 2026-10-19 (Phase 1A partial verification)

- Confirmed the official `6.2` tag commit and verified the release archive hash.
- Passed strict C++17 syntax checks for the project against raylib 6.2 headers.
- Configured and compiled the platform-neutral core and passed 1 of 1 CTest tests with GNU 11.4.0 and CMake 3.22.1 on Linux.
- Confirmed the web helper exits with actionable guidance when Emscripten is not installed.
- During the initial partial check, native configuration was blocked by missing RandR development headers; the later Phase 1A completion check above supersedes that limitation.
- During the initial partial check, Emscripten was not available on `PATH`; the later browser verification entry above records the completed build and runtime check.

### Added — 2026-10-19

- Established the Phase 0 repository foundation.
- Added project overview, contribution guidance, proposed architecture, initial game design, milestone roadmap, current-status tracking, and the first architecture decision record.
- Added the empty source, asset, test, and script layout for future implementation.
- Added ignore rules for native, CMake, editor, cache, and WebAssembly build artifacts.

### Verified — 2026-10-19

- Inspected the repository structure and documentation for Phase 0 consistency.
- Confirmed `git diff --check` reports no whitespace errors.

No application, build, or gameplay feature is included in this entry.
