# ARENA CLASH

> An original 2D one-on-one fighting game built for responsive local play, portable native builds, and a future rollback-ready online mode.

**Project status:** Phase 1D-A complete. VOLT and EMBER retain the Phase 1C combat sandbox and now gain/display energy plus enter a fixed-tick charge state through buffered commands. Native Linux and WebAssembly builds pass; Chrome runtime checks exercised energy and charging. Actual special/charged attacks remain Phase 1D-B work.

## Current implementation

- C++17 fighter state and movement simulation with a fixed 60 Hz tick
- Accumulator-based desktop loop with a 250 ms frame-delta clamp
- Verified Emscripten callback-based browser loop using requestAnimationFrame scheduling
- Per-player held/pressed/released commands with a raylib-free render-to-simulation edge buffer
- Walking, air control, jumping, gravity, landing, stage bounds, body collision, and automatic facing
- Data-backed 5/3/10 light and 12/4/17 heavy attack timing, mirrored rectangle hitboxes, one-hit activation tracking, and fair simultaneous trades
- Grounded directional blocking, 100-HP mutable health, hit-stun, knockback, stage-constrained KO, and combat lockouts
- 0–100 energy with once-per-activation hit/block awards and a reserved 25-energy future-special cost
- Grounded fixed-tick charging with an 18-tick minimum, 72-tick cap, early-release cancellation, hit interruption, and action lockouts
- 1280×720 logical render target with aspect-preserving window scaling
- Primitive-rendered arena and fighters with attack/guard/charge poses, hit/block feedback, health/energy HUD, charge percentage, KO labels, control help, and optional `F1` debug boxes
- Headless movement/combat/energy/charge timing, collision, state-transition, boundary, edge-buffer, lockout, and same-build replay tests
- CMake FetchContent dependency on the official raylib 6.2 tag archive, verified by SHA-256

Round flow, chip/block-stun, combos, actual special/charged/ultimate attacks, projectiles, audio, AI, controllers, replay storage, and networking are not implemented.

## Controls

| Fighter | Move | Jump | Block | Light | Heavy | Special | Charge |
| --- | --- | --- | --- | --- | --- | --- | --- |
| VOLT (Player 1) | `A` / `D` | `W` | `S` | `F` | `G` | `H` | Hold/release `J` |
| EMBER (Player 2) | Left / Right arrows | Up arrow | Down arrow | `K` | `L` | Semicolon (`;`) | Hold/release apostrophe (`'`) |

Both players can act at the same time. Opposing directions cancel, and buffered press/release edges trigger once rather than repeating on catch-up ticks. Block is grounded, must face the attacker, and currently prevents all normal-attack damage. Charge is also grounded; it locks other actions, cancels if released before 18 simulation ticks, caps at 72, and currently produces no attack. Special inputs are buffered but intentionally do nothing until Phase 1D-B adds real moves. Press `F1` to show green hurtboxes and red active hitboxes.

## Technology

- C++17
- CMake 3.22 or newer
- raylib 6.2, fetched per build directory
- Emscripten 6.0.12 for the verified WebAssembly build

Linux is the verified native development environment, using X11 and Intel/Mesa OpenGL 4.6 for the recorded runtime check. The browser build is verified on Linux in automated Chrome/WebGL testing. Windows and macOS remain unverified targets.

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

The first configure downloads raylib 6.2. This path is verified on Ubuntu 22.04 with CMake 3.22.1 and GNU 11.4.0. The Phase 1D-A build launched on X11/OpenGL 4.6 and rendered its first frame; the current automated check ended after five seconds, so native Phase 1D-A controls and normal shutdown were not re-exercised.

## Browser build

Install and activate the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) so that `emcc` and `emcmake` are on `PATH`, then run:

```sh
./scripts/run-web.sh
```

The script configures `build/web`, builds `arena-clash.html` plus its JavaScript/Wasm output, and serves it on `127.0.0.1`. By default the OS selects an available port and the script prints the exact URL. Set `PORT` to request a specific port:

```sh
PORT=8000 ./scripts/run-web.sh
```

This path is verified with Emscripten 6.0.12 and automated Google Chrome 153 testing on Linux. Phase 1D-A captures covered the energy HUD, both charge controls (`J` and apostrophe/DOM `Quote`), live charge percentages/cap, simultaneous held movement, and a light hit producing 8 energy and 94 defender health. No JavaScript or WebAssembly runtime exception occurred; WebGL reported error state 0.

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

The entry point owns platform lifecycle and fixed-step scheduling. `KeyboardInput` samples device state, `FrameCommandBuffer` bridges render frames to fixed ticks, and `Game` owns platform-neutral authoritative movement/combat/energy/charge state. `src/combat/` owns move/rule data and rectangle queries; `GameRenderer` observes state and owns raylib presentation. Future local, replay, AI, or network sources can supply the same command type.

See [Architecture](docs/ARCHITECTURE.md), [Game Design](docs/GAME_DESIGN.md), [Development Roadmap](docs/ROADMAP.md), and [Project Status](docs/STATUS.md) for the detailed boundaries and verified state.

## Roadmap

| Phase | Goal | Status |
| --- | --- | --- |
| 0 | Repository foundation and persistent documentation | Complete |
| 1A | Native and browser window setup | Complete — Linux desktop and browser verified |
| 1B | Two fighters and movement | Complete — Linux desktop and browser verified |
| 1C | Basic combat and health | Complete — Linux desktop and browser verified |
| 1D-A | Energy and special-move foundation | Complete — Linux build and browser runtime verified |
| 1D-B | Character specials and charged attack resolution | Next |
| 1E | Complete local matches | Planned |
| 2 | Animations and visual polish | Planned |
| 3 | Additional gameplay and AI | Planned |
| 4 | Online multiplayer | Planned |

## Contributing

Before changing the project, read [AGENTS.md](AGENTS.md) for coding, verification, documentation, and Git workflow expectations.
