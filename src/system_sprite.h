#pragma once

#include <canyon/graphics/igraphics.h>
#include <entt/entity/fwd.hpp>
#include <moth_ui/utils/vector.h>

struct ComponentSprite {
    std::shared_ptr<canyon::graphics::IImage> m_sprite;
    moth_ui::FloatVec2 m_size;
};

class SystemSprite {
public:
    static void Update(entt::registry& registry, canyon::graphics::IGraphics& graphics);
};
