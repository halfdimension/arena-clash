# Agent Guide

This file contains persistent instructions for coding agents working on ARENA CLASH. It applies to the entire repository unless a more specific `AGENTS.md` is added below a subdirectory.

## Project goals and constraints

- Build an original 2D one-on-one fighting game in C++17 with raylib 6.2 and CMake.
- Target Linux, Windows, macOS, and browsers through Emscripten. Do not describe a target as supported until its build has been tested.
- Run gameplay simulation at a fixed 60 Hz.
- Keep input, simulation, rendering, and audio separate.
- Convert device input into per-frame commands before gameplay consumes it.
- Keep combat rules independent of raylib and presentation code.
- Prefer composition and data-driven fighter definitions over deep inheritance.
- Preserve a path toward deterministic-enough replays and rollback networking, but never claim cross-platform determinism without automated evidence.
- Favor clear, modular C++17 over premature frameworks or abstractions.

## Sources of truth

- `docs/STATUS.md`: current verified implementation state and immediate next work
- `docs/ROADMAP.md`: milestones, deliverables, and acceptance criteria
- `docs/ARCHITECTURE.md`: proposed and implemented technical boundaries
- `docs/GAME_DESIGN.md`: gameplay rules and provisional balance values
- `docs/decisions/`: durable architecture decisions and their rationale
- `docs/CHANGELOG.md`: chronological record of meaningful verified changes

If documents disagree, reconcile them as part of the change. `docs/STATUS.md` is authoritative about what exists now; the roadmap and design documents describe intent.

## C++ conventions

- Use C++17 and portable standard-library facilities unless an approved dependency provides a clearer solution.
- Use four spaces for indentation; never use tabs.
- Prefer braces on the same line as declarations and control statements.
- Use `PascalCase` for types, `camelCase` for functions and local variables, and `kPascalCase` for constants.
- Use `.hpp` for headers and `.cpp` for implementation files.
- Prefer value semantics, RAII, scoped enums, `const`, and explicit ownership.
- Avoid global mutable state, owning raw pointers, hidden allocations in frame-critical code, and gameplay logic in rendering functions.
- Keep headers self-contained and dependencies narrow. Use forward declarations where they genuinely reduce coupling.
- Express simulation time in frames or explicit fixed-step units. Do not let render delta time directly drive combat rules.
- Keep fighter tuning in data/configuration structures rather than scattering numeric literals through logic.
- Add short comments for intent and invariants, not line-by-line narration.

## Change discipline

- Inspect existing code and documentation before editing.
- Make the smallest coherent change that meets the current milestone.
- Preserve established module boundaries. If a boundary must change, explain why and update the architecture record in the same change.
- Avoid unrelated formatting, renaming, cleanup, or dependency additions.
- Do not add empty abstractions or speculative systems solely for future features.
- Keep changes reviewable; separate mechanical refactors from behavioral changes when practical.
- Preserve user-authored work and call out overlapping or unexplained modifications before proceeding.

## Testing and verification

- Add or update tests for simulation and combat behavior when such code exists.
- Run the narrowest relevant checks first, then the broader available suite.
- For build-system changes, configure and build every locally available affected target.
- For rendering or input changes, record the manual checks performed and their platform.
- Run `git diff --check` before handoff.
- Never say a build, test, platform, or feature works unless it was actually exercised. Report skipped checks and their reason.
- Do not claim deterministic behavior from identical code alone; verify it with repeatable state/hash tests on the relevant targets.

## Documentation workflow

After every meaningful, verified change:

1. Update `docs/STATUS.md` with verified progress, open work, known issues, the verification performed, and the next recommended task.
2. Add a dated entry to `docs/CHANGELOG.md` describing the verified change.
3. Update `docs/ARCHITECTURE.md` when module boundaries, data flow, build design, or networking strategy changes.
4. Update `docs/GAME_DESIGN.md` when combat rules, controls, roster, or balance targets change.
5. Update `docs/ROADMAP.md` when milestone scope, ordering, or acceptance criteria change.
6. Update `README.md` after major public-facing milestones and only add build instructions that have been verified.

Keep planned behavior clearly separated from implemented behavior everywhere.

## Git workflow

- Review `git status` and the relevant diff before and after editing.
- Do not discard, overwrite, or rewrite changes you did not create.
- Use focused commits with imperative subjects if the user authorizes commits.
- Never commit, amend, rebase, tag, push, publish, create a remote, or open a pull request without explicit user approval.
- Do not commit generated build output or local configuration.

## Required handoff

End each task with a concise summary containing:

- files and behavior changed;
- what was actually verified, including commands or manual checks;
- checks not run and why;
- known issues or assumptions;
- the exact recommended next step.
