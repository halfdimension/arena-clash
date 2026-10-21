# Game Design

## Status

Phase 1D-A energy and charge-state foundations are implemented and verified alongside Phase 1C movement and normal combat. Values labeled **implemented** below are simulation truth; actual special/charged attacks, projectiles, ultimates, combos, audio, and round flow remain **planned/provisional** and require later implementation and playtesting.

## Match concept

ARENA CLASH is a 2D one-on-one fighter focused on readable decisions, distinct character identities, and short local matches. The current combat sandbox pits VOLT against EMBER on one flat arena; it reaches KO but does not yet run or reset complete rounds.

## Fighters

### VOLT

A fast lightning fighter intended to win through movement and quick pressure. VOLT currently has the faster walk/jump tuning but shares Phase 1C normal-attack data with EMBER.

Planned signature moves:

- **Arc Dash:** a quick advancing strike.
- **Spark Shot:** a fast, lower-damage projectile.
- **Thunder Rise:** an upward anti-air attack.
- **Storm Circuit (ultimate):** a rapid sequence of lightning strikes around the opponent.

### EMBER

A powerful fire fighter intended to win through space control and heavier commitments. EMBER currently has slower movement and shares Phase 1C normal-attack data with VOLT.

Planned signature moves:

- **Flame Rush:** an advancing, armored-feeling heavy strike; armor behavior is not committed.
- **Fireball:** a slower, higher-impact projectile.
- **Cinder Uppercut:** a forceful anti-air attack.
- **Inferno Crown (ultimate):** a large area burst with a clear startup tell.

## Core actions

| Action | Current behavior |
| --- | --- |
| Move | **Implemented:** walk left/right while available; fighters automatically face one another |
| Jump | **Implemented:** fixed initial jump impulse, 75% horizontal air control, and no double jump |
| Block | **Implemented:** hold the dedicated guard button while grounded; blocks Phase 1C normals from the facing side with no chip/block-stun |
| Light attack | **Implemented:** fast 6-damage normal with short reach and recovery |
| Heavy attack | **Implemented:** slower 14-damage normal with greater reach, stun, and knockback |
| Special attack | **Foundation implemented:** dedicated command and 25-energy affordability/spending rule exist; no move activates yet |
| Charged attack | **Foundation implemented:** dedicated hold/release command drives charge timing/outcomes; no hitbox or damage exists yet |
| Ultimate | **Planned Phase 1D-B:** high-cost character-specific attack |

Crouching attacks, throws, dashes, air specials, and cancels are not part of Phase 1C.

## Combat states and transitions — Implemented

- `Available`: may move, jump, start a normal attack or charge, or enter grounded block.
- `Attacking`: owns `Startup`, `Active`, or `Recovery`; it cannot start another action. Grounded attacks stop horizontal movement, while airborne attacks retain existing momentum.
- `Blocking`: requires the held guard command and grounded state. Block wins over an attack pressed on the same tick and exits when released.
- `Charging`: grounded fixed-tick hold state; stops movement and rejects jump, block entry, normal attacks, and special activation until release/interruption.
- `HitStun`: rejects movement/attacks while the configured stun ticks and knockback are processed.
- `KnockedOut`: entered at 0 HP and cannot initiate actions; round-over handling is deferred.

The accepted press tick is startup frame 1. Startup, active, and recovery each last exactly their configured number of fixed ticks. The next phase begins at the start of the tick after the prior phase's final frame; availability returns at the start of the tick after final recovery. There are no cancels.

Both active attacks are checked before either result changes state. If both valid hitboxes overlap on the same tick, both damage outcomes resolve. A hit or block consumes the attack activation so later active frames cannot connect again.

## Phase 1C move data — Implemented at 60 Hz

| Move | Damage | Startup | Active | Recovery | Hitbox (W×H) | Forward / bottom offset | Hit-stun | Knockback |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Light | 6 | 5 | 3 | 10 | 52×44 | 4 / 42 | 12 | 300 |
| Heavy | 14 | 12 | 4 | 17 | 72×60 | 6 / 26 | 20 | 480 |

The hitbox begins beyond the attacker's 64-unit-wide movement body, mirrors with facing, and is positioned vertically from the fighter's bottom-center anchor. Hurtboxes currently equal the 64×112 movement body. All values are provisional balance even though they are implemented.

