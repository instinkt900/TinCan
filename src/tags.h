#pragma once

// Marks the entity as the player entity. Should only ever be one.
struct PlayerTag{};

// Marks the entity as a hostile entity to the player.
struct EnemyTag{};

// Marks the entity as a target for a projectile. Projectiles will hit targets not in its team.
struct TargetTag{};

// Marks the entity as dead and that the lifetime system should remove it.
struct DeadTag{};

// Marks a dead entity as having left the play area under its own steam rather than having been killed.
// Set alongside DeadTag, so that whatever removes the entity can tell its group which of the two happened.
struct EscapedTag{};

// Marks an enemy whose death is a big moment: a bigger explosion, a shake and a hit-stop.
struct LargeTag{};

// Marks the entity as playing a one-shot animation. The animation system kills
// it once the animation stops.
struct OneShotTag{};
