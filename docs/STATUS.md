# Project Status

**Current phase:** Phase 1D-B — Character-specific special powers (complete)

**Phase state:** Complete and verified; parent Phase 1D remains in progress

**Last updated:** 2026-10-21

This file is the source of truth for the current verified state. Roadmap and game-design documents may describe work that does not exist yet.

## Implemented components

- All verified Phase 1A–1D-A windowing, fixed-step, movement, normal combat, health, energy, input buffering, and charge-foundation behavior remains intact.
- Data-backed special definitions extend the normal move pipeline with owner, behavior, energy cost, frame timing, damage, hit-stun, knockback, hitbox, and projectile metadata.
- VOLT's **Thunder Bolt** costs 25 energy and uses 12 startup / 1 spawn-active / 20 recovery ticks. It creates one 34×22 projectile moving at 650 units/second for at most 90 fixed ticks; contact deals 16 damage, 18 hit-stun ticks, and 420 horizontal knockback.
- EMBER's **Inferno Burst** costs 25 energy and uses 16 startup / 5 active / 24 recovery ticks. Its facing-aware 110×96 short-range hitbox deals 20 damage, 24 hit-stun ticks, and 620 horizontal knockback.
- Special activation is accepted only from `Available` state with sufficient energy. Exactly 25 energy is deducted when startup begins; rejected inputs do not spend, held input does not repeat, and later interruption does not refund.
- Thunder Bolt projectiles are value state stored in an eight-slot fixed array. Each stores active state, owner, source move, position, velocity, captured direction, remaining lifetime, and current hitbox without heap allocation or raylib types.
- Projectile direction is captured at creation, projectiles never target their owner, and they continue independently if the owner turns, enters recovery/hit-stun, or is knocked out. A projectile is removed on hit/block, fixed lifetime expiry, or leaving the horizontal arena bounds.
- Fighter melee/burst contacts and all projectile contacts are snapshotted before any outcome mutates fighter state. Valid same-tick normal/special interactions therefore trade rather than being suppressed by player update order.
- Existing normal energy awards remain unchanged. Special hits and blocked specials grant no energy to either fighter, preventing special-generated meter loops.
- Blocking remains grounded and facing-dependent with no chip or block-stun. A block consumes a Thunder Bolt or Inferno Burst activation and triggers special-colored impact feedback.
- Read-only rendering adds a named Thunder Bolt casting state, cyan projectile/trail, electric impact rings, named Inferno Burst casting state, expanding orange/red explosion, fire impact rings, and visible meter deduction.
- Headless regression coverage includes all earlier tests plus activation/spending/lockouts, exact special timing, projectile spawn/movement/lifetime/ownership/direction, owner independence, blocking, burst hit/miss/facing/one-hit behavior, interruption, trades, KO, simultaneous special activation, and complete-state replay consistency including projectiles.

## Exact special and projectile semantics

- The accepted special-input tick is startup frame 1, matching normal attacks. Each phase lasts its configured processed ticks; recovery returns to `Available` on the following tick.
- Thunder Bolt transitions to a one-tick active phase after startup. Its projectile is created once on that active tick after fighter movement/collision/facing updates; it may contact at its spawn position and begins horizontal movement on the following tick.
- A projectile receives 90 contact opportunities including its spawn tick. Its remaining lifetime is decremented after contact resolution each tick.
- Existing projectiles advance before newly active moves spawn. Fighter and projectile contacts are then collected together, every source is marked consumed, combat counters advance, and all snapshotted outcomes apply.
- Thunder Bolt uses its stored projectile position as knockback origin, not the owner's later position. Inferno Burst uses its attacker's snapshotted side.
- An unblocked hit interrupts special startup/active/recovery through the existing hit-stun transition. A projectile already created remains active. Spent energy is never refunded.
- Command priority is grounded block, light, heavy, special, then charge. Insufficient special energy falls through without spending; all non-`Available` states reject activation.

## Verified functionality

- `cmake --build build/tests --parallel` completed and `ctest --test-dir build/tests --output-on-failure` passed 1 of 1 test executables.
- `cmake --build build/desktop --parallel` completed and `ctest --test-dir build/desktop --output-on-failure` passed 1 of 1 test executables.
- The native Linux/X11 application initialized an Intel/Mesa OpenGL 4.6 context, the logical render target, and its first frame. The automated check ended it after five seconds rather than exercising native controls or normal shutdown.
- `cmake --build build/web --parallel` linked the Emscripten 6.0.12/raylib 6.2 HTML, JavaScript, and Wasm outputs.
- Chrome 153 DevTools input built meter through real heavy hits, activated Thunder Bolt with `H`, and captured the named cyan projectile/trail in flight after energy dropped from 36 to 11.
- Chrome DevTools input activated Inferno Burst with the semicolon DOM key, and captured its named expanding orange/red effect, energy deduction, 20-damage impact feedback, and KO.
- Chrome reported WebGL error state 0 and no JavaScript or WebAssembly exception. The only page/network error was the unchanged harmless `/favicon.ico` 404; headless screenshot/readback warnings were also recorded.
- `git diff --check` passed after the final source and documentation edits.

## Unavailable verification

- Windows and macOS builds were not attempted because those toolchains/platforms are unavailable.
- No human-operated playtest was performed. Native Phase 1D-B combat controls/effects and normal shutdown were not exercised.
- Physical keyboard rollover/ghosting, non-US punctuation layouts, and controller input remain unverified.
- Cross-platform or cross-compiler deterministic equivalence was not tested. Replay consistency is same-build only.

## Known issues and limitations

- The projectile pool has eight fixed slots. Current energy/timing rules cannot practically exhaust it, but activation does not expose a presentation warning if no slot were available.
- Simultaneous contacts all resolve, but if several hits apply different stun/knockback to one defender on the exact tick, stable contact-array order determines the final stun/velocity while all damage still applies.
- Charge release still produces no attack, and ultimates remain unimplemented.
- There are no AI, networking, round-management changes, audio, advanced sprites, replay storage, or general state serialization.
- KO still leaves the current sandbox running with placeholder round/timer presentation.

## Next recommended task

Define a separate Phase 1D-C specification for charged-attack release levels, damage, hitboxes, and interruption/cancel behavior. Keep it in the existing move/contact pipeline and decide whether ultimate abilities remain in Phase 1D or move to a later milestone before implementation.
