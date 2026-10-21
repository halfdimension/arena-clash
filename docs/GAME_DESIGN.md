# Game Design

## Status

Phase 1E local matches and Phase 1D-C charged attacks are implemented and verified in automated Linux builds/tests and a Chrome/WebAssembly runtime. Values labeled **implemented** below are simulation truth. Ultimates, combos, audio, pause, and AI remain planned and require later design/playtesting.

## Match concept

ARENA CLASH is a 2D one-on-one fighter focused on readable decisions, distinct character identities, and short local matches. VOLT and EMBER fight first-to-two rounds on one flat arena with a fixed-tick countdown, KO/timeout resolution, complete round resets, match victory, and rematch.

## Fighters

### VOLT

A fast lightning fighter intended to win through movement and quick pressure. VOLT currently has the faster walk/jump tuning but shares Phase 1C normal-attack data with EMBER.

Signature moves:

- **Thunder Bolt (implemented):** a fast, lower-damage horizontal lightning projectile.
- **Thunder Strike (implemented):** a quick, long-reaching three-level charged melee strike.
- **Arc Dash:** a quick advancing strike.
- **Thunder Rise:** an upward anti-air attack.
- **Storm Circuit (ultimate):** a rapid sequence of lightning strikes around the opponent.

### EMBER

A powerful fire fighter intended to win through space control and heavier commitments. EMBER currently has slower movement and shares Phase 1C normal-attack data with VOLT.

Signature moves:

- **Inferno Burst (implemented):** a short-range fire blast with high damage and knockback.
- **Flame Smash (implemented):** a slower three-level charged melee smash with heavy knockback.
- **Flame Rush:** an advancing, armored-feeling heavy strike; armor behavior is not committed.
- **Cinder Uppercut:** a forceful anti-air attack.
- **Inferno Crown (ultimate):** a large area burst with a clear startup tell.

## Core actions

| Action | Current behavior |
| --- | --- |
| Move | **Implemented:** walk left/right while available; fighters automatically face one another |
| Jump | **Implemented:** fixed initial jump impulse, 75% horizontal air control, and no double jump |
| Block | **Implemented:** hold the dedicated guard button while grounded; blocks normals and specials from the facing side with no chip/block-stun |
| Light attack | **Implemented:** fast 6-damage normal with short reach and recovery |
| Heavy attack | **Implemented:** slower 14-damage normal with greater reach, stun, and knockback |
| Special attack | **Implemented:** VOLT fires Thunder Bolt and EMBER performs Inferno Burst for 25 energy |
| Charged attack | **Implemented:** hold/release selects one of three Thunder Strike or Flame Smash levels and enters normal attack phases |
| Ultimate | **Planned:** high-cost character-specific attack |

Crouching attacks, throws, dashes, air specials, and cancels are not part of Phase 1C.

## Combat states and transitions — Implemented

- `Available`: may move, jump, start a normal/special attack or charge, or enter grounded block.
- `Attacking`: owns `Startup`, `Active`, or `Recovery`; it cannot start another action. Grounded attacks stop horizontal movement, while airborne attacks retain existing momentum.
- `Blocking`: requires the held guard command and grounded state. Block wins over an attack pressed on the same tick and exits when released.
- `Charging`: grounded fixed-tick hold state; stops movement and rejects jump, block entry, normal attacks, and special activation until release/interruption.
- `HitStun`: rejects movement/attacks while the configured stun ticks and knockback are processed.
- `KnockedOut`: entered at 0 HP and cannot initiate actions; after all same-tick contacts resolve, the match state transitions to round over.

The accepted press tick is startup frame 1. Startup, active, and recovery each last exactly their configured number of fixed ticks. The next phase begins at the start of the tick after the prior phase's final frame; availability returns at the start of the tick after final recovery. There are no cancels.

Both active attacks are checked before either result changes state. If both valid hitboxes overlap on the same tick, both damage outcomes resolve. A hit or block consumes the attack activation so later active frames cannot connect again.

## Phase 1C move data — Implemented at 60 Hz

