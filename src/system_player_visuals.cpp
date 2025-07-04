#include "system_player_visuals.h"
#include "game_layer.h"
#include "system_drawable.h"
#include "system_shield.h"
#include "system_weapon.h"
#include <entt/entt.hpp>
#include "utils.h"

void SystemPlayerVisuals::Update(entt::registry& registry, uint32_t ticks) {
    auto view = registry.view<ComponentEntity, ComponentDrawable, ComponentShield, ComponentWeapon>();
    for (auto [entity, entityData, drawable, shield, weapon] : view.each()) {
        // TODO: this is unsafe access
        drawable.m_sprites[1].m_color = GetEnergyColor(entityData.m_color);
        drawable.m_sprites[1].m_size = { shield.m_radius * 2, shield.m_radius * 2 };
    }
}
