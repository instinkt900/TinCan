#include "system_sprite.h"
#include "system_movement.h"
#include <entt/entt.hpp>

void SystemSprite::Update(entt::registry& registry, canyon::graphics::IGraphics& graphics) {
    auto view = registry.view<ComponentSprite, ComponentPosition>();

    for (auto [entity, sprite, pos] : view.each()) {
        moth_ui::FloatVec2 halfSize = sprite.m_size / 2;
        auto destRect = canyon::MakeRect(pos.m_position.x - halfSize.x, pos.m_position.y - halfSize.y, sprite.m_size.x, sprite.m_size.y);
        graphics.DrawImage(*sprite.m_sprite, static_cast<canyon::IntRect>(destRect), nullptr);
    }
}
