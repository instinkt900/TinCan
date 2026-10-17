#include "system_hit_stop.h"
#include "game_world.h"
#include <algorithm>

void SystemHitStop::Start(moth::ecs::World& registry, uint32_t milliseconds) {
    auto view = registry.View<ComponentHitStop>();
    for (auto [entity, hitStop] : view.each()) {
        hitStop.m_msLeft = std::max(hitStop.m_msLeft, milliseconds);
        return;
    }
}

bool SystemHitStop::Freeze(GameWorld& world, uint32_t ticks) {
    auto view = world.GetRegistry().View<ComponentHitStop>();
    for (auto [entity, hitStop] : view.each()) {
        if (hitStop.m_msLeft == 0) {
            return false;
        }
        hitStop.m_msLeft -= std::min(hitStop.m_msLeft, ticks);
        return true;
    }
    return false;
}
