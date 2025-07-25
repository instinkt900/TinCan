#include "system_animation.h"
#include "system_drawable.h"

void SystemAnimation::Update(GameWorld &world, uint32_t ticks) {
    auto const dt = static_cast<float>(ticks) / 1000.0f;
    auto& registry = world.GetRegistry();
    auto view = registry.view<ComponentDrawable>();
    for (auto [entity, drawable]: view.each()) {
        for (auto& [name, sprite] : drawable.m_sprites) {
            if (sprite.m_fps > 0) {
                auto const frameTime = 1.0f / sprite.m_fps;
                sprite.m_animationTime += dt;
                while (sprite.m_animationTime >= frameTime) {
                    sprite.m_cellIndex = (sprite.m_cellIndex + 1) % static_cast<int32_t>(sprite.m_cellRects.size());
                    sprite.m_animationTime -= frameTime;
                }
            }
        }
    }
}

