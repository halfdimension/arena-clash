# ARENA CLASH

> A planned original 2D one-on-one fighting game built for responsive local play, portable native builds, and a future rollback-ready online mode.

**Project status:** Phase 0 — repository foundation. The game, build system, and playable features have not been implemented yet.

## Vision

ARENA CLASH aims to combine the immediacy of an arcade fighter with a compact, understandable C++ codebase. Its first playable target is a complete local two-player match between two distinct fighters. Later phases will add animation, richer gameplay, AI, and online multiplayer.

## Planned features

- Fixed 60 Hz combat simulation
- Local two-player movement, jumping, attacks, and blocking
- Light, heavy, special, charged, projectile, and ultimate attacks
- Explicit hitboxes, hurtboxes, hit-stun, knockback, combos, health, and energy
- Data-driven fighter definitions and composition-oriented gameplay code
- Native desktop and browser builds
- Replay-friendly per-frame input commands, with rollback networking as a future goal

These are roadmap goals, not currently implemented features.

## Fighters

| Fighter | Identity | Planned play style |
| --- | --- | --- |
| **VOLT** | Lightning fighter | Fast movement, quick pressure, and mobility-focused specials |
| **EMBER** | Fire fighter | Higher-impact attacks, deliberate pressure, and fire projectiles |

See [Game Design](docs/GAME_DESIGN.md) for the provisional move sets and balance targets.

## Technology

- C++17
- raylib 6.2
- CMake
- Emscripten for WebAssembly builds

The intended platforms are Linux, Windows, macOS, and modern web browsers. Platform support remains unverified until builds are implemented and exercised on each target.

## Architecture

Input, fixed-step simulation, rendering, and audio will remain separated. Gameplay will consume per-frame commands instead of querying devices directly, while the renderer will observe game state without owning combat rules. This boundary is intended to support testing, replays, and eventual rollback networking.

The detailed design, including what is proposed versus implemented, is in [Architecture](docs/ARCHITECTURE.md). The key decision record is [ADR 0001: Technology and architectural baseline](docs/decisions/0001-tech-stack.md).

## Roadmap

| Phase | Goal | Status |
| --- | --- | --- |
| 0 | Repository foundation and persistent documentation | Complete (documentation verified) |
| 1A | Native and browser window setup | Next |
| 1B | Two fighters and movement | Planned |
| 1C | Basic combat and health | Planned |
| 1D | Special and charged attacks | Planned |
| 1E | Complete local matches | Planned |
| 2 | Animations and visual polish | Planned |
| 3 | Additional gameplay and AI | Planned |
| 4 | Online multiplayer | Planned |

Objectives and acceptance criteria are tracked in the [Development Roadmap](docs/ROADMAP.md). The verified current state is maintained in [Project Status](docs/STATUS.md), and completed changes are recorded in the [Changelog](docs/CHANGELOG.md).

## Build

There are no build instructions yet because no build configuration or application code has been implemented or verified. Phase 1A will establish and test the first CMake, raylib, native, and browser build paths before instructions are published here.

## Screenshots and demo

_Screenshots will be added after the first visually representative milestone._

_A playable demo link will be added only after a browser build is deployed and verified._

## Documentation

- [Architecture](docs/ARCHITECTURE.md)
- [Game Design](docs/GAME_DESIGN.md)
- [Roadmap](docs/ROADMAP.md)
- [Current Status](docs/STATUS.md)
- [Changelog](docs/CHANGELOG.md)
- [Architecture Decision Records](docs/decisions/)

## Contributing

The project is at an early foundation stage. Before changing it, read [AGENTS.md](AGENTS.md) for coding, verification, documentation, and Git workflow expectations.
