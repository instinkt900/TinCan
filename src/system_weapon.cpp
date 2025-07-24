#include "system_weapon.h"
#include "component_entity.h"
#include "component_passives.h"
#include "gamedata_weapon.h"
#include "system_drawable.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include "system_projectile.h"
#include "system_world_bounds.h"
#include "tags.h"
#include <entt/entt.hpp>
#include <moth_ui/utils/vector_utils.h>
#include <spdlog/spdlog.h>
#include "game_world.h"

namespace {
    int const ProjectileBorder = 200;
}

ComponentWeapon::ComponentWeapon(WeaponData const& weapon)
    : m_damage(weapon.damage)
    , m_cooldown(weapon.cooldown)
    , m_maxCooldown(weapon.cooldown)
    , m_burst(weapon.burst)
    , m_maxBurst(weapon.burst)
    , m_burstCooldown(weapon.burst_delay)
    , m_maxBurstCooldown(weapon.burst_delay)
    , m_playerTracking(weapon.player_tracking)
    , m_projectile(*weapon.projectile) {
    if (weapon.pickup.has_value()) {
        m_pickup = *weapon.pickup.value();
    }
    m_barrelGroups.clear();
    m_barrelGroupIds.clear();
    for (auto const& barrel : weapon.barrels) {
        m_barrelGroups[barrel.group].push_back({ barrel.offset, barrel.angle });
    }
    for (auto const& [id, group] : m_barrelGroups) {
        m_barrelGroupIds.push_back(id);
    }
}

ComponentWeapon* SystemWeapon::InitWeapon(entt::registry& registry, entt::entity entity,
                                          WeaponData const& weaponData, GameData const& gamedata) {
    auto& weapon = registry.get_or_emplace<ComponentWeapon>(entity, weaponData);
    return &weapon;
}

struct Passives {
    float cooldownMult = 1.0f;
    float burstCooldownMult = 1.0f;
    float damageMult = 1.0f;
    float burstAdd = 0.0f;
};

void CollectPassives(entt::registry& registry, entt::entity entity, Passives& passives) {
    if (auto const* component = registry.try_get<ComponentPassives>(entity)) {
        passives.cooldownMult = PassiveValue(*component, PassiveType::WeaponCooldown);
        passives.burstCooldownMult = PassiveValue(*component, PassiveType::WeaponCooldownBurst);
        passives.damageMult = PassiveValue(*component, PassiveType::WeaponDamage);
        passives.burstAdd = PassiveValue(*component, PassiveType::WeaponBurst);
    }
}

void SystemWeapon::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    ComponentPosition* playerPosition = nullptr;
    auto playerView = registry.view<ComponentPosition, PlayerTag>();
    if (playerView.begin() != playerView.end()) {
        auto playerEntity = *playerView.begin();
        playerPosition = registry.try_get<ComponentPosition>(playerEntity);
    }

    auto view = registry.view<ComponentWeapon, ComponentEntity, ComponentPosition>();
    for (auto [entity, weapon, entityData, position] : view.each()) {

        Passives passives;
        CollectPassives(registry, entity, passives);

        auto const weaponCooldown = static_cast<float>(weapon.m_maxCooldown) * passives.cooldownMult;
        auto const burstCooldown =
            std::max(50.0f, static_cast<float>(weapon.m_maxBurstCooldown) * passives.burstCooldownMult);
        auto const burstCount = weapon.m_maxBurst + static_cast<int32_t>(passives.burstAdd);
        auto const damage = weapon.m_damage * passives.damageMult;

        if (weapon.m_cooldown > 0) {
            weapon.m_cooldown -= static_cast<int32_t>(ticks);
        }

        if (weapon.m_cooldown <= 0 && weapon.m_active) {
            weapon.m_burst = static_cast<int32_t>(burstCount);
            weapon.m_cooldown += static_cast<int32_t>(weaponCooldown);
        }

        if (weapon.m_burst > 0) {
            if (weapon.m_burstCooldown > 0) {
                weapon.m_burstCooldown -= static_cast<int32_t>(ticks);
            }

            while (weapon.m_burstCooldown <= 0 && weapon.m_burst > 0 &&
                   (weapon.m_active || weapon.m_burst < burstCount)) {
                weapon.m_burst -= 1;
                weapon.m_burstCooldown += static_cast<int32_t>(burstCooldown);

                auto const barrelGroupId = weapon.m_barrelGroupIds[weapon.m_barrelGroupIndex];
                auto const& barrelGroup = weapon.m_barrelGroups[barrelGroupId];
                weapon.m_barrelGroupIndex =
                    (weapon.m_barrelGroupIndex + 1) % static_cast<int32_t>(weapon.m_barrelGroupIds.size());
                for (auto const& barrel : barrelGroup) {
                    auto const barrelAngle = entityData.m_angle + barrel.m_angle;
                    auto direction = moth_ui::Rotate2D({ 0, 1.0f }, barrelAngle);
                    auto const offset =
                        moth_ui::Rotate2D({ barrel.m_offset.x, -barrel.m_offset.y }, entityData.m_angle);

                    if (weapon.m_playerTracking && (playerPosition != nullptr)) {
                        auto delta = position.m_position - playerPosition->m_position;
                        direction = moth_ui::Normalized(delta);
                    }

                    auto const* projectileData = &weapon.m_projectile;
                    if (projectileData != nullptr) {
                        auto const projectileEntity = SystemProjectile::CreateProjectile(
                            registry, *projectileData, entity, position.m_position + offset, direction,
                            barrelAngle, damage);
                        auto& projectileBounds = registry.emplace<ComponentBounds>(projectileEntity);
                        projectileBounds.m_bounds = static_cast<canyon::FloatRect>(canyon::MakeRect(
                            -ProjectileBorder, -ProjectileBorder, world.GetWorldSize().x + ProjectileBorder,
                            world.GetWorldSize().y + ProjectileBorder));
                        projectileBounds.m_behaviour = BoundsBehaviour::Kill;
                    }
                }
            }
        }
    }
}
