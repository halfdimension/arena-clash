# ADR 0001: Technology and architectural baseline

- **Status:** Accepted for initial development
- **Date:** 2026-10-19

## Context

ARENA CLASH needs a small, understandable foundation for a responsive 2D fighting game. The project targets desktop platforms and the browser, while leaving room for replay tools and eventual rollback multiplayer. Combat behavior must be testable without a graphics context, and early development should avoid a large engine or unnecessary framework layers.

## Decision

Use the following baseline:

- **C++17** for the game and shared platform-neutral systems.
- **raylib 6.2** for windowing, device access, 2D rendering, and audio presentation.
- **CMake** as the build-system generator.
- **Emscripten** for the WebAssembly/browser target.
- **Native desktop and browser application targets** that share the same gameplay simulation.
- **Separated input, simulation, rendering, and audio**, with fixed 60 Hz gameplay and per-frame input commands.

Simulation and combat code must not depend on raylib types or calls. Platform input is translated into commands before it reaches the simulation. Rendering and audio observe state and presentation events without determining gameplay outcomes.

## Rationale

- C++17 provides modern language features while remaining broadly supported by desktop and Emscripten toolchains.
- raylib offers a deliberately small C API and supports the required graphics/audio/input scope without imposing a game-object architecture.
- CMake can describe native and Emscripten builds from one project and integrates with common C++ tools.
- Emscripten is the established path for compiling C++ applications to WebAssembly and integrating a browser main loop.
- Shared simulation code limits platform-specific behavior and makes headless tests practical.
- Per-frame commands and fixed ticks are useful foundations for input recording, replay, and rollback resimulation.

## Consequences

### Positive

- Combat can be unit-tested without opening a window.
- Native and browser targets can share rules and data.
- Presentation systems can evolve without rewriting combat logic.
- Recorded commands can later drive replay and networking experiments.

### Costs and risks

- The project must maintain explicit boundaries rather than relying on an engine to enforce them.
- Browser lifecycle, asset packaging, and audio restrictions need target-specific handling.
- Fixed ticks do not guarantee deterministic outcomes. Floating-point behavior, compiler/platform differences, serialization, and update ordering require dedicated tests.
- A raylib acquisition strategy still needs to be selected and verified in Phase 1A; no dependency is installed by this decision.

## Alternatives considered

- **A full game engine:** faster access to editor tooling, but more framework complexity and less direct control over the learning-oriented architecture.
- **A newer C++ standard:** useful features, but C++17 currently offers a conservative compatibility baseline for the intended toolchains.
- **Variable-step gameplay:** simpler for a visual prototype, but harder to test, replay, and reconcile for fighting-game combat.
- **Direct device polling inside fighters:** quick initially, but tightly couples controls to simulation and prevents clean replay, AI, and rollback input sources.

## Validation required

Phase 1A must verify the chosen CMake/raylib strategy with one native build and an Emscripten browser build. Later phases must test replayable state sequences and must not claim cross-platform determinism until comparable state-hash tests pass on the claimed platforms.
