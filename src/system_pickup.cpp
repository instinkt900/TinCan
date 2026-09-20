#include "system_pickup.h"
#include "utils_collision.h"
#include "component_body.h"
#include "component_entity.h"
#include "component_passives.h"
#include "gamedata_pickup.h"
#include "gamedata_weapon.h"
#include "system_drawable.h"
#include "system_map.h"
#include "system_movement.h"
#include "system_weapon.h"
#include "tags.h"
#include "utils.h"
#include <moth/ecs/world.h>
#include <moth/graphics/utils/math_utils.h>
#include <magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <cmath>
#include "gamedata.h"
#include "game_world.h"

float const PickupRadius = 30.0f;
// pickups drift with the map. without a map entity they fall at this speed instead
float const PickupFallbackSpeed = 100.0f;

// a swapped out weapon drops somewhere in this ring around the player. the inner edge clears the
// player so the drop is not collected again the moment it appears
float const DropMinDistance = 80.0f;
float const DropMaxDistance = 140.0f;

// a pickup travels at the map's scroll speed, so it stays over the ground it appeared on
float MapScrollSpeed(moth::ecs::World& registry) {
    auto view = registry.View<ComponentMapScroll>();
    for (auto [entity, scroll] : view.each()) {
        return scroll.m_scrollSpeed;
    }
    return PickupFallbackSpeed;
}

template <> struct fmt::formatter<PickupType> : fmt::formatter<std::string> {
    static auto format(PickupType type, format_context& ctx)  -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "{}", magic_enum::enum_name(type));
    }
};

moth::ecs::Entity SystemPickup::CreatePickup(moth::ecs::World& registry, moth::gfx::FloatVec2 const& position,
                                        PickupData const& pickupData, GameData const& gamedata) {
    auto entity = registry.Create();
    registry.Emplace<ComponentEntity>(entity);
    registry.Emplace<ComponentBody>(entity, PickupRadius);
    registry.Emplace<ComponentPosition>(entity, position);
    auto& vel = registry.Emplace<ComponentVelocity>(entity);
    registry.Emplace<ComponentSprite>(entity, *pickupData.sprite);
    auto& pickup = registry.Emplace<ComponentPickup>(entity);

    vel.m_velocity = { 0.0f, MapScrollSpeed(registry) };

    pickup.m_data = pickupData;

    return entity;
}

void PlayerPickupWeapon(PickupData const& pickup, moth::gfx::FloatVec2 const& position,
                        moth::ecs::World& registry, GameData const& gamedata) {
    // spawn the current weapon as a pickup if it has one defined
    moth::ecs::Entity playerEntity = moth::ecs::kNullEntity;
    ComponentWeapon* currentWeapon = nullptr;
    auto view = registry.View<ComponentWeapon, PlayerTag>();
    for (auto [entity, weapon] : view.each()) {
        playerEntity = entity;
        currentWeapon = &weapon;
    }
    if (currentWeapon != nullptr && currentWeapon->m_pickup.valid()) {
        auto const angle = Random(0.0f, moth::gfx::F_PI * 2.0f);
        auto const distance = Random(DropMinDistance, DropMaxDistance);
        moth::gfx::FloatVec2 const offset{ std::cos(angle) * distance, std::sin(angle) * distance };
        SystemPickup::CreatePickup(registry, position + offset, *currentWeapon->m_pickup, gamedata);
    }

    // replace the current weapon
    auto const* weaponData = gamedata.Get<WeaponData>(pickup.name);
    if (weaponData == nullptr) {
        spdlog::error("Unknown weapon {}", pickup.name);
        return;
    }
    SystemWeapon::InitWeapon(registry, playerEntity, *weaponData, gamedata);
}

void PlayerPickupPassive(PickupData const& pickup, moth::gfx::FloatVec2 const& position,
                         moth::ecs::World& registry, GameData const& gamedata) {
    auto const passiveTypeOpt = magic_enum::enum_cast<PassiveType>(pickup.name);
    if (!passiveTypeOpt.has_value()) {
        spdlog::error("PlayerPickupPassive - Unknown passive name {}", pickup.name);
        return;
    }

    ComponentPassives* playerPassives = nullptr;
    auto view = registry.View<ComponentPassives, PlayerTag>();
    for (auto [entity, passives]: view.each()) {
        playerPassives = &passives;
    }

    if (playerPassives == nullptr) {
        spdlog::warn("PlayerPickupPassive - Cannot find player passives component.");
        return;
    }

    auto const passiveType = passiveTypeOpt.value();
    auto const currentBonus = PassiveValue(*playerPassives, passiveType);
    auto const newBonus = PassiveAccumulate(passiveType, currentBonus, pickup.value.value_or(0));
    playerPassives->m_value[passiveType] = newBonus;
}

void PlayerPickup(ComponentPickup const& pickup, moth::gfx::FloatVec2 const& position, moth::ecs::World& registry,
                  GameData const& gamedata) {
    switch (pickup.m_data.type) {
    case PickupType::Weapon:
        PlayerPickupWeapon(pickup.m_data, position, registry, gamedata);
        break;
    case PickupType::Passive:
        PlayerPickupPassive(pickup.m_data, position, registry, gamedata);
        break;
    }
}

void SystemPickup::Update(GameWorld& world, uint32_t ticks) {
    // check for pickup collision

    auto& registry = world.GetRegistry();
    auto const& gamedata = world.GetGameData();

    ComponentPosition const* playerPosition = nullptr;
    ComponentBody const* playerBody = nullptr;
    {
        auto view = registry.View<PlayerTag, ComponentBody, ComponentPosition>();
        for (auto [entity, body, position] : view.each()) {
            playerPosition = &position;
            playerBody = &body;
        }
    }

    if (playerPosition == nullptr) {
        // there is no player for a pickup to reach, which is the normal state between the player dying
        // and the next one. nothing to do, and nothing worth a line of log a frame
        return;
    }

    auto view = registry.View<ComponentPickup, ComponentEntity, ComponentBody, ComponentPosition>();
    for (auto [entity, pickup, pickupEntity, body, position] : view.each()) {
        auto const t =
            SweepTest(playerPosition->m_position, playerPosition->m_lastPosition, playerBody->m_radius,
                      position.m_position, position.m_lastPosition, body.m_radius);

        if (t < 1.0f) {
            // detroy the pickup.
            registry.GetOrEmplace<DeadTag>(entity);

            PlayerPickup(pickup, playerPosition->m_position, registry, gamedata);
        }
    }
}
