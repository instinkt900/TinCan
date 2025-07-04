#pragma once

#include <canyon/graphics/igraphics.h>
#include <entt/entity/fwd.hpp>
#include <moth_ui/utils/vector.h>
#include <vector>

struct Sprite {
    std::shared_ptr<canyon::graphics::IImage> m_image;
    moth_ui::FloatVec2 m_size;
    canyon::graphics::Color m_color = canyon::graphics::BasicColors::White;
    canyon::graphics::BlendMode m_blendMode = canyon::graphics::BlendMode::Replace;
    int32_t m_zOrder = 0;

    // drawable system used
    moth_ui::FloatVec2 m_cachedPos;
};

struct ComponentDrawable {
    std::vector<Sprite> m_sprites;
};

class SystemDrawable {
public:
    static void Update(entt::registry& registry, canyon::graphics::IGraphics& graphics);
};
