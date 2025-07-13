#include "system_player_visuals.h"
#include "component_entity.h"
#include "system_drawable.h"
#include "system_shield.h"
#include "system_weapon.h"
#include <entt/entt.hpp>
#include "game_world.h"

void SystemPlayerVisuals::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    auto view = registry.view<ComponentEntity, ComponentDrawable, ComponentShield, ComponentWeapon>();
    for (auto [entity, entityData, drawable, shield, weapon] : view.each()) {
        // TODO: this is unsafe access
        drawable.m_spriteData.images["shield"].color = GetEnergyColor(entityData.m_color);
    }
}
