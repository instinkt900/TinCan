#include "system_pickup.h"
#include "game_layer.h"
#include "system_drawable.h"
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>

float const PickupRadius = 10.0f;
moth_ui::FloatVec2 const PickupVelocity{ 0.0f, 100.0f };

entt::entity SystemPickup::CreatePickup(entt::registry& registry, moth_ui::FloatVec2 const& position,
                          std::string const& name, Gamedata const& gamedata) {
    auto const* dropDatabase = gamedata.GetDropDatabase();
    auto const* spriteDatabase = gamedata.GetSpriteDatabase();
    if (dropDatabase == nullptr) {
        spdlog::error("Unable to read drop database");
        return entt::null;
    }
    if (spriteDatabase == nullptr) {
        spdlog::error("Unable to read sprite database");
        return entt::null;
    }
    auto const* dropData = dropDatabase->Get(name);
    if (dropData == nullptr) {
        return entt::null;
    }
    auto const* spriteData = spriteDatabase->Get(dropData->sprite);
    if (spriteData == nullptr) {
        return entt::null;
    }
    auto entity = registry.create();
    auto& entityDetails = registry.emplace<ComponentEntity>(entity);
    auto& pos = registry.emplace<ComponentPosition>(entity);
    auto& vel = registry.emplace<ComponentVelocity>(entity);
    auto& sprite = registry.emplace<ComponentDrawable>(entity);

    entityDetails.m_team = Team::NONE;
    entityDetails.m_color = EnergyColor::WHITE;
    entityDetails.m_radius = PickupRadius;

    pos.m_lastPosition = position;
    pos.m_position = position;

    vel.m_velocity = PickupVelocity;

    sprite.m_spriteData = *spriteData;

    return entity;
}

void SystemPickup::Update(entt::registry& registry, uint32_t ticks) {
    // check for pickup collision
}
