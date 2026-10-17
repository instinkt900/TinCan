#include "system_weapon.h"
#include "component_entity.h"
#include "gamedata_weapon.h"
#include "system_drawable.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include "system_projectile.h"
#include "system_world_bounds.h"
#include "tags.h"
#include <algorithm>
#include <moth/ecs/world.h>
#include <moth/graphics/utils/math_utils.h>
#include <moth/ui/utils/vector_utils.h>
#include <spdlog/spdlog.h>
#include "game_world.h"
#include "utils_entity.h"

namespace {
    int const ProjectileBorder = 200;
}

Weapon::Weapon(std::string name, WeaponData const& weapon)
    : m_name(std::move(name))
    , m_damage(weapon.damage)
    , m_cooldown(weapon.cooldown)
    , m_maxCooldown(weapon.cooldown)
    , m_burst(weapon.burst)
    , m_maxBurst(weapon.burst)
    , m_burstCooldown(weapon.burst_delay)
    , m_maxBurstCooldown(weapon.burst_delay)
    , m_playerTracking(weapon.player_tracking)
    , m_fireState(weapon.fire_state)
    , m_projectile(*weapon.projectile)
    , m_muzzleSpeed(weapon.muzzle_speed) {
    m_barrelGroups.clear();
    m_barrelGroupIds.clear();
    for (auto const& barrel : weapon.barrels) {
        // barrel angles are written in degrees, and entity angles are held in radians
        m_barrelGroups[barrel.group].push_back({ barrel.offset, moth::gfx::DegToRad(barrel.angle) });
    }
    for (auto const& [id, group] : m_barrelGroups) {
        m_barrelGroupIds.push_back(id);
    }
}

Weapon* SystemWeapon::InitWeapon(moth::ecs::World& registry, moth::ecs::Entity entity,
                                 std::string const& name, WeaponData const& weaponData,
                                 GameData const& gamedata, bool active) {
    // Weapon dereferences the projectile, so an unresolved one leaves
    // the entity weaponless rather than taking the game down.
    if (!weaponData.projectile.valid()) {
        spdlog::error("Weapon has no valid projectile; skipping it");
        return nullptr;
    }
    auto& weapons = registry.GetOrEmplace<ComponentWeapon>(entity);
    weapons.m_weapons.clear();
    auto& weapon = weapons.m_weapons.emplace_back(name, weaponData);
    weapon.m_active = active;
    return &weapon;
}

Weapon* SystemWeapon::AddWeapon(moth::ecs::World& registry, moth::ecs::Entity entity,
                                std::string const& name, WeaponData const& weaponData,
                                GameData const& gamedata, bool active) {
    if (!weaponData.projectile.valid()) {
        spdlog::error("Weapon has no valid projectile; skipping it");
        return nullptr;
    }
    auto& weapons = registry.GetOrEmplace<ComponentWeapon>(entity);
    // carrying the same weapon twice would just double its fire rate, so the duplicate is dropped
    for (auto& existing : weapons.m_weapons) {
        if (existing.m_name == name) {
            return &existing;
        }
    }
    // a fresh weapon starts its timers from now, which puts it out of step with the weapons already
    // carried. they would then fire on different frames, and a moving player spawns each group from
    // a different position, so the shots stagger instead of staying evenly spaced. taking over the
    // first weapon's timers puts the new one on the same beat, and from there the two step through
    // the same states together as long as they share the same cooldowns. both timers are copied:
    // the burst one carries a remainder between shots, and leaving that behind is enough for the
    // two to fire a frame apart
    bool const hasBeat = !weapons.m_weapons.empty();
    auto const cooldownBeat = hasBeat ? weapons.m_weapons.front().m_cooldown : 0;
    auto const burst = hasBeat ? weapons.m_weapons.front().m_burst : 0;
    auto const burstBeat = hasBeat ? weapons.m_weapons.front().m_burstCooldown : 0;

    auto& weapon = weapons.m_weapons.emplace_back(name, weaponData);
    if (hasBeat) {
        weapon.m_cooldown = std::min(cooldownBeat, weapon.m_maxCooldown);
        weapon.m_burst = std::min(burst, weapon.m_maxBurst);
        // not clamped to this weapon's burst delay: the running remainder comes from the floor the
        // update applies, which can be longer than the weapon's own delay
        weapon.m_burstCooldown = burstBeat;
    }
    weapon.m_active = active;
    return &weapon;
}

