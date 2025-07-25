#include "system_enemy_spawner.h"
#include "component_entity.h"
#include "system_behaviour.h"
#include "system_group.h"
#include "component_health.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include "system_drawable.h"
#include "system_weapon.h"
#include "system_world_bounds.h"
#include "tags.h"
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>
#include "game_world.h"
#include "gamedata_enemy.h"

ComponentEnemySpawner::ComponentEnemySpawner(SpawnerData const& data)
    : m_active(true)
    , m_type(data.type)
    , m_offsetStep(data.offset_step)
    , m_count(data.count)
    , m_maxCooldown(data.cooldown)
    , m_maxGroupCount(data.group_count)
    , m_maxGroupCooldown(data.group_delay)
    , m_currentGroupEntity(entt::null)
    , m_behaviour(data.behaviour)
    , m_behaviourParameters(data.behaviour_parameters)
    , m_killType(data.kill_type)
    , m_lifetime(data.lifetime)
    , m_boundsBorder(data.bounds_border) {
    if (data.enemy == nullptr) {
        spdlog::error("Spawner with bad enemy reference");
    } else {
        m_enemy = *data.enemy;
    }
    if (data.drop.has_value()) {
        m_groupDrop = *data.drop;
    }
}

void SystemEnemySpawner::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    auto const& gamedata = world.GetGameData();
    auto view = registry.view<ComponentEnemySpawner, ComponentPosition>();

    for (auto [entity, spawner, spawnerPosition] : view.each()) {
        if (spawner.m_count > 0) {
            if (spawner.m_cooldown > 0) {
                spawner.m_cooldown -= static_cast<int32_t>(ticks);
            }

            if (spawner.m_cooldown <= 0) {
                spawner.m_groupCount = spawner.m_maxGroupCount;
                if (spawner.m_maxGroupCount > 1) {
                    spawner.m_currentGroupEntity = entt::null;
                }
                spawner.m_cooldown += spawner.m_maxCooldown;
                spawner.m_count -= 1;
            }
        }

        if (spawner.m_groupCount > 0) {
            if (spawner.m_groupCooldown > 0) {
                spawner.m_groupCooldown -= static_cast<int32_t>(ticks);
            }

            while (spawner.m_groupCount > 0 && spawner.m_groupCooldown <= 0) {
                spawner.m_groupCount -= 1;
                spawner.m_groupCooldown += spawner.m_maxGroupCooldown;

                if (spawner.m_groupCount <= 0 && spawner.m_count <= 0) {
                    registry.emplace<DeadTag>(entity);
                }

                auto position = spawnerPosition.m_position;
                switch (spawner.m_type) {
                case SpawnerType::Staggered: {
                    auto const side = spawner.m_groupCount % 2;
                    position += spawner.m_offsetStep * (side == 0 ? 1.0f : -1.0f);
                } break;
                case SpawnerType::Stepped: {
                    position += spawner.m_offsetStep *
                                static_cast<float>(spawner.m_maxGroupCount - (spawner.m_groupCount + 1));
                } break;
                default:
                    break;
                }

                auto enemy = CreateEnemy(registry, spawner.m_enemy, gamedata, position, spawner.m_behaviour,
                                         spawner.m_behaviourParameters);

                if (spawner.m_killType == EnemyKillType::Time) {
                    auto& lifetime = registry.emplace<ComponentLifetime>(enemy);
                    lifetime.m_msLeft = spawner.m_lifetime.value();
                } else if (spawner.m_killType == EnemyKillType::Bounds) {
                    auto& bounds = registry.emplace<ComponentBounds>(enemy);
                    canyon::FloatRect boundsRect{ { 0.0f, 0.0f },
                                                  static_cast<canyon::FloatVec2>(world.GetWorldSize()) };
                    if (spawner.m_boundsBorder.has_value()) {
                        boundsRect.topLeft -=
                            { spawner.m_boundsBorder.value(), spawner.m_boundsBorder.value() };
                        boundsRect.bottomRight +=
                            { spawner.m_boundsBorder.value(), spawner.m_boundsBorder.value() };
                    }
                    bounds.m_bounds = boundsRect;
                    bounds.m_behaviour = BoundsBehaviour::Kill;
                }

                if (spawner.m_maxGroupCount > 1) {
                    if (spawner.m_currentGroupEntity == entt::null) {
                        spawner.m_currentGroupEntity =
                            SystemGroup::CreateGroup(registry, spawner.m_groupDrop);
                    }
                    SystemGroup::AddMember(registry, spawner.m_currentGroupEntity, enemy);
                }
            }
        }
    }
}

entt::entity SystemEnemySpawner::CreateEnemy(entt::registry& registry, EnemyData const& enemyData,
                                             GameData const& gamedata, canyon::FloatVec2 const& position,
                                             EnemyBehaviour behaviourType,
                                             BehaviourParameterList const& behaviourParameters) {
    auto enemy = registry.create();
    auto& entityData = registry.emplace<ComponentEntity>(enemy);
    auto& health = registry.emplace<ComponentHealth>(enemy);
    auto& pos = registry.emplace<ComponentPosition>(enemy);
    auto& behaviour = registry.emplace<ComponentBehaviour>(enemy);
    registry.emplace<TargetTag>(enemy);

    entityData.m_team = Team::ENEMY;
    entityData.m_color = enemyData.color;
    entityData.m_radius = enemyData.radius;
    entityData.m_angle = M_PI; // TODO: read from data? spawner? behaviour?

    health.m_currentHealth = enemyData.health;
    health.m_maxHealth = enemyData.health;

    SpriteData const* spriteData = enemyData.sprite;
    if (spriteData == nullptr) {
        spdlog::error("Unknown sprite for enemy.");
        return entt::null;
    }
    registry.emplace<ComponentDrawable>(enemy, *spriteData);

    pos.m_position = position;

    auto* weapon = SystemWeapon::InitWeapon(registry, enemy, *enemyData.weapon, gamedata);
    if (weapon != nullptr) {
        weapon->m_active = true;
    }

    behaviour.m_behaviour = behaviourType;
    behaviour.m_parameters = behaviourParameters;
    behaviour.m_offset = pos.m_position;

    return enemy;
}

entt::entity SystemEnemySpawner::CreateSpawner(entt::registry& registry, SpawnerData const& data,
                                               GameData const& gamedata, canyon::FloatVec2 const& position) {
    auto enemySpawner = registry.create();
    registry.emplace<ComponentEnemySpawner>(enemySpawner, data);
    auto& spawnerPos = registry.emplace<ComponentPosition>(enemySpawner);
    spawnerPos.m_position = position;
    return enemySpawner;
}
