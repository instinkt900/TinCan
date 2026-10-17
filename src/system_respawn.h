#pragma once

#include "gamedata_fwd.h"
#include <moth/ecs/world.h>
#include <moth/graphics/utils/vector.h>
#include <cstdint>

class GameWorld;

// What the run carries from one ship to the next. One entity holds it for the whole run, so it outlives the ship.
// The entity also carries a ComponentInput, which keeps track of the keys while there is no ship, so a key held
// through a respawn is still held when the new ship gets control.
struct ComponentRun {
    int32_t m_lives = 0;
    int32_t m_maxLives = 0;
    int32_t m_mainGunLevel = 0;  // the level the main gun had when the ship was destroyed, as a progression index
    uint32_t m_respawnLeft = 0;  // counts down to the next ship once one is destroyed
    bool m_respawning = false;
    int32_t m_score = 0;
    int32_t m_extraLifeProgress = 0; // points towards the next extra life, kept apart so a game over can clear it
};

// A ship flying in from below the screen. It has no control until it arrives.
struct ComponentFlyIn {
    moth::gfx::FloatVec2 m_from;
    moth::gfx::FloatVec2 m_to;
    uint32_t m_elapsed = 0;
    uint32_t m_duration = 0;
};

// A ship that enemy bullets and contact cannot destroy. It blinks while this lasts.
struct ComponentSafe {
    uint32_t m_msLeft = 0;
    uint32_t m_elapsed = 0;
};

class SystemRespawn {
public:
    // takes a life for a destroyed ship, keeps what the next ship needs, and starts the wait for it
    static void OnPlayerDestroyed(moth::ecs::World& registry, moth::ecs::Entity player, GameData const& gamedata);
    // whether the ship cannot be destroyed right now: while it flies in and while it is safe
    static bool IsProtected(moth::ecs::World& registry, moth::ecs::Entity player);
    static int32_t GetLives(moth::ecs::World const& registry);
    static void Update(GameWorld& world, uint32_t ticks);
};
