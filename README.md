# ARENA CLASH

> An original 2D one-on-one fighting game built for responsive local play, portable native builds, and a future rollback-ready online mode.

**Project status:** Phase 1A complete. The platform-neutral core tests pass, and both the native Linux and Emscripten builds render successfully in verified Linux environments. Phase 1B movement and input work has not started.

## Current implementation

- C++17 core state with a fixed 60 Hz simulation tick
- Accumulator-based desktop loop with a 250 ms frame-delta clamp
- Verified Emscripten callback-based browser loop using requestAnimationFrame scheduling
- 1280×720 logical render target with aspect-preserving window scaling
- Primitive-rendered atmospheric arena, distinct Volt and Ember placeholders, health HUD, round label, simulation/FPS diagnostics, and platform label
- Headless core-state smoke test
- CMake FetchContent dependency on the official raylib 6.2 tag archive, verified by SHA-256

Movement, jumping, combat, input commands, audio, AI, and networking are not implemented.

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

The first configure downloads raylib 6.2. This path is verified on Ubuntu 22.04 with CMake 3.22.1 and GNU 11.4.0; the application opened, rendered the complete Phase 1A scene, and closed cleanly.

## Browser build

Install and activate the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) so that `emcc` and `emcmake` are on `PATH`, then run:

```sh
./scripts/run-web.sh
```

The script configures `build/web`, builds `arena-clash.html` plus its JavaScript/Wasm output, and serves it on `127.0.0.1`. By default the OS selects an available port and the script prints the exact URL. Set `PORT` to request a specific port:

```sh
PORT=8000 ./scripts/run-web.sh
```

This path is verified with Emscripten 6.0.12 and automated Google Chrome 153 testing on Linux. The arena, Volt, Ember, health bars, and HUD rendered without a JavaScript or WebAssembly runtime exception.

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

The entry point owns platform lifecycle and fixed-step scheduling. `Game` owns platform-neutral authoritative state and advances only on fixed ticks. `GameRenderer` observes that state and owns all raylib drawing/resources. This keeps the simulation free of raylib and leaves room for future per-frame commands, replay, state restoration, and rollback work without implementing those systems prematurely.

See [Architecture](docs/ARCHITECTURE.md), [Game Design](docs/GAME_DESIGN.md), [Development Roadmap](docs/ROADMAP.md), and [Project Status](docs/STATUS.md) for the detailed boundaries and verified state.

## Roadmap

| Phase | Goal | Status |
| --- | --- | --- |
| 0 | Repository foundation and persistent documentation | Complete |
| 1A | Native and browser window setup | Complete — Linux desktop and browser verified |
| 1B | Two fighters and movement | Next |
| 1C | Basic combat and health | Planned |
| 1D | Special and charged attacks | Planned |
| 1E | Complete local matches | Planned |
| 2 | Animations and visual polish | Planned |
| 3 | Additional gameplay and AI | Planned |
| 4 | Online multiplayer | Planned |

## Contributing

Before changing the project, read [AGENTS.md](AGENTS.md) for coding, verification, documentation, and Git workflow expectations.
