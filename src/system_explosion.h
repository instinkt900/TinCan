#pragma once

#include <moth/ecs/world.h>
#include <moth/graphics/graphics/igraphics.h>
#include <moth/graphics/graphics/sprite.h>
#include <moth/graphics/graphics/spritesheet.h>
#include <moth/graphics/utils/vector.h>
#include <memory>

class GameWorld;

struct ComponentExplosion {
    moth::gfx::Sprite m_sprite;

    explicit ComponentExplosion(std::shared_ptr<moth::gfx::SpriteSheet> sheet)
        : m_sprite(std::move(sheet)) {}
};

class SystemExplosion {
public:
    // plays the sheet's "explode" clip once at the position, then removes itself
    static moth::ecs::Entity CreateExplosion(moth::ecs::World& registry,
                                             std::shared_ptr<moth::gfx::SpriteSheet> const& sheet,
                                             moth::gfx::FloatVec2 const& position);
    static void Update(GameWorld& world, uint32_t ticks);
    static void Draw(GameWorld& world, moth::gfx::IGraphics& graphics);
};
