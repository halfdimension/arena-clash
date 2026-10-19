# ARENA CLASH

> An original 2D one-on-one fighting game built for responsive local play, portable native builds, and a future rollback-ready online mode.

**Project status:** Phase 1B complete. Volt and Ember can move and jump simultaneously through fixed-tick commands, and the behavior is verified in native Linux and Chrome/WebAssembly builds. Combat begins in Phase 1C and is not implemented yet.

## Current implementation

- C++17 fighter state and movement simulation with a fixed 60 Hz tick
- Accumulator-based desktop loop with a 250 ms frame-delta clamp
- Verified Emscripten callback-based browser loop using requestAnimationFrame scheduling
- Per-player held and just-pressed commands produced by a raylib keyboard adapter
- Walking, air control, jumping, gravity, landing, stage bounds, body collision, and automatic facing
- 1280×720 logical render target with aspect-preserving window scaling
- Primitive-rendered atmospheric arena, state-driven Volt and Ember placeholders, control overlay, health HUD, round label, simulation/FPS diagnostics, and platform label
- Headless movement and state-sequence tests
- CMake FetchContent dependency on the official raylib 6.2 tag archive, verified by SHA-256

Combat, damage, mutable health, attacks, audio, AI, and networking are not implemented.

## Controls

| Fighter | Move | Jump |
| --- | --- | --- |
| VOLT (Player 1) | `A` / `D` | `W` |
| EMBER (Player 2) | Left / Right arrows | Up arrow |

Both players can act at the same time. Opposing left/right inputs for one fighter cancel, and holding jump does not trigger another jump after landing.

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

The first configure downloads raylib 6.2. This path is verified on Ubuntu 22.04 with CMake 3.22.1 and GNU 11.4.0; the application accepted simultaneous two-player movement/jump input, rendered the resulting state, landed both fighters, and closed cleanly.

## Browser build

Install and activate the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) so that `emcc` and `emcmake` are on `PATH`, then run:

```sh
./scripts/run-web.sh
```

The script configures `build/web`, builds `arena-clash.html` plus its JavaScript/Wasm output, and serves it on `127.0.0.1`. By default the OS selects an available port and the script prints the exact URL. Set `PORT` to request a specific port:

```sh
PORT=8000 ./scripts/run-web.sh
```

This path is verified with Emscripten 6.0.12 and automated Google Chrome 153 testing on Linux. The arena, moving/jumping Volt and Ember, controls, health bars, and HUD rendered without a JavaScript or WebAssembly runtime exception.

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

The entry point owns platform lifecycle and fixed-step scheduling. `KeyboardInput` converts device state into per-player commands, `Game` consumes those values and owns platform-neutral authoritative movement state, and `GameRenderer` observes that state and owns all raylib drawing/resources. This keeps the simulation free of raylib and lets future local, replay, AI, or network sources supply the same command type.

See [Architecture](docs/ARCHITECTURE.md), [Game Design](docs/GAME_DESIGN.md), [Development Roadmap](docs/ROADMAP.md), and [Project Status](docs/STATUS.md) for the detailed boundaries and verified state.

## Roadmap

| Phase | Goal | Status |
| --- | --- | --- |
| 0 | Repository foundation and persistent documentation | Complete |
| 1A | Native and browser window setup | Complete — Linux desktop and browser verified |
| 1B | Two fighters and movement | Complete — Linux desktop and browser verified |
| 1C | Basic combat and health | Next |
| 1D | Special and charged attacks | Planned |
| 1E | Complete local matches | Planned |
| 2 | Animations and visual polish | Planned |
| 3 | Additional gameplay and AI | Planned |
| 4 | Online multiplayer | Planned |

## Contributing

Before changing the project, read [AGENTS.md](AGENTS.md) for coding, verification, documentation, and Git workflow expectations.
