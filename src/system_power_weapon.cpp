#include "system_power_weapon.h"
#include "component_entity.h"
#include "game_world.h"
#include "system_movement.h"
#include "system_projectile.h"
#include "system_shield.h"
#include <canyon/utils/math_utils.h>

void SystemPowerWeapon::InitWeapon(entt::registry& registry, entt::entity entity, std::string const& name,
                                   GameData const& gamedata) {
    auto& powerWeapon = registry.emplace<ComponentPowerWeapon>(entity);
    powerWeapon.m_activated = false;
    powerWeapon.m_projectileName = "player_homing_bullet";
    powerWeapon.m_powerPerProjectile = 10.0f;
    powerWeapon.m_damage = 50.0f;
}

void SystemPowerWeapon::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    auto view = registry.view<ComponentPowerWeapon, ComponentPower, ComponentPosition, ComponentEntity>();
    for (auto [entity, weapon, power, position, details] : view.each()) {
        if (!weapon.m_activated) {
            continue;
        }

        weapon.m_activated = false;

        // fetch the projectile data
        auto const& gamedata = world.GetGameData();
        auto const* projectileData = gamedata.Get<ProjectileData>(weapon.m_projectileName);
        if (projectileData == nullptr) {
            spdlog::error("Unable to find power weapon's projectile {}", weapon.m_projectileName);
        }

        // work out how many projectiles to fire
        auto const availablePower = power.m_power;
        auto const powerPerProjectile = weapon.m_powerPerProjectile;
        auto const projectileCount = std::floor(availablePower / powerPerProjectile);
        auto const usedPower = projectileCount * powerPerProjectile;

        // if not enough power, just exit out
        if (projectileCount <= 0) {
            continue;
        }

        // subtract the power
        power.m_power -= usedPower;

        // work out the angle for the projectiles.
        auto const maxSeparation = canyon::Radians(10.0f);
        auto const anglePerProjectile = std::max(maxSeparation, canyon::Radians(360.0f) / projectileCount);
        auto const fullSpread = anglePerProjectile * projectileCount;
        auto const startAngle = -fullSpread / 2.0f;

        for (uint32_t i = 0; i < static_cast<uint32_t>(projectileCount); ++i) {
            auto const angle = startAngle + (anglePerProjectile * static_cast<float>(i));
            auto const direction = canyon::Rotate2D({ 0.0f, -1.0f }, angle);
            SystemProjectile::CreateProjectile(registry, *projectileData, entity, position.m_position,
                                               direction, angle, weapon.m_damage);
        }
    }
}
