# ARENA CLASH

> An original 2D one-on-one fighting game built for responsive local play, portable native builds, and a future rollback-ready online mode.

**Project status:** Phase 1E complete. ARENA CLASH now runs first-to-two local matches with fixed-tick round flow, scoring, reset, victory, and rematch. Phase 1D-C charged attacks are also complete; parent Phase 1D still lacks ultimate abilities. Linux builds/tests and the Chrome/WebAssembly match flow are verified, while the current native runtime could not access X11 display `:0`.

## Current implementation

- C++17 fighter state and movement simulation with a fixed 60 Hz tick
- Accumulator-based desktop loop with a 250 ms frame-delta clamp
- Verified Emscripten callback-based browser loop using requestAnimationFrame scheduling
- Per-player held/pressed/released commands with a raylib-free render-to-simulation edge buffer
- Walking, air control, jumping, gravity, landing, stage bounds, body collision, and automatic facing
- Data-backed 5/3/10 light and 12/4/17 heavy attack timing, mirrored rectangle hitboxes, one-hit activation tracking, and fair simultaneous trades
- Grounded directional blocking, 100-HP mutable health, hit-stun, knockback, stage-constrained KO, and combat lockouts
- 0–100 energy with once-per-activation normal hit/block awards and atomic 25-energy special spending
- Grounded fixed-tick charging with 18/36/60 level thresholds, a 72-tick cap, early-release cancellation, hit interruption, and action lockouts
- Three-level VOLT Thunder Strike and EMBER Flame Smash charged attacks with distinct timing, range, stun, knockback, and procedural effects
- VOLT Thunder Bolt with fixed-tick projectile movement, ownership, blocking, lifetime, and one-contact behavior
- EMBER Inferno Burst with a facing-aware short-range hitbox, stronger damage, stun, and knockback
- Fixed-tick `RoundIntro`, `Fighting`, `RoundOver`, and `MatchOver` flow with a 90-second timer
- Post-contact KO, double-KO, and timeout adjudication with KO precedence on the final timer tick
- First-to-two scoring, replayed unscored draws, complete round resets, match victory, and `R`/Enter rematch
- 1280×720 logical render target with aspect-preserving window scaling
- Primitive-rendered arena and fighters with combat effects, health/energy, timer, round score, countdown/results, match winner/rematch UI, control help, and optional `F1` debug boxes
- Headless movement/combat/energy/charge/projectile/match timing, outcome, reset, edge-buffer, lockout, interaction, and same-build replay tests
- CMake FetchContent dependency on the official raylib 6.2 tag archive, verified by SHA-256

Chip/block-stun, combos, ultimates, pause, audio, AI, controllers, replay storage, and networking are not implemented.

## Controls

| Fighter | Move | Jump | Block | Light | Heavy | Special | Charge |
| --- | --- | --- | --- | --- | --- | --- | --- |
| VOLT (Player 1) | `A` / `D` | `W` | `S` | `F` | `G` | `H` | Hold/release `J` |
| EMBER (Player 2) | Left / Right arrows | Up arrow | Down arrow | `K` | `L` | Semicolon (`;`) | Hold/release apostrophe (`'`) |

After either fighter reaches two round wins, press `R` or Enter to begin a new match.

Both players can act at the same time. Opposing directions cancel, and buffered press/release edges trigger once rather than repeating on catch-up ticks. Block is grounded, must face the contact, and prevents current normal/special/charged damage. Charge is also grounded and locks other actions: release before 18 simulation ticks cancels, while 18–35, 36–59, and 60–72 release Thunder Strike or Flame Smash at levels 1, 2, and 3. A special costs 25 energy: VOLT fires Thunder Bolt and EMBER performs Inferno Burst. Charged attacks cost no energy. Press `F1` to show hurtboxes and active hitboxes, including projectiles.

## Technology

- C++17
- CMake 3.22 or newer
- raylib 6.2, fetched per build directory
- Emscripten 6.0.12 for the verified WebAssembly build

