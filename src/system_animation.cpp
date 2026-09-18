#include "system_animation.h"
#include "system_drawable.h"

void SystemAnimation::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    auto view = registry.View<ComponentSprite>();
    for (auto [entity, sprite] : view.each()) {
        if (sprite.m_animator) {
            // Animator and gfx::Sprite both take milliseconds; pass ticks straight through.
            sprite.m_animator->Update(ticks);
        }
    }
}