## Health, guard, and knockback — Implemented

- Each fighter starts with **100 HP**.
- Unblocked damage is subtracted once and clamped to zero.
- A grounded guard prevents all Phase 1C light/heavy damage when the attacker is on the side the defender faces.
- There is no chip damage or block-stun.
- A successful unblocked hit interrupts the defender into hit-stun, applies horizontal knockback away from the attacker, and triggers a short hit flash.
- Ground knockback is multiplied by **0.82 per fixed tick** and remains constrained by body collision and stage edges.
- Reaching 0 HP enters KO. Full round resolution/reset is not implemented.

## Movement values — Implemented

| Attribute | VOLT | EMBER |
| --- | ---: | ---: |
| Health | 100 | 100 |
| Walk speed | 360 units/s | 300 units/s |
| Jump velocity | 780 units/s | 740 units/s |

Shared values:

- Gravity: **2,200 units/s²**
- Horizontal stage edges: **x=80 and x=1,200**; fighter centers are inset by half their body width
- Ground plane: **564 world units**
- Movement body/hurtbox: **64 units wide × 112 units tall**
- Air-control multiplier: **0.75**

Left and right together cancel. Jump begins only from a just-pressed command while available and grounded; holding jump through landing does not jump again. Grounded body overlap is resolved symmetrically. A sufficiently high jump may cross over, after which both fighters face each other.

## Keyboard controls — Implemented

| Action | Player 1 — VOLT | Player 2 — EMBER |
| --- | --- | --- |
| Move left / right | `A` / `D` | Left / Right arrows |
| Jump | `W` | Up arrow |
| Block | `S` | Down arrow |
| Light attack | `F` | `K` |
| Heavy attack | `G` | `L` |
| Special command (no move yet) | `H` | Semicolon (`;`) |
| Charge hold/release | `J` | Apostrophe (`'`) |
| Debug hurtboxes/hitboxes | `F1` | `F1` |

Both players may act simultaneously. Press/release edges are buffered until one simulation tick and consumed once; held values remain available for catch-up ticks. Held normal-attack buttons do not repeat a move without a new press. Browser input uses raylib's physical DOM-code mapping for semicolon/quote. Physical keyboard ghosting, rollover, and non-US punctuation layouts still depend on hardware/browser. Remapping and controllers remain future work.

## Special energy and charge foundation — Implemented Phase 1D-A

- Each fighter starts at **0 energy** with a **100-energy maximum**.
- A successful light hit grants **8** energy to its attacker; a successful heavy grants **12**.
- A blocked normal grants **4** to the attacker and **6** to the defender.
- Each connected activation awards only once, including blocks; gains clamp to 100.
- The reserved special cost is **25**. A future special may spend only while the fighter is `Available` and has at least 25; a refused spend leaves energy unchanged. Phase 1D-A special commands do not invoke spending because no actual move exists.
- Charge time is measured only in fixed simulation ticks. The accepted press tick is charge frame 1, minimum readiness is **18 ticks**, and duration caps at **72 ticks**.
- Releasing at 0–17 ticks cancels; releasing at 18–72 produces a one-tick `Ready` outcome for future charged-move activation.
- A valid unblocked hit interrupts a fighter who is still charging. A release is processed during command validation, so a release on the impact tick resolves before that tick's later collision snapshot.
- Block retains priority over normal/charge buttons, and existing light-before-heavy behavior is preserved. Normal attacks are checked before charge on the same tick; while charging, movement, jump, new block, normal attacks, and future specials are locked out.

Phase 1D-A has no charged attack damage, hitbox, startup/active/recovery phases, levels, or release animation. Character-specific special attacks, projectiles, ultimates, and final charged-attack behavior belong to Phase 1D-B.

## Rounds and win conditions — Planned Phase 1E

- A future match is intended to use best-of-three rounds and a 99-second timer.
- KO, timeout, draw, double-KO scoring, round reset, match-over, and rematch behavior are not implemented.
- The current sandbox leaves fighters in KO and keeps the HUD's round/timer labels static.
- Repeated draws and a final deciding-round policy remain unresolved.
