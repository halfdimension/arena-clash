# Development Roadmap

The roadmap describes intended scope. A milestone is complete only when its acceptance criteria have been exercised and the results are recorded in `docs/STATUS.md` and `docs/CHANGELOG.md`.

## Phase 0 — Repository foundation

**Objectives**

- Establish a clear repository structure and persistent guidance.
- Separate proposed features from verified implementation status.

**Deliverables**

- Project README, agent instructions, architecture, game design, roadmap, status, changelog, and initial decision record
- Source, asset, test, and script directory skeleton
- Project-appropriate `.gitignore`

**Acceptance criteria**

- All listed documents exist and agree that gameplay and builds are not implemented.
- Proposed architecture, initial design, milestone sequence, and agent workflow are documented.
- Required empty directories are retained with marker files.
- Repository structure is inspected and `git diff --check` passes.

## Phase 1A — Native and browser window setup

**Status:** Complete. The application, renderer, fixed-step lifecycle, build paths, scripts, and core smoke test are implemented. Core tests pass; the native Linux executable and browser output both compile, render the Phase 1A scene, and shut down or remain scheduled through their documented lifecycle paths. Verification details are recorded in `docs/STATUS.md` and `docs/CHANGELOG.md`.

**Objectives**

- Establish the smallest maintainable application and build configuration.
- Prove a shared lifecycle can open a window on an available native platform and in a browser.

**Deliverables**

- Top-level CMake project using C++17
- Explicit, reproducible raylib 6.2 dependency strategy
- Minimal entry point and application lifecycle with a clear-screen window
- Native and Emscripten build paths, plus verified build instructions
- Basic automated smoke or lifecycle coverage where practical

**Acceptance criteria**

- Configure and compile succeed for at least one documented native environment.
- The native executable is manually verified to open, render, and close cleanly.
- Emscripten configure and compile succeed, and the browser output is manually verified to load and render.
- No gameplay logic is coupled to the platform entry point.
- README, status, changelog, and architecture reflect only the targets actually tested.

## Phase 1B — Two fighters and movement

**Status:** Complete. Data-backed fighter state, per-player commands, fixed-tick movement, jumping, gravity, facing, stage/body collision, state-driven rendering, and headless coverage are implemented. Replaying the tested command stream produces the same state at every tick. Automated key-driven runtime checks passed on native Linux and Chrome/WebAssembly; details are recorded in `docs/STATUS.md` and `docs/CHANGELOG.md`.

**Objectives**

- Introduce Volt and Ember as data-backed fighters.
- Implement frame-command input and fixed-step movement.

**Deliverables**

- Fixed 60 Hz simulation loop with a documented catch-up policy
- Per-player command representation and provisional keyboard mappings
- Fighter definitions and mutable fighter state
- Walking, jumping, gravity, facing, stage bounds, and placeholder rendering
- Headless movement/state-transition tests

**Acceptance criteria**

- Two players can independently move and jump on one keyboard.
- Movement produces the same verified state sequence when replaying the same command sequence in the tested environment.
- Simulation does not query raylib input or use render delta time for combat state.
- Fighters remain within stage bounds and return to grounded state consistently.

## Phase 1C — Basic combat and health

**Status:** Complete. Data-backed light/heavy attacks, explicit combat phases, grounded directional guard, rectangle hit/hurt collision, one-hit activation tracking, damage/stun/knockback, health/KO, combat presentation, and regression coverage are implemented. Native Linux and Chrome/WebAssembly combat runtime checks passed; details are recorded in `docs/STATUS.md` and `docs/CHANGELOG.md`.

**Objectives**

- Make light/heavy combat functional and readable.
- Establish tested hit resolution and round health.

**Deliverables**

- Fighter combat states and frame-based move data
- Hitboxes, hurtboxes, collision queries, and one-hit-per-activation tracking
- Light/heavy attacks, grounded blocking, damage, hit-stun, and knockback
- Live health HUD, combat feedback, KO indication, and optional debug boxes
- Headless combat tests for hit, miss, block, trade, and repeated-hit rules

