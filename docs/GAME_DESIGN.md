# Game Design

## Status

This document defines the initial design target. No gameplay is implemented. All move names, timings, damage values, resource values, controls, and round rules below are **provisional** and require playtesting.

## Match concept

ARENA CLASH is a 2D one-on-one fighter focused on readable decisions, distinct character identities, and short local matches. The initial playable version pits Volt against Ember on a single flat arena.

## Fighters

### Volt

A fast lightning fighter who wins through movement and quick pressure. Volt has lower per-hit damage but faster attacks and stronger repositioning.

Planned signature moves:

- **Arc Dash:** a quick advancing strike.
- **Spark Shot:** a fast, lower-damage projectile.
- **Thunder Rise:** an upward anti-air attack.
- **Storm Circuit (ultimate):** a rapid sequence of lightning strikes around the opponent.

### Ember

A powerful fire fighter who wins through space control and heavier commitments. Ember moves more slowly but deals greater damage and knockback.

Planned signature moves:

- **Flame Rush:** an advancing, armored-feeling heavy strike; armor behavior is not yet committed.
- **Fireball:** a slower, higher-impact projectile.
- **Cinder Uppercut:** a forceful anti-air attack.
- **Inferno Crown (ultimate):** a large area burst with a clear startup tell.

## Core actions

| Action | Intended behavior |
| --- | --- |
| Move | Walk left or right; fighters automatically face one another when neutral rules allow |
| Jump | Fixed initial jump impulse with air control; no double jump initially |
| Block | Hold away from the opponent; blocks eligible attacks while grounded or airborne as tuned |
| Light attack | Fast startup, low damage, short recovery, combo starter |
| Heavy attack | Slower startup, higher damage and knockback, punish tool |
| Special attack | Character-specific action that spends special energy |
| Charged attack | Hold heavy to increase power, then release; charging creates counterplay risk |
| Ultimate | High-cost character-specific attack with strong audiovisual feedback |

Crouching attacks, throws, dashes, air specials, and cancels may be evaluated later; they are not requirements for the first complete local match.

## Combat states

The initial state set is planned as:

- `Idle`, `Walk`, `Jump`, and `Fall`
- `AttackStartup`, `AttackActive`, and `AttackRecovery`
- `Charging`
- `Block` and `BlockStun`
- `HitStun` and `Knockdown`
- `RoundIntro`, `RoundOver`, and `MatchOver`

State transitions will be frame-based. A state may restrict movement, turning, blocking, or new attacks. Exact cancel rules will be introduced only with tests and documented frame data.

## Special energy

- Each fighter starts a round with **0 of 100** energy.
- Landing an attack grants **8** energy to the attacker.
- Receiving an unblocked hit grants **4** energy to the defender.
- A blocked attack grants **2** energy to the attacker and **2** to the defender.
- A standard special costs **25** energy.
- An ultimate costs **100** energy.
- Energy does not carry between rounds in the initial design.

All values are provisional. Energy awards should occur once per resolved hit, not once per active frame.

## Charged attacks

Holding heavy begins a charge if the fighter may attack. Releasing performs one of three provisional levels:

| Level | Hold duration | Damage multiplier | Additional intent |
| --- | ---: | ---: | --- |
| 1 | 0–29 frames | 1.0× | Standard heavy behavior |
| 2 | 30–59 frames | 1.25× | Increased knockback |
| 3 | 60+ frames | 1.5× | Maximum knockback and stronger effect cue |

Charge is capped at 60 simulation frames for scaling purposes. Taking a hit interrupts charging. Whether blocking can cancel a charge is deferred until playtesting.

## Provisional initial balance

| Attribute | Volt | Ember |
| --- | ---: | ---: |
| Health | 1,000 | 1,000 |
| Walk speed | 360 units/s | 300 units/s |
| Jump velocity | 780 units/s | 740 units/s |
| Light startup / active / recovery | 5 / 3 / 9 frames | 7 / 3 / 11 frames |
| Light damage | 45 | 55 |
| Heavy startup / active / recovery | 11 / 4 / 18 frames | 15 / 5 / 22 frames |
| Heavy damage | 90 | 120 |
| Projectile damage | 60 | 85 |
| Standard special cost | 25 energy | 25 energy |
| Ultimate damage target | 260 | 300 |

Shared provisional values:

- Gravity: **2,200 units/s²**
- Maximum horizontal arena distance: to be set from the first arena dimensions
- Base light hit-stun: **12 frames**
- Base heavy hit-stun: **20 frames**
- Blocked damage (chip): **10%** of normal damage, rounded consistently
- Round timer: **99 seconds**

Coordinate scale, collision dimensions, input buffer length, knockback units, combo scaling, and frame advantage require implementation experiments before values are chosen.

## Planned keyboard controls

The first local input layout is provisional and must be checked for keyboard ghosting.

| Action | Player 1 | Player 2 |
| --- | --- | --- |
| Move left / right | `A` / `D` | Left / Right arrows |
| Jump | `W` | Up arrow |
| Light attack | `F` | `K` |
| Heavy attack / hold to charge | `G` | `L` |
| Special attack | `H` | `;` (semicolon) |
| Ultimate | `R` | `O` |
| Block | Hold away | Hold away |

Menus and pause controls will be assigned during match-flow implementation. Remappable controls and gamepads are planned after the basic local controls are validated.

## Rounds and win conditions

- A match is best of three rounds; the first fighter to win two rounds wins the match.
- A round ends when a fighter reaches zero health or the 99-second timer expires.
- On timeout, the fighter with more health wins.
- Equal health on timeout produces a draw round.
- If both fighters reach zero health on the same simulation tick, the round is a double knockout and counts as a draw.
- The handling of repeated draws and a final deciding round is still unresolved and must be chosen before Phase 1E acceptance.
