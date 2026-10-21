# Changelog

This file records meaningful verified repository changes. Planned features belong in `docs/ROADMAP.md` or `docs/GAME_DESIGN.md`, not in the completed-change record below.

The format is inspired by Keep a Changelog. The project does not yet have a released version.

## Unreleased

### Added — 2026-10-21 (Phase 1D-A energy and charge foundation)

- Added centralized 0–100 energy rules: light hits award 8, heavy hits award 12, and blocked attacks award 4 to the attacker plus 6 to the defender, with once-per-activation awards and clamping.
- Added 25-energy future-special affordability/atomic spending helpers without adding or activating character specials.
- Added platform-neutral special and charged-attack held/pressed/released commands plus a raylib-free buffer that preserves edges until one fixed tick and prevents repeats during catch-up ticks.
- Mapped VOLT special/charge to `H`/`J` and EMBER special/charge to semicolon/apostrophe.
- Added a grounded `Charging` state with exact 18-tick minimum, 72-tick cap, early-release cancellation, ready release, valid-hit interruption, action lockouts, and one-tick release outcomes. No charged damage or hitbox was added.
- Added energy bars, numeric energy, charging pose/aura, percentage feedback, and updated control help through read-only rendering.
- Expanded headless regression coverage for energy outcomes/clamping/spending, buffered input edges, charge thresholds/cap/interruption/lockouts, and full-state same-build replay consistency while retaining all Phase 1A–1C cases.

### Verified — 2026-10-21 (Phase 1D-A completion)

- Built the isolated and desktop Linux Release targets; both CTest runs passed 1 of 1 test executables.
- Launched the native X11/OpenGL 4.6 application and confirmed successful window, renderer, and first-frame initialization; this automated check timed out after five seconds and did not exercise native controls or normal shutdown.
- Linked the Emscripten 6.0.12/raylib 6.2 browser output and exercised it in Chrome 153 with automated DevTools input.
- Visually inspected browser captures showing both fighters' energy HUD, VOLT `J` charge, EMBER apostrophe/DOM `Quote` charge, live charge percentages/cap, and a successful light hit producing 8 energy and 94 defender health.
- Confirmed WebGL error state 0 and no JavaScript or WebAssembly exception. The only network error remained the harmless missing favicon; headless screenshot/readback warnings were also recorded.
- Inspected raylib 6.2's Emscripten backend to confirm DOM `Semicolon` and `Quote` mappings plus key-down/key-up canvas callbacks. Physical keyboard rollover and non-US punctuation layouts remain unverified.

### Added — 2026-10-21 (Phase 1C combat)

- Added raylib-independent light/heavy move definitions, rectangle hitbox/hurtbox queries, mirrored attack orientation, and facing-side guard validation.
- Added 100-HP mutable health; available, attacking/startup-active-recovery, blocking, hit-stun, and KO state; one-hit-per-activation tracking; knockback; wall constraints; and simultaneous candidate-hit collection before mutation.
- Extended per-player commands and keyboard mapping with buffered light/heavy press edges and held block/attack state for VOLT (`S`/`F`/`G`) and EMBER (Down/`K`/`L`).
- Added live/numeric health rendering, distinct light/heavy poses, guard and hit feedback, KO labels, updated control help, and an `F1` hurtbox/active-hitbox overlay without moving combat rules into rendering.
- Expanded the headless suite with exact move timing, unblocked damage, directional guard, recovery/stun/KO lockouts, repeated-hit prevention, horizontal/vertical misses, orientation, trades, wall knockback, health clamping, held-attack behavior, and complete-state same-build replay checks while retaining Phase 1B regressions.

### Fixed — 2026-10-21 (Phase 1C final audit)

- Marked all snapshotted simultaneous attackers as connected before resolving either queued outcome, removing player-order-dependent transient `attackHasConnected` state after a trade.

### Verified — 2026-10-21 (Phase 1C completion)

- Built the native Linux Release application with CMake 3.22.1, GNU 11.4.0, and raylib 6.2; both isolated headless and desktop CTest runs passed 1 of 1 test executables.
- Launched the X11/OpenGL 4.6 native build, used automated two-player input to produce a simultaneous light trade, visually inspected a captured 94/94 health frame with hit flashes, and closed cleanly through Escape.
- Linked the browser target with Emscripten 6.0.12 and raylib 6.2, served the generated HTML/JavaScript/Wasm, and exercised it in Chrome 153.
- Visually inspected captured browser frames for a 94/94 light trade, a successful 100/100 held block with guard feedback, a heavy hit reducing EMBER to 86, and repeated heavy hits reaching 0 HP with KO feedback.
- Confirmed the browser page loaded, WebGL reported error state 0, and no JavaScript or WebAssembly runtime exception occurred. The only page error was the known harmless missing favicon request.
- Verified complete fighter-state repeatability for two simulations receiving the same 300-tick movement/combat command stream in the current Linux build; this is not a cross-platform determinism claim.

### Added — 2026-10-19 (Phase 1B movement)

- Added composed VOLT and EMBER state with platform-neutral position, velocity, movement tuning, grounded/facing state, collision dimensions, identity, and placeholder health.
- Added per-player held/pressed command values and a raylib keyboard adapter for `A`/`D`/`W` and the arrow keys; pending jump edges are consumed by one fixed simulation tick.
- Added fixed-tick walking, air control, jumping, gravity, landing, stage constraints, body collision, and automatic facing without introducing raylib into the simulation library.
- Connected fighter rendering and orientation to authoritative state and added a compact controls overlay.
- Expanded the headless test executable to cover movement, conflicting directions, jumping, gravity, landing, midair and held-jump rules, boundaries, simultaneous input, collision, facing, and same-command state-sequence repeatability.

### Verified — 2026-10-19 (Phase 1B completion)

- Built the native Linux Release application with CMake 3.22.1, GNU 11.4.0, and raylib 6.2; the expanded CTest suite passed 1 of 1 test executables.
- Exercised both players simultaneously in the X11 build with automated movement and jump input, visually inspected captured movement/airborne/landed frames, and confirmed clean exit through the window-manager close path.
- Built the browser application with Emscripten 6.0.12 and raylib 6.2, then exercised simultaneous movement and held jump controls in Google Chrome 153.
- Captured and inspected browser frames showing movement, jumping, landing, facing, arena, fighters, controls, and HUD; Chrome reported WebGL error state 0 and no JavaScript or WebAssembly runtime exception.
- Confirmed the same command sequence produces exactly matching fighter state on every tested fixed tick in two independent simulations. This is same-build repeatability, not a cross-platform determinism claim.

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
