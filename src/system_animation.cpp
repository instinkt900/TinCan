#include "system_animation.h"
#include "system_drawable.h"

void SystemAnimation::Update(GameWorld& world, uint32_t ticks) {
    auto const dt = static_cast<float>(ticks) / 1000.0f;
    auto& registry = world.GetRegistry();
    auto view = registry.view<ComponentSprite>();
    for (auto [entity, drawable] : view.each()) {
        if (drawable.m_fps > 0) {
            auto const frameTime = 1.0f / drawable.m_fps;
            drawable.m_animationTime += dt;
            while (drawable.m_animationTime >= frameTime) {
                drawable.m_cellIndex =
                    (drawable.m_cellIndex + 1) % static_cast<int32_t>(drawable.m_cellRects.size());
                drawable.m_animationTime -= frameTime;
            }
        }
    }
}
