#include "system_pickup.h"
#include "collision_utils.h"
#include "component_entity.h"
#include "component_passives.h"
#include "gamedata_pickup.h"
#include "system_drawable.h"
#include "system_movement.h"
#include "system_weapon.h"
#include "tags.h"
#include <entt/entt.hpp>
#include <magic_enum.hpp>
#include <spdlog/spdlog.h>
#include "gamedata.h"
#include "game_world.h"

float const PickupRadius = 30.0f;
canyon::FloatVec2 const PickupVelocity{ 0.0f, 100.0f };

template <> struct fmt::formatter<PickupType> : fmt::formatter<std::string> {
    static auto format(PickupType type, format_context& ctx)  -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "{}", magic_enum::enum_name(type));
    }
};

entt::entity SystemPickup::CreatePickup(entt::registry& registry, canyon::FloatVec2 const& position,
                                        PickupData const& pickupData, GameData const& gamedata) {
    auto entity = registry.create();
    auto& entityDetails = registry.emplace<ComponentEntity>(entity);
    auto& pos = registry.emplace<ComponentPosition>(entity);
    auto& vel = registry.emplace<ComponentVelocity>(entity);
    auto& sprite = registry.emplace<ComponentDrawable>(entity);
    auto& pickup = registry.emplace<ComponentPickup>(entity);

    entityDetails.m_team = Team::NONE;
    entityDetails.m_color = EnergyColor::Blue;
    entityDetails.m_radius = PickupRadius;

    pos.m_lastPosition = position;
    pos.m_position = position;

    vel.m_velocity = PickupVelocity;

    sprite.m_spriteData = *pickupData.sprite;

    pickup.m_name = pickupData.name;

    return entity;
}

void PlayerPickupWeapon(PickupData const& pickup, canyon::FloatVec2 const& position,
                        entt::registry& registry, GameData const& gamedata) {
    // spawn the current weapon as a pickup if it has one defined
    entt::entity playerEntity = entt::null;
    ComponentWeapon* currentWeapon = nullptr;
    auto view = registry.view<ComponentWeapon, PlayerTag>();
    for (auto [entity, weapon] : view.each()) {
        playerEntity = entity;
        currentWeapon = &weapon;
    }
    if (currentWeapon != nullptr && currentWeapon->m_pickup.has_value()) {
        auto const side = rand() % 2 == 1 ? 1.0f : -1.0f;
        canyon::FloatVec2 const offset = { 100.0f * side, -20.0f };
        SystemPickup::CreatePickup(registry, position + offset, currentWeapon->m_pickup.value(), gamedata);
    }

    // replace the current weapon
    auto const* weaponData = gamedata.Get<WeaponData>(pickup.name);
    if (weaponData == nullptr) {
        spdlog::error("Unknown weapon {}", pickup.name);
        return;
    }
    SystemWeapon::InitWeapon(registry, playerEntity, *weaponData, gamedata);
}

void PlayerPickupPassive(PickupData const& pickup, canyon::FloatVec2 const& position,
                         entt::registry& registry, GameData const& gamedata) {
    auto const passiveTypeOpt = magic_enum::enum_cast<PassiveType>(pickup.name);
    if (!passiveTypeOpt.has_value()) {
        spdlog::error("PlayerPickupPassive - Unknown passive name {}", pickup.name);
        return;
    }

    ComponentPassives* playerPassives = nullptr;
    auto view = registry.view<ComponentPassives, PlayerTag>();
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

void PlayerPickup(ComponentPickup const& pickup, canyon::FloatVec2 const& position, entt::registry& registry,
                  GameData const& gamedata) {
    auto const* pickupData = gamedata.Get<PickupData>(pickup.m_name);
    if (pickupData == nullptr) {
        spdlog::error("PlayerPickup - Unable to find pickup name {}.", pickup.m_name);
        return;
    }

    switch (pickupData->type) {
    case PickupType::Weapon:
        PlayerPickupWeapon(*pickupData, position, registry, gamedata);
        break;
    case PickupType::Passive:
        PlayerPickupPassive(*pickupData, position, registry, gamedata);
        break;
    }
}

void SystemPickup::Update(GameWorld& world, uint32_t ticks) {
    // check for pickup collision

    auto& registry = world.GetRegistry();
    auto const& gamedata = world.GetGameData();

    ComponentEntity const* playerEntity = nullptr;
    ComponentPosition const* playerPosition = nullptr;
    {
        auto view = registry.view<PlayerTag, ComponentEntity, ComponentPosition>();
        for (auto [entity, entityDetails, position] : view.each()) {
            playerEntity = &entityDetails;
            playerPosition = &position;
        }
    }

    if (playerPosition == nullptr) {
        spdlog::error("Unable to locate player");
    }

    auto view = registry.view<ComponentPickup, ComponentEntity, ComponentPosition>();
    for (auto [entity, pickup, pickupEntity, position] : view.each()) {
        auto const t =
            SweepTest(playerPosition->m_position, playerPosition->m_lastPosition, playerEntity->m_radius,
                      position.m_position, position.m_lastPosition, pickupEntity.m_radius);

        if (t < 1.0f) {
            // detroy the pickup.
            registry.get_or_emplace<DeadTag>(entity);

            PlayerPickup(pickup, playerPosition->m_position, registry, gamedata);
        }
    }
}
