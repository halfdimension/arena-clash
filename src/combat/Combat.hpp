#pragma once

namespace arena {

struct Fighter;

enum class MoveId {
    None,
    Light,
    Heavy,
};

enum class CombatState {
    Available,
    Attacking,
    Blocking,
    HitStun,
    KnockedOut,
};

enum class AttackPhase {
    None,
    Startup,
    Active,
    Recovery,
};

struct Rectanglef {
    float x;
    float y;
    float width;
    float height;
};

struct HitboxDefinition {
    float width;
    float height;
    float forwardOffset;
    float bottomOffset;
};

struct MoveDefinition {
    MoveId id;
    int damage;
    int startupFrames;
    int activeFrames;
    int recoveryFrames;
    HitboxDefinition hitbox;
    int hitStunFrames;
    float knockbackStrength;
};

const MoveDefinition& GetMoveDefinition(MoveId moveId);
Rectanglef GetHurtbox(const Fighter& fighter);
Rectanglef GetAttackHitbox(const Fighter& fighter, const MoveDefinition& move);
bool RectanglesOverlap(const Rectanglef& first, const Rectanglef& second);
bool IsAttackInFrontOfDefender(const Fighter& attacker, const Fighter& defender);

}  // namespace arena