| Move | Damage | Startup | Active | Recovery | Hitbox (W×H) | Forward / bottom offset | Hit-stun | Knockback |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Light | 6 | 5 | 3 | 10 | 52×44 | 4 / 42 | 12 | 300 |
| Heavy | 14 | 12 | 4 | 17 | 72×60 | 6 / 26 | 20 | 480 |

The hitbox begins beyond the attacker's 64-unit-wide movement body, mirrors with facing, and is positioned vertically from the fighter's bottom-center anchor. Hurtboxes currently equal the 64×112 movement body. All values are provisional balance even though they are implemented.

## Phase 1D-B special move data — Implemented at 60 Hz

| Move | Fighter | Cost | Damage | Startup | Active | Recovery | Hitbox (W×H) | Hit-stun | Knockback | Projectile |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| Thunder Bolt | VOLT | 25 | 16 | 12 | 1 spawn tick | 20 | 34×22 | 18 | 420 | 650 units/s, 90-tick maximum lifetime |
| Inferno Burst | EMBER | 25 | 20 | 16 | 5 | 24 | 110×96 | 24 | 620 | None |

Thunder Bolt creates one projectile on its active tick. The projectile captures its firing direction, does not track its owner, cannot hit its owner, persists if its owner is later hit or knocked out, and disappears on hit, block, horizontal arena exit, or lifetime expiry. Its spawn tick is its first collision opportunity and counts toward the 90-tick cap.

Inferno Burst is a facing-aware short-range melee effect using forward/bottom offsets of −16 / 8 from EMBER's anchor. A contact consumes the activation, so its five active ticks cannot damage or trigger block repeatedly. Both specials use the same pre-mutation contact snapshot as normal attacks, allowing fair normal/special/projectile trades. Specials do not award energy on hit or block.

## Phase 1D-C charged move data — Implemented at 60 Hz

| Move | Fighter | Level ticks | Damage | Startup | Active | Recovery | Hitbox (W×H) | Forward / bottom offset | Hit-stun | Knockback |
| --- | --- | --- | ---: | ---: | ---: | ---: | ---: | --- | ---: | ---: |
| Thunder Strike L1 | VOLT | 18–35 | 16 | 8 | 3 | 18 | 96×60 | 6 / 28 | 18 | 440 |
| Thunder Strike L2 | VOLT | 36–59 | 23 | 8 | 3 | 18 | 96×60 | 6 / 28 | 21 | 500 |
| Thunder Strike L3 | VOLT | 60–72 | 30 | 8 | 3 | 18 | 96×60 | 6 / 28 | 24 | 560 |
| Flame Smash L1 | EMBER | 18–35 | 16 | 13 | 4 | 23 | 76×72 | 4 / 20 | 20 | 560 |
| Flame Smash L2 | EMBER | 36–59 | 23 | 13 | 4 | 23 | 76×72 | 4 / 20 | 23 | 680 |
| Flame Smash L3 | EMBER | 60–72 | 30 | 13 | 4 | 23 | 76×72 | 4 / 20 | 26 | 800 |

Thunder Strike is faster and reaches farther horizontally. Flame Smash is slower and shorter but produces stronger knockback at every level. Both are facing-aware melee attacks with no projectile. A valid release starts startup rather than dealing immediate damage. Hits and blocks consume the activation, and the shared pre-mutation contact snapshot permits fair charged trades and charged-vs-normal/special trades.

## Health, guard, and knockback — Implemented

- Each fighter starts with **100 HP**.
- Unblocked damage is subtracted once and clamped to zero.
- A grounded guard prevents all current normal/special/charged damage when the contact origin is on the side the defender faces.
- There is no chip damage or block-stun.
- A successful unblocked hit interrupts the defender into hit-stun, applies horizontal knockback away from the attacker, and triggers a short hit flash.
- Ground knockback is multiplied by **0.82 per fixed tick** and remains constrained by body collision and stage edges.
- Reaching 0 HP enters KO. Round resolution occurs after all valid same-tick contacts apply, allowing double KO without player-order bias.

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
| Special attack | `H` | Semicolon (`;`) |
| Charge hold/release | `J` | Apostrophe (`'`) |
| Rematch after match over | `R` or Enter | `R` or Enter |
| Debug hurtboxes/hitboxes | `F1` | `F1` |

