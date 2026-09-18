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
#include <moth/ecs/world.h>
#include <moth/ui/utils/vector_utils.h>
#include <spdlog/spdlog.h>
#include "game_world.h"
#include "utils_entity.h"

namespace {
    int const ProjectileBorder = 200;
}

ComponentWeapon::ComponentWeapon(WeaponData const& weapon)
    : m_pickup(weapon.pickup)
    , m_damage(weapon.damage)
    , m_cooldown(weapon.cooldown)
    , m_maxCooldown(weapon.cooldown)
    , m_burst(weapon.burst)
    , m_maxBurst(weapon.burst)
    , m_burstCooldown(weapon.burst_delay)
    , m_maxBurstCooldown(weapon.burst_delay)
    , m_playerTracking(weapon.player_tracking)
    , m_fireState(weapon.fire_state)
    , m_projectile(*weapon.projectile) {
    m_barrelGroups.clear();
    m_barrelGroupIds.clear();
    for (auto const& barrel : weapon.barrels) {
        m_barrelGroups[barrel.group].push_back({ barrel.offset, barrel.angle });
    }
    for (auto const& [id, group] : m_barrelGroups) {
        m_barrelGroupIds.push_back(id);
    }
}

ComponentWeapon* SystemWeapon::InitWeapon(moth::ecs::World& registry, moth::ecs::Entity entity,
                                          WeaponData const& weaponData, GameData const& gamedata,
                                          bool active) {
    // ComponentWeapon dereferences the projectile, so an unresolved one leaves
    // the entity weaponless rather than taking the game down.
    if (!weaponData.projectile.valid()) {
        spdlog::error("Weapon has no valid projectile; skipping it");
        return nullptr;
    }
    // GetOrEmplace leaves an existing component as it is and drops the arguments, so swapping an
    // entity onto a different weapon has to overwrite what is already there
    auto& weapon = registry.GetOrEmplace<ComponentWeapon>(entity, weaponData);
    weapon = ComponentWeapon(weaponData);
    weapon.m_active = active;
    return &weapon;
}

struct Passives {
    float cooldownMult = 1.0f;
    float burstCooldownMult = 1.0f;
    float damageMult = 1.0f;
    float burstAdd = 0.0f;
};

void CollectPassives(moth::ecs::World& registry, moth::ecs::Entity entity, Passives& passives) {
    if (auto const* component = registry.TryGet<ComponentPassives>(entity)) {
        passives.cooldownMult = PassiveValue(*component, PassiveType::WeaponCooldown);
        passives.burstCooldownMult = PassiveValue(*component, PassiveType::WeaponCooldownBurst);
        passives.damageMult = PassiveValue(*component, PassiveType::WeaponDamage);
        passives.burstAdd = PassiveValue(*component, PassiveType::WeaponBurst);
    }
}

void SystemWeapon::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    ComponentPosition* playerPosition = nullptr;
    auto playerView = registry.View<ComponentPosition, PlayerTag>();
    if (playerView.begin() != playerView.end()) {
        auto playerEntity = *playerView.begin();
        playerPosition = registry.TryGet<ComponentPosition>(playerEntity);
    }

    auto view = registry.View<ComponentWeapon, ComponentPosition>();
    for (auto [entity, weapon, position] : view.each()) {

        auto const* entityDetails = GetEntityDetails(registry, entity);
        if (entityDetails == nullptr) {
            continue;
        }

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

                if (!weapon.m_fireState.empty()) {
                    if (auto* sprite = registry.TryGet<ComponentSprite>(entity)) {
                        SetSpriteState(*sprite, weapon.m_fireState);
                    }
                }

                auto const barrelGroupId = weapon.m_barrelGroupIds[weapon.m_barrelGroupIndex];
                auto const& barrelGroup = weapon.m_barrelGroups[barrelGroupId];
                weapon.m_barrelGroupIndex =
                    (weapon.m_barrelGroupIndex + 1) % static_cast<int32_t>(weapon.m_barrelGroupIds.size());
                for (auto const& barrel : barrelGroup) {
                    auto const barrelAngle = entityDetails->m_angle + barrel.m_angle;
                    auto direction = moth::ui::Rotate2D({ 0, 1.0f }, barrelAngle);
                    auto const offset =
                        moth::ui::Rotate2D({ barrel.m_offset.x, -barrel.m_offset.y }, entityDetails->m_angle);

                    if (weapon.m_playerTracking && (playerPosition != nullptr)) {
                        auto delta = position.m_position - playerPosition->m_position;
                        direction = moth::ui::Normalized(delta);
                    }

                    auto const* projectileData = &weapon.m_projectile;
                    if (projectileData != nullptr) {
                        auto const projectileEntity = SystemProjectile::CreateProjectile(
                            registry, *projectileData, entity, position.m_position + offset, direction,
                            barrelAngle, damage);
                        auto& projectileBounds = registry.Emplace<ComponentBounds>(projectileEntity);
                        projectileBounds.m_bounds = static_cast<moth::gfx::FloatRect>(moth::gfx::MakeRect(
                            -ProjectileBorder, -ProjectileBorder, world.GetWorldSize().x + ProjectileBorder,
                            world.GetWorldSize().y + ProjectileBorder));
                        projectileBounds.m_behaviour = BoundsBehaviour::Kill;
                    }
                }
            }
        }
    }
}