**Acceptance criteria**

- Both fighters can hit, block, take damage, enter stun, recover, and display current health.
- Collision and damage outcomes follow a documented tick order.
- A hitbox cannot unintentionally damage the same target on consecutive active frames.
- Automated tests cover representative hit, block, miss, simultaneous-hit, and boundary cases.

Phase 1C deliberately uses no chip damage, block-stun, or combo tracking. Those mechanics require explicit future design and are not completion criteria for this basic combat milestone.

## Phase 1D — Special and charged attacks

**Objectives**

- Differentiate the two fighters with signature moves.
- Add energy spending and risk/reward charging.

**Deliverables**

- Special-energy gain, spending, caps, and HUD
- Volt and Ember special attacks and projectiles
- Three-level hold/release charged heavy attacks
- Ultimate abilities with placeholder presentation
- Tests for charge thresholds, interruption, energy rules, and projectiles

**Acceptance criteria**

- Each fighter has at least one distinct special, a projectile, a charged heavy, and an ultimate.
- Moves cannot begin without sufficient energy, and energy changes exactly once per qualifying event.
- Charge levels transition on the documented simulation frames and cap correctly.
- Projectile lifetime, collision, ownership, and repeated-hit behavior are tested.

## Phase 1E — Complete local matches

**Objectives**

- Turn the combat sandbox into a complete local match flow.
- Stabilize the first playable ruleset.

**Deliverables**

- Round intro, timer, knockout, timeout, draw, round reset, and match-over flow
- Best-of-three scoring, rematch, pause, and exit behavior
- One complete arena and readable placeholder UI/audio feedback
- Resolved rules for repeated draws and deciding rounds
- End-to-end local match checks

**Acceptance criteria**

- Two players can start, finish, and rematch a complete best-of-three match without restarting the application.
- KO, double KO, timeout, draw, and match victory outcomes follow documented rules.
- State resets do not leak health, position, transient hit state, or unintended energy between rounds.
- A documented manual test matrix passes on every platform claimed in the README.

## Phase 2 — Animations and visual polish

**Objectives**

- Replace programmer presentation with cohesive character and arena visuals.
- Improve audiovisual feedback without changing simulation truth.

**Deliverables**

- Character animation pipeline and authored animation sets
- Arena art, camera behavior, effects, HUD polish, audio, and accessibility review
- Asset loading/attribution workflow and performance profiling

**Acceptance criteria**

- Visual animation follows simulation state without controlling hit timing.
- Every combat state has readable player feedback.
- Assets have documented origin/license information.
- Frame pacing meets an explicit tested target on claimed platforms.

## Phase 3 — Additional gameplay and AI

**Objectives**

- Deepen the local game after the core match is stable.
- Add a useful offline opponent.

**Deliverables**

- Selected mechanics such as additional moves, cancels, throws, or defensive options
- Training/debug tools and expanded replay fixtures
- AI opponent with configurable difficulty
- Balance revisions backed by playtest notes

**Acceptance criteria**

- Added mechanics have documented rules, frame data, and automated simulation coverage.
- AI can complete matches using the same command interface as a human player.
- Difficulty does not depend on reading hidden future player input.
- The complete local match regression suite continues to pass.

## Phase 4 — Online multiplayer

**Objectives**

- Add responsive remote matches using measured, debuggable rollback behavior.
- Preserve offline simulation behavior.

**Deliverables**

- Versioned input/state serialization and deterministic replay test harness
- State snapshot, prediction, rollback, resimulation, and desync diagnostics
- Session, transport, matchmaking/invite, disconnect, and latency behavior
- Platform compatibility and soak-test plan

**Acceptance criteria**

- Recorded command streams reproduce verified state hashes on each supported build configuration; any cross-platform limitations are documented.
- Automated network simulations cover delay, jitter, loss, prediction mismatch, rollback, and disconnects.
- Online matches remain playable within documented network bounds without corrupting authoritative state.
- Desyncs are detectable and produce actionable diagnostics.