Linux is the verified native build/test environment. Earlier milestones ran on X11 and Intel/Mesa OpenGL 4.6, but the current Phase 1E executable could not open display `:0`, so this revision's native runtime is not claimed as verified. The current browser build and complete match/rematch flow are verified on Linux in automated Chrome/WebGL testing. Windows and macOS remain unverified targets.

## Ubuntu dependencies

Install the compiler/build tools and raylib's desktop development dependencies using your system package manager. On Ubuntu 22.04, the expected packages are:

```sh
sudo apt update
sudo apt install build-essential cmake git \
    libasound2-dev libx11-dev libxrandr-dev libxi-dev \
    libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev
```

The project scripts never install packages or modify global environment configuration.

## Native desktop build

Configure and build in the dedicated native directory:

```sh
cmake -S . -B build/desktop -DCMAKE_BUILD_TYPE=Release
cmake --build build/desktop --parallel
./build/desktop/ArenaClash
```

Or use the helper that performs those steps and launches the game:

```sh
./scripts/run-desktop.sh
```

The first configure downloads raylib 6.2. This build path is verified on Ubuntu 22.04 with CMake 3.22.1 and GNU 11.4.0. The current executable compiles, but its runtime could not open the environment's X11 display `:0`; current native controls, visuals, and shutdown remain unverified.

## Browser build

Install and activate the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) so that `emcc` and `emcmake` are on `PATH`, then run:

```sh
./scripts/run-web.sh
```

The script configures `build/web`, builds `arena-clash.html` plus its JavaScript/Wasm output, and serves it on `127.0.0.1`. By default the OS selects an available port and the script prints the exact URL. Set `PORT` to request a specific port:

```sh
PORT=8000 ./scripts/run-web.sh
```

This path is verified with Emscripten 6.0.12 and automated Google Chrome 153 testing on Linux. The current runtime completed a two-round charged-attack match, displayed countdown/timer, KO/result/score, reset into Round 2, displayed a 2–0 match victory, and accepted `R` for a clean rematch. WebGL reported error state 0; the only HTTP error was the harmless missing favicon request.

## Core tests

Core simulation tests do not require raylib or a display:

```sh
cmake -S . -B build/tests \
    -DARENA_CLASH_BUILD_APPLICATION=OFF \
    -DCMAKE_BUILD_TYPE=Release
cmake --build build/tests --parallel
ctest --test-dir build/tests --output-on-failure
```

## Architecture

The entry point owns platform lifecycle and fixed-step scheduling. `KeyboardInput` samples device state, `FrameCommandBuffer` bridges render frames to fixed ticks, and `Game` owns platform-neutral authoritative match, movement, combat, energy, charge, and projectile state. `src/combat/` owns move/rule data and rectangle queries; `GameRenderer` observes state and owns raylib presentation. Future replay, AI, or network sources can supply the same command type.

See [Architecture](docs/ARCHITECTURE.md), [Game Design](docs/GAME_DESIGN.md), [Development Roadmap](docs/ROADMAP.md), and [Project Status](docs/STATUS.md) for the detailed boundaries and verified state.

## Roadmap

| Phase | Goal | Status |
| --- | --- | --- |
| 0 | Repository foundation and persistent documentation | Complete |
| 1A | Native and browser window setup | Complete — Linux desktop and browser verified |
| 1B | Two fighters and movement | Complete — Linux desktop and browser verified |
| 1C | Basic combat and health | Complete — Linux desktop and browser verified |
| 1D-A | Energy and special-move foundation | Complete — Linux build and browser runtime verified |
| 1D-B | Character-specific special powers | Complete — Linux build and browser runtime verified |
| 1D-C | Charged attack resolution | Complete — Linux tests/builds and browser runtime verified |
| 1D-D | Ultimate abilities | Planned — remaining parent Phase 1D scope |
| 1E | Complete local matches | Complete — Linux tests/builds and browser runtime verified |
| 2 | Animations and visual polish | Planned |
| 3 | Additional gameplay and AI | Planned |
| 4 | Online multiplayer | Planned |

## Contributing

Before changing the project, read [AGENTS.md](AGENTS.md) for coding, verification, documentation, and Git workflow expectations.
