# Changelog

This file records meaningful verified repository changes. Planned features belong in `docs/ROADMAP.md` or `docs/GAME_DESIGN.md`, not in the completed-change record below.

The format is inspired by Keep a Changelog. The project does not yet have a released version.

## Unreleased

### Added — 2026-10-21 (Phase 2B-B sprite animation infrastructure)

- Added raylib-free clip definitions and fixed-tick sampling for idle, walking, jumping, blocking, light/heavy attacks, hit reaction, charging, fighter-specific special/charged attacks, and KO.
- Added ordered per-frame PNG paths, durations, dimensions, ground pivots, looping/one-shot behavior, presentation playback reset/advance, and startup/active/recovery phase-constrained attack sampling.
- Replaced the two-texture renderer path with renderer-owned per-fighter texture caches and frame bindings; missing, invalid, or dimension-mismatched authored frames use the approved neutral texture and metadata, while neutral failure retains the procedural fallback.
- Kept breathing, facing-safe mirroring, calibrated ground anchors, gameplay state/timing, and combat/effect layering unchanged. No production animation artwork was added.
- Made all character PNGs native build dependencies while retaining directory copying and complete Emscripten character-tree preloading.

### Verified — 2026-10-21 (Phase 2B-B sprite animation infrastructure)

- Passed both the existing core regression executable and new animation executable in isolated and desktop CTest trees (2 of 2 each).
- Verified exact frame-duration boundaries, looping, one-shot completion, playback resets, missing-frame fallback, facing/pivot geometry, all requested state mappings, and live light-attack phase synchronization without changing combat lifecycle.
- Built the GNU/Linux raylib application and Emscripten 6.0.12 output. Chrome 153 loaded HTML/JavaScript/Wasm/data with HTTP 200 and rendered both neutral fallbacks at their preserved anchors; only the known favicon 404 appeared.
- Native runtime rendering remains unverified because GLFW cannot open X11 display `:0`. No authored walking or attack animation is claimed complete.

### Added — 2026-10-21 (Phase 2B-A procedural idle animation)

- Added a renderer-only three-second sine breathing cycle for grounded, stationary fighters in `Available` state, reusing the approved neutral PNGs without changing simulation state.
- Limited the transform to +/−0.6% vertical scale and inverse +/−0.3% horizontal scale, with a quarter-cycle character phase offset.
- Applied the same axis scales to destination dimensions and the mirrored texture origin so each existing world-space ground pivot remains fixed.
- Kept movement, airborne, blocking, attacking, charging, hit-stun, and KO presentation at the exact Phase 2A scale; procedural body fallbacks remain unchanged.

### Verified — 2026-10-21 (Phase 2B-A procedural idle animation)

- Built the GNU/Linux desktop and Emscripten 6.0.12 application targets and retained passing isolated/desktop core regression suites.
- Inspected twelve Chrome 153/WebAssembly idle captures: the detected top edge moved by three pixels for each fighter while VOLT and EMBER foot rows stayed fixed at y=449 and y=446 in every sample.
- Captured charging and jumping states with breathing disabled. Native runtime presentation remains unverified because GLFW cannot open the available X11 display.

### Added — 2026-10-21 (Phase 2A static character sprites)

- Replaced successful VOLT and EMBER body draws with their original RGBA neutral PNG textures while retaining the procedural bodies as per-fighter load-failure fallbacks.
- Added character-specific texture-space pivots and uniform scales, including pivot-corrected horizontal mirroring that keeps the world-space ground anchor fixed.
- Split fighter backdrop effects, body rendering, foreground feedback, projectiles, debug boxes, and HUD into explicit layers so the static art does not suppress combat presentation.
- Added renderer-owned texture lifetime management, executable-relative native asset lookup/copying, and Emscripten preloading at `/assets/characters`.

### Verified — 2026-10-21 (Phase 2A static character sprites)

- Built the GNU/Linux desktop application and Emscripten 6.0.12 browser application; the browser output includes a 2.1 MB `arena-clash.data` package with both neutral PNGs.
- Captured and inspected Chrome 153/WebAssembly frames confirming both assets load with transparency, preserve aspect ratio, align at their ground pivots, face inward, track a jump, remain below the HUD, and retain charge/block feedback and `F1` hurtboxes.
- The native runtime remained unavailable because the environment has no X11 display. The Phase 2A browser pass did not recapture short attack-active, hit/impact, projectile, or KO effect windows; prior milestones verified those effects before the body replacement, and their draw paths remain independent.

### Added — 2026-10-21 (Phase 1E local match flow)

