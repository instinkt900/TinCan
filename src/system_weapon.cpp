#include "system_weapon.h"
#include "component_entity.h"
#include "component_passives.h"
#include "system_drawable.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include "system_projectile.h"
#include "tags.h"
#include <entt/entt.hpp>
#include <moth_ui/utils/vector_utils.h>
#include <spdlog/spdlog.h>
#include "game_world.h"

ComponentWeapon* SystemWeapon::InitWeapon(entt::registry& registry, entt::entity entity,
                                          std::string const& name, GameData const& gamedata) {
    if (name == "") {
        return nullptr;
    }

    auto const* weaponData = gamedata.GetWeaponDatabase().Get(name);
    if (weaponData == nullptr) {
        spdlog::error("SystemWeapon::InitWeapon - Unable to get weapon {}", name);
        return nullptr;
    }

    auto& weapon = registry.get_or_emplace<ComponentWeapon>(entity);
    weapon.m_active = false;
    weapon.m_pickupName = weaponData->pickup_name;
    weapon.m_cooldown = weaponData->cooldown;
    weapon.m_maxCooldown = weaponData->cooldown;
    weapon.m_burst = weaponData->burst;
    weapon.m_maxBurst = weaponData->burst;
    weapon.m_burstCooldown = weaponData->burst_delay;
    weapon.m_maxBurstCooldown = weaponData->burst_delay;
    weapon.m_playerTracking = weaponData->player_tracking;
    weapon.m_projectileName = weaponData->projectile_name;
    weapon.m_barrelGroupIndex = 0;
    weapon.m_barrelGroups.clear();
    weapon.m_barrelGroupIds.clear();
    for (auto const& barrel : weaponData->barrels) {
        weapon.m_barrelGroups[barrel.group].push_back({ barrel.offset, barrel.angle });
    }
    for (auto const& [id, group] : weapon.m_barrelGroups) {
        weapon.m_barrelGroupIds.push_back(id);
    }

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
    auto const& gamedata = world.GetGameData();
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

            if (weapon.m_burstCooldown <= 0 && (weapon.m_active || weapon.m_burst < burstCount)) {
                weapon.m_burst -= 1;
                weapon.m_burstCooldown += static_cast<int32_t>(burstCooldown);

                auto const barrelGroupId = weapon.m_barrelGroupIds[weapon.m_barrelGroupIndex];
                auto const& barrelGroup = weapon.m_barrelGroups[barrelGroupId];
                weapon.m_barrelGroupIndex =
                    (weapon.m_barrelGroupIndex + 1) % static_cast<int32_t>(weapon.m_barrelGroupIds.size());
                for (auto const& barrel : barrelGroup) {
                    auto direction = moth_ui::Rotate2D({ 0, 1.0f }, barrel.m_angle + entityData.m_angle);
                    auto offset =
                        moth_ui::Rotate2D({ barrel.m_offset.x, -barrel.m_offset.y }, entityData.m_angle);

                    if (weapon.m_playerTracking && (playerPosition != nullptr)) {
                        auto delta = position.m_position - playerPosition->m_position;
                        direction = moth_ui::Normalized(delta);
                    }

                    auto const* projectileData =
                        gamedata.GetProjectileDatabase().Get(weapon.m_projectileName);
                    if (projectileData != nullptr) {
                        auto projectile = registry.create();
                        auto& projectileComp = registry.emplace<ComponentProjectile>(projectile);
                        auto& projectileEntityData = registry.emplace<ComponentEntity>(projectile);
                        auto& projectilePos = registry.emplace<ComponentPosition>(projectile);
                        auto& projectileVel = registry.emplace<ComponentVelocity>(projectile);
                        auto& projectileDrawable = registry.emplace<ComponentDrawable>(projectile);
                        auto& projectileLifetime = registry.emplace<ComponentLifetime>(projectile);

                        projectileComp.m_owner = entity;
                        projectileComp.m_color = entityData.m_color;
                        projectileComp.m_damage = projectileData->damage;

                        projectileEntityData.m_team = entityData.m_team;
                        projectileEntityData.m_radius = projectileData->radius;
                        projectilePos.m_position = position.m_position + offset;
                        projectileVel.m_velocity =
                            direction * -projectileData->speed; // -speed because we want -y as up

                        auto const& sprite = entityData.m_color == EnergyColor::Blue
                                                 ? projectileData->white_sprite
                                                 : projectileData->black_sprite;
                        projectileDrawable.m_spriteData = sprite;
                        projectileLifetime.m_msLeft = 3000;
                    }
                }
            }
        }
    }
}
