#include "system_pickup.h"
#include "utils_collision.h"
#include "component_body.h"
#include "component_entity.h"
#include "gamedata_pickup.h"
#include "gamedata_player.h"
#include "gamedata_weapon.h"
#include "system_drawable.h"
#include "system_map.h"
#include "system_movement.h"
#include "system_lifetime.h"
#include "system_shield.h"
#include "system_sound.h"
#include "system_weapon.h"
#include "system_world_bounds.h"
#include "tags.h"
#include "utils.h"
#include <moth/ecs/world.h>
#include <moth/graphics/utils/math_utils.h>
#include <magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <cmath>
#include <limits>
#include <utility>
#include <vector>
#include "gamedata.h"
#include "game_world.h"

float const PickupRadius = 30.0f;
// how far from where the ship died its dropped weapons appear
float const DropSpreadRadius = 48.0f;
// pickups drift with the map. without a map entity they fall at this speed instead
float const PickupFallbackSpeed = 100.0f;

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

void PlayerPickupWeapon(PickupData const& pickup, int32_t levels, moth::gfx::FloatVec2 const& position,
                        moth::ecs::World& registry, GameData const& gamedata) {
    moth::ecs::Entity playerEntity = moth::ecs::kNullEntity;
    auto view = registry.View<PlayerTag>();
    for (auto entity : view) {
        playerEntity = entity;
    }
    if (playerEntity == moth::ecs::kNullEntity) {
        return;
    }

    // the weapon joins the ones the player already carries, or goes up a level. a weapon with no level left
    // to give turns the pickup into power instead
    auto const overflow = SystemWeapon::AddWeaponLevels(registry, playerEntity, pickup.name, levels, gamedata);
    if (overflow > 0) {
        auto const* playerData = gamedata.GetPlayer();
        auto* power = registry.TryGet<ComponentPower>(playerEntity);
        if (playerData != nullptr && power != nullptr) {
            power->m_power += playerData->max_level_power * static_cast<float>(overflow);
        }
    }
}

void PlayerPickup(ComponentPickup const& pickup, moth::gfx::FloatVec2 const& position, moth::ecs::World& registry,
                  GameData const& gamedata) {
    switch (pickup.m_data.type) {
    case PickupType::Weapon:
        PlayerPickupWeapon(pickup.m_data, pickup.m_levels, position, registry, gamedata);
        break;
    }
}

void SystemPickup::DropWeapons(moth::ecs::World& registry, moth::ecs::Entity player, GameData const& gamedata) {
    auto const* playerData = gamedata.GetPlayer();
    auto const* weapons = registry.TryGet<ComponentWeapon>(player);
    auto const* position = registry.TryGet<ComponentPosition>(player);
    if (playerData == nullptr || weapons == nullptr || position == nullptr) {
        return;
    }

    // each weapon to drop, with the levels it had. copied out first, because making pickups adds to the pools
    std::vector<std::pair<PickupData const*, int32_t>> drops;
    auto const pickupNames = gamedata.Get<PickupData>()->Keys();
    for (auto const& weapon : weapons->m_weapons) {
        if (weapon.m_progression.empty() || weapon.m_progression == playerData->weapon) {
            continue;
        }
        PickupData const* pickupData = nullptr;
        for (auto const& name : pickupNames) {
            auto const* candidate = gamedata.Get<PickupData>(name);
            if (candidate != nullptr && candidate->type == PickupType::Weapon && candidate->name == weapon.m_progression) {
                pickupData = candidate;
                break;
            }
        }
        if (pickupData == nullptr) {
            spdlog::error("No pickup gives the weapon {}, so it is not dropped", weapon.m_progression);
            continue;
        }
        drops.emplace_back(pickupData, weapon.m_level + 1);
    }
    if (drops.empty()) {
        return;
    }

    // the ship's bounds are the screen, and a drop that falls past the bottom of it is gone
    auto bottom = std::numeric_limits<float>::max();
    if (auto const* bounds = registry.TryGet<ComponentBounds>(player)) {
        bottom = bounds->m_bounds.bottom() + PickupRadius;
    }

    auto const origin = position->m_position;
    auto const angleStep = moth::gfx::DegToRad(360.0f) / static_cast<float>(drops.size());
    for (size_t i = 0; i < drops.size(); ++i) {
        auto const& [pickupData, levels] = drops[i];
        // spread evenly round where the ship died, starting straight up
        auto const offset = moth::gfx::Rotate2D({ 0.0f, -DropSpreadRadius }, angleStep * static_cast<float>(i));
        auto const entity = CreatePickup(registry, origin + offset, *pickupData, gamedata);
        registry.Get<ComponentPickup>(entity).m_levels = levels;
        registry.Emplace<ComponentLifetime>(entity, 0u, playerData->drop_lifetime);
        auto& bounds = registry.Emplace<ComponentBounds>(entity);
        bounds.m_behaviour = BoundsBehaviour::Kill;
        auto constexpr Far = 100000.0f;
        bounds.m_bounds = moth::gfx::MakeRect(-Far, -Far, Far * 2.0f, bottom + Far);
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
            SystemSound::Play(registry, "pickup");

            PlayerPickup(pickup, playerPosition->m_position, registry, gamedata);
        }
    }
}