int32_t SystemWeapon::AddWeaponLevels(moth::ecs::World& registry, moth::ecs::Entity entity,
                                      std::string const& progressionName, int32_t levels,
                                      GameData const& gamedata) {
    if (levels <= 0) {
        return 0;
    }
    auto const* progression = gamedata.Get<WeaponProgressionData>(progressionName);
    if (progression == nullptr) {
        return 0;
    }
    auto const lastLevel = static_cast<int32_t>(progression->levels.size()) - 1;

    auto& weapons = registry.GetOrEmplace<ComponentWeapon>(entity);
    auto carried = std::find_if(weapons.m_weapons.begin(), weapons.m_weapons.end(),
                                [&](Weapon const& weapon) { return weapon.m_progression == progressionName; });
    if (carried == weapons.m_weapons.end()) {
        auto const& firstName = progression->levels.front();
        auto const* firstData = gamedata.Get<WeaponData>(firstName);
        if (firstData == nullptr) {
            return 0;
        }
        if (AddWeapon(registry, entity, firstName, *firstData, gamedata) == nullptr) {
            return 0;
        }
        // AddWeapon may have grown the vector, so the weapon is found again rather than kept by iterator
        carried = std::find_if(weapons.m_weapons.begin(), weapons.m_weapons.end(),
                               [&](Weapon const& weapon) { return weapon.m_name == firstName; });
        carried->m_progression = progressionName;
        carried->m_level = 0;
        levels -= 1;
    }

    auto const targetLevel = carried->m_level + levels;
    auto const newLevel = std::min(targetLevel, lastLevel);
    auto const overflow = targetLevel - newLevel;
    if (newLevel == carried->m_level) {
        return overflow;
    }

    auto const& levelName = progression->levels[static_cast<size_t>(newLevel)];
    auto const* levelData = gamedata.Get<WeaponData>(levelName);
    if (levelData == nullptr || !levelData->projectile.valid()) {
        spdlog::error("Weapon level {} of {} cannot be built; keeping the current level", levelName,
                      progressionName);
        return overflow;
    }

    // the new level takes over the old one's timers, so it keeps firing on the same beat as the other weapons
    Weapon raised(levelName, *levelData);
    raised.m_progression = progressionName;
    raised.m_level = newLevel;
    raised.m_active = carried->m_active;
    raised.m_cooldown = std::min(carried->m_cooldown, raised.m_maxCooldown);
    raised.m_burst = std::min(carried->m_burst, raised.m_maxBurst);
    raised.m_burstCooldown = carried->m_burstCooldown;
    *carried = std::move(raised);
    return overflow;
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
    for (auto [entity, weapons, position] : view.each()) {

        auto const* entityDetails = GetEntityDetails(registry, entity);
        if (entityDetails == nullptr) {
            continue;
        }

        for (auto& weapon : weapons.m_weapons) {
            auto const weaponCooldown = weapon.m_maxCooldown;
            auto const burstCooldown = std::max(50, weapon.m_maxBurstCooldown);
            auto const burstCount = weapon.m_maxBurst;
            auto const damage = weapon.m_damage;

            if (weapon.m_cooldown > 0) {
                weapon.m_cooldown -= static_cast<int32_t>(ticks);
            }

            if (weapon.m_cooldown <= 0 && weapon.m_active) {
                weapon.m_burst = burstCount;
                weapon.m_cooldown += weaponCooldown;
            }

            if (weapon.m_burst > 0) {
                if (weapon.m_burstCooldown > 0) {
                    weapon.m_burstCooldown -= static_cast<int32_t>(ticks);
                }

                while (weapon.m_burstCooldown <= 0 && weapon.m_burst > 0 &&
                       (weapon.m_active || weapon.m_burst < burstCount)) {
                    weapon.m_burst -= 1;
                    weapon.m_burstCooldown += burstCooldown;

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
                                weapon.m_muzzleSpeed, barrelAngle, damage);
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
}
