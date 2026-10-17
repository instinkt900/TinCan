#pragma once

#include "gamedata_fwd.h"
#include <moth/ecs/world.h>

// Applies damage to an entity and handles everything that follows from it: the death tag, the
// death sprite, whatever the entity drops, the camera shake and the damage flash. Anything that
// hurts an entity goes through here so the consequences of dying are the same however it died.
//
// The damage is the final amount. Multipliers that belong to the thing doing the damage, such as
// the projectile affinity bonus, are applied by the caller.
class SystemDamage {
public:
    static void ApplyDamage(moth::ecs::World& registry, moth::ecs::Entity target, float damage,
                            GameData const& gamedata);

    // The ship has no health: one hit destroys it, with the explosion, shake, hit-stop and sound that go
    // with it. ApplyDamage sends any damage to the player here.
    static void DestroyPlayer(moth::ecs::World& registry, moth::ecs::Entity player, GameData const& gamedata);
};
