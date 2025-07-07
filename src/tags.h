#pragma once

// Marks the entity as the player entity. Should only ever be one.
struct PlayerTag{};

// Marks the entity as a hostile entity to the player.
struct EnemyTag{};

// Marks the entity as dead and that the lifetime system should remove it.
struct DeadTag{};

