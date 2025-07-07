#pragma once

#include "gamedata_sprite.h"
#include <canyon/graphics/igraphics.h>
#include <entt/entity/fwd.hpp>
#include <moth_ui/utils/vector.h>

struct ComponentDrawable {
    SpriteData m_spriteData;
};

class SystemDrawable {
public:
    static void Update(entt::registry& registry, canyon::graphics::IGraphics& graphics);
};