Both players may act simultaneously. Press/release edges are buffered until one simulation tick and consumed once; held values remain available for catch-up ticks. Held normal-attack buttons do not repeat a move without a new press. Browser input uses raylib's physical DOM-code mapping for semicolon/quote. Physical keyboard ghosting, rollover, and non-US punctuation layouts still depend on hardware/browser. Remapping and controllers remain future work.

## Special energy and charge rules — Implemented through Phase 1D-C

- Each fighter starts at **0 energy** with a **100-energy maximum**.
- A successful light hit grants **8** energy to its attacker; a successful heavy grants **12**.
- A blocked normal grants **4** to the attacker and **6** to the defender.
- Each connected activation awards only once, including blocks; gains clamp to 100.
- Both implemented specials cost **25**. A special may activate only while the fighter is `Available` with at least 25 energy; startup and the energy deduction occur atomically. Rejected inputs leave energy unchanged, interruption does not refund, and holding the key does not repeat activation.
- Special and charged hits/blocks award no energy to either fighter. Charged activation costs no energy; normal-hit and normal-block awards remain unchanged.
- Charge time is measured only in fixed simulation ticks. The accepted press tick is charge frame 1; level thresholds are **18**, **36**, and **60** ticks, and duration caps at **72 ticks**.
- Releasing at 0–17 ticks cancels. Releasing at 18–35, 36–59, or 60–72 selects level 1, 2, or 3 and immediately begins the configured startup phase; release itself cannot deal damage.
- A valid unblocked hit interrupts a fighter who is still charging. A release is processed during command validation, so a release on the impact tick resolves before that tick's later collision snapshot.
- Command priority is block, light, heavy, special, then charge. While charging, movement, jump, new block, normal attacks, and specials are locked out.

Charge duration is cleared after cancel, release, or interruption, and release edges are consumed once by the existing command buffer. Charged startup, active, and recovery use the same combat lockouts and one-hit bookkeeping as every other attack. Ultimate abilities remain the unimplemented portion of Phase 1D.

## Rounds and win conditions — Implemented in Phase 1E

- A match begins with a **3-second (180-tick) RoundIntro**. Fighters cannot act and the round timer does not decrease during the intro.
- Each fighting round has **90 seconds (5,400 ticks)**. The timer decreases once per `Fighting` simulation tick, clamps at zero, and is frozen during intro, round over, and match over. The HUD displays the ceiling of remaining ticks divided by 60, so it shows 90 at round start and 0 at expiration.
- Combat completes before round adjudication. All snapshotted same-tick contacts apply first; KO is then checked before timeout. Therefore a KO on the final timer tick is a KO result, and simultaneous final hits may produce double KO.
- If one fighter has 0 HP, the other wins. If both have 0 HP, the result is **Double K.O. / draw**. If time expires with neither knocked out, higher remaining health wins; equal health is a draw.
- A win awards exactly one round point. Draw and double-KO rounds award neither fighter. Results hold for **3 seconds (180 ticks)** with combat frozen.
- The first fighter to **two round wins** wins the match. Because draws are replayed and still advance the displayed round number, this is commonly best-of-three but is not a strict maximum of three rounds.
- Between rounds, both fighters return to 100 health, 0 energy, canonical start positions, zero velocity, grounded/facing defaults, and `Available`. Attack, hit-stun, charge, release outcome, connection/effect, and presentation timers are cleared; all projectiles are removed; the 90-second timer and intro restart. Round-win scores remain.
- One-shot inputs are consumed even while combat is disabled. Presses during result/intro phases and held attack/charge buttons do not become fresh presses in the next round.
- After the result hold for a fighter's second win, the game enters `MatchOver`, freezes combat/timer, displays `VOLT WINS` or `EMBER WINS`, and accepts buffered `R` or Enter. Rematch resets scores and all round state and begins a new Round 1 intro.