- Added platform-neutral `RoundIntro`, `Fighting`, `RoundOver`, and `MatchOver` state with round result/reason, first-to-two scores, match winner, round number, and fixed-tick phase/timer counters.
- Added a 90-second fighting timer, 3-second intro/result holds, post-contact KO/double-KO/timeout adjudication, KO precedence on the final timer tick, and unscored replayed draws.
- Added centralized round/new-match reset paths for health, zero-start energy, positions, velocities, facing, attacks, stun, charge, feedback, projectiles, timer, scores, and winner state.
- Added a buffered match-level restart command mapped to `R` and Enter; combat-phase gating consumes stale edges and prevents attacks during intro, round-over, and match-over phases.
- Replaced placeholder round UI with authoritative timer/round text, score pips, countdown, KO/double-KO/time-over results, winner text, and a rematch panel.
- Expanded headless coverage for exact timer behavior, every KO/timeout/draw outcome, score-once rules, both round winners, one-win-each progression, first-to-two, complete resets, projectile clearing, stale edges, frozen combat, rematch, and full match-transition replay consistency.

### Verified — 2026-10-21 (Phase 1E completion)

- Built the isolated and desktop Linux targets; both CTest runs passed 1 of 1 test executables with all Phase 1A–1E regressions.
- Built the Emscripten 6.0.12/raylib 6.2 target after fixing a Clang-only aggregate-initialization incompatibility by separating `Game`'s default and explicit rules constructors.
- Ran an automated Chrome 153/WebAssembly match: observed countdown and active timer, used Thunder Strike for a Round 1 KO, verified score/result hold and Round 2 reset, reached a 2–0 `VOLT WINS` match screen, then used `R` to return to a clean Round 1 intro.
- Confirmed WebGL error state 0. HTML, JavaScript, and Wasm loaded successfully; only the known favicon 404 and headless software-rendering/readback diagnostics appeared.
- Attempted the current native runtime, which could not open X11 display `:0`; native runtime presentation and controls therefore remain unverified for this revision.

### Added — 2026-10-21 (Phase 1D-C charged attacks)

- Added three data-backed Thunder Strike and Flame Smash levels selected at exact 18/36/60-tick thresholds with a 72-tick cap, early-release cancellation, and startup beginning on valid release.
- Reused normal combat phases and simultaneous contact snapshotting for charged hit/block/miss/trade/KO behavior while keeping charged activation and contact energy-neutral.
- Added fighter-specific charged poses, electric/fire charge feedback, displayed level/percentage, level-scaled attack streaks, names, and impact rings.
- Added exact boundary, move-data, damage/timing, facing, one-hit, interruption, stale-release, recovery/KO, normal/special interaction, trade, energy, and replay tests.

### Verified — 2026-10-21 (Phase 1D-C completion)

- Built and passed the isolated and desktop Linux CTest suites, built the native application, and linked the Emscripten/WebAssembly application.
- Exercised Thunder Strike charging, release, active effect, impacts, damage, and KO during the Phase 1E Chrome runtime playthrough, closing the prior charged-presentation verification gap.
- Native charged presentation remains unverified in this revision because X11 display `:0` cannot be opened.

### Added — 2026-10-21 (Phase 1D-B character specials)

- Extended move definitions with owner, behavior, energy cost, and projectile tuning while retaining the normal startup/active/recovery pipeline.
- Added VOLT's 25-energy Thunder Bolt: 12 startup, one projectile-spawn active tick, 20 recovery, a 34×22 projectile at 650 units/second for up to 90 ticks, 16 damage, 18 hit-stun, and 420 knockback.
- Added EMBER's 25-energy Inferno Burst: 16 startup, five active ticks, 24 recovery, a facing-aware 110×96 close-range hitbox, 20 damage, 24 hit-stun, and 620 knockback.
- Added a fixed eight-slot value projectile pool with ownership, captured direction, position/velocity, lifetime, and hitbox state; projectiles persist independently of their owner's later state and are removed on contact, expiry, or arena exit.
- Unified fighter and projectile contact snapshotting so valid normal/special/projectile interactions resolve fairly even when another contact interrupts a source on the same tick.
- Added cyan Thunder Bolt casting/projectile/trail/impact effects and orange-red Inferno Burst casting/explosion/impact effects using lightweight raylib primitives.
- Added headless coverage for special activation and every spending lockout, exact move timing, held-input behavior, startup interruption/no refund, projectile lifecycle/ownership/direction/owner independence/blocking, burst hit/miss/block/facing/one-hit behavior, trades, KO, simultaneous specials, and full projectile replay state.

### Verified — 2026-10-21 (Phase 1D-B completion)

- Built the isolated and desktop Linux targets; both CTest runs passed 1 of 1 test executables with all Phase 1A–1D-B regressions.
- Built the Emscripten 6.0.12/raylib 6.2 WebAssembly target and exercised both special controls in Chrome 153.
- Built meter through real browser combat, then captured Thunder Bolt's named cyan projectile/trail with energy reduced from 36 to 11.
- Captured Inferno Burst's named expanding fire effect, 25-energy deduction, 20-damage impact feedback, and KO after semicolon input.
- Confirmed WebGL error state 0 with no JavaScript or Wasm exception. The existing favicon 404 and headless screenshot/readback warnings remained the only page diagnostics.
- Launched the native X11/OpenGL 4.6 build and confirmed renderer/first-frame initialization; the automated run timed out after five seconds without native input or normal-shutdown verification.

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
