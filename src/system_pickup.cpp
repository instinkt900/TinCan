#include "system_pickup.h"
#include "collision_utils.h"
#include "game_layer.h"
#include "system_drawable.h"
#include "system_weapon.h"
#include "tags.h"
#include <entt/entt.hpp>
#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

float const PickupRadius = 30.0f;
moth_ui::FloatVec2 const PickupVelocity{ 0.0f, 100.0f };

template<>
struct fmt::formatter<PickupType> : fmt::formatter<std::string>
{
    auto format(PickupType type, format_context &ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "{}", magic_enum::enum_name(type));
    }
};

std::string GetSpriteForPickupType(PickupType type) {
    switch (type) {
    default:
    case PickupType::Weapon:
        return "drop_01";
    }
}

entt::entity SystemPickup::CreatePickup(entt::registry& registry, moth_ui::FloatVec2 const& position,
                                        PickupType type, std::string const& name, Gamedata const& gamedata) {
    auto const* spriteDatabase = gamedata.GetSpriteDatabase();
    if (spriteDatabase == nullptr) {
        spdlog::error("SystemPickup::CreatePickup - Unable to read sprite database");
        return entt::null;
    }

    auto const* spriteData = spriteDatabase->Get(GetSpriteForPickupType(type));
    if (spriteDatabase == nullptr) {
        spdlog::error("SystemPickup::CreatePickup - Unable to get sprite for pickup type {}", type);
        return entt::null;
    }

    auto entity = registry.create();
    auto& entityDetails = registry.emplace<ComponentEntity>(entity);
    auto& pos = registry.emplace<ComponentPosition>(entity);
    auto& vel = registry.emplace<ComponentVelocity>(entity);
    auto& sprite = registry.emplace<ComponentDrawable>(entity);
    auto& pickup = registry.emplace<ComponentPickup>(entity);

    entityDetails.m_team = Team::NONE;
    entityDetails.m_color = EnergyColor::WHITE;
    entityDetails.m_radius = PickupRadius;

    pos.m_lastPosition = position;
    pos.m_position = position;

    vel.m_velocity = PickupVelocity;

    sprite.m_spriteData = *spriteData;

    pickup.m_type = type;
    pickup.m_name = name;

    return entity;
}

void PlayerPickupWeapon(ComponentPickup const& pickup, moth_ui::FloatVec2 const& position, entt::registry& registry, Gamedata const& gamedata) {
    auto const* weaponDatabase = gamedata.GetWeaponDatabase();
    if (weaponDatabase == nullptr) {
        spdlog::error("PlayerPickupWeapon - Unable to read weapon database.");
        return;
    }

    auto const* weaponData = weaponDatabase->Get(pickup.m_name);
    if (weaponData == nullptr) {
        spdlog::error("PlayerPickupWeapon - Unable to find weapon data for {}", pickup.m_name);
        return;
    }

    // spawn the current weapon as a pickup
    entt::entity playerEntity = entt::null;
    ComponentWeapon* currentWeapon = nullptr;
    auto view = registry.view<ComponentWeapon, PlayerTag>();
    for (auto [entity, weapon]: view.each()) {
        playerEntity = entity;
        currentWeapon = &weapon;
    }
    if (currentWeapon != nullptr) {
        auto const side = rand() % 2 == 1 ? 1.0f : -1.0f;
        moth_ui::FloatVec2 const offset = { 100.0f * side, -20.0f };
        SystemPickup::CreatePickup(registry, position + offset, pickup.m_type, currentWeapon->m_name, gamedata);
    }

    // replace the current weapon
    SystemWeapon::InitWeapon(registry, playerEntity, pickup.m_name, gamedata);
}

void PlayerPickup(ComponentPickup const& pickup, moth_ui::FloatVec2 const& position, entt::registry& registry, Gamedata const& gamedata) {
    switch (pickup.m_type) {
    case PickupType::Weapon:
        PlayerPickupWeapon(pickup, position, registry, gamedata);
        break;
    }
}

void SystemPickup::Update(entt::registry& registry, uint32_t ticks, Gamedata const& gamedata) {
    // check for pickup collision

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
