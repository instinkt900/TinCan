#pragma once

struct SpriteData;

// The sprite spawned where the entity dies. Points into the game data, which
// outlives the world.
struct ComponentDeathSprite {
    SpriteData const* m_sprite = nullptr;
};
