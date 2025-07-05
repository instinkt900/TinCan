#include "system_drawable.h"
#include "system_movement.h"
#include <entt/entt.hpp>

void SystemDrawable::Update(entt::registry& registry, canyon::graphics::IGraphics& graphics) {
    auto view = registry.view<ComponentDrawable, ComponentPosition>();

    std::vector<Sprite*> m_opaqueSprites;
    std::vector<Sprite*> m_blendedSprites;

    for (auto [entity, drawable, pos] : view.each()) {
        for (auto& sprite : drawable.m_sprites) {
            sprite.m_cachedPos = static_cast<moth_ui::IntVec2>(pos.m_position);
            if (sprite.m_blendMode == canyon::graphics::BlendMode::Replace) {
                m_opaqueSprites.push_back(&sprite);
            } else {
                m_blendedSprites.push_back(&sprite);
            }
        }
    }

    std::sort(m_opaqueSprites.begin(), m_opaqueSprites.end(),
              [](Sprite* a, Sprite* b) { return a->m_zOrder < b->m_zOrder; });

    std::sort(m_blendedSprites.begin(), m_blendedSprites.end(),
              [](Sprite* a, Sprite* b) { return a->m_zOrder < b->m_zOrder; });

    graphics.SetBlendMode(canyon::graphics::BlendMode::Replace);
    for (auto& sprite : m_opaqueSprites) {
        graphics.SetColor(sprite->m_color);
        moth_ui::IntVec2 halfSize = sprite->m_size / 2;
        auto destRect = canyon::MakeRect(sprite->m_cachedPos.x - halfSize.x, sprite->m_cachedPos.y - halfSize.y,
                sprite->m_size.x, sprite->m_size.y);
        graphics.DrawImage(*sprite->m_image, static_cast<canyon::IntRect>(destRect), nullptr);
    }

    for (auto& sprite : m_blendedSprites) {
        graphics.SetBlendMode(sprite->m_blendMode);
        graphics.SetColor(sprite->m_color);
        moth_ui::IntVec2 halfSize = sprite->m_size / 2;
        auto destRect = canyon::MakeRect(sprite->m_cachedPos.x - halfSize.x, sprite->m_cachedPos.y - halfSize.y,
                sprite->m_size.x, sprite->m_size.y);
        graphics.DrawImage(*sprite->m_image, static_cast<canyon::IntRect>(destRect), nullptr);
    }
}
