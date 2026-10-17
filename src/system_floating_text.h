#pragma once

#include <moth/ecs/world.h>
#include <moth/graphics/graphics/igraphics.h>
#include <moth/graphics/utils/vector.h>
#include <cstdint>
#include <string>

class GameWorld;

// A short line of text that rises from where it was made and is gone after its time, such as "1UP".
struct ComponentFloatingText {
    std::string m_text;
    uint32_t m_msLeft = 0;
};

class SystemFloatingText {
public:
    static void Create(moth::ecs::World& registry, std::string text, moth::gfx::FloatVec2 const& position,
                       uint32_t duration);
    static void Update(GameWorld& world, uint32_t ticks);
    static void Draw(GameWorld& world, moth::gfx::IGraphics& graphics, moth::gfx::IFont& font);
};
