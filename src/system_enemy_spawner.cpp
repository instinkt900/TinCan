#include "system_enemy_spawner.h"
#include "component_entity.h"
#include "system_behaviour.h"
#include "system_group.h"
#include "component_health.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include "system_drawable.h"
#include "system_weapon.h"
#include "tags.h"
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>
#include "game_world.h"

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

            if (spawner.m_groupCooldown <= 0) {
                spawner.m_groupCount -= 1;
                spawner.m_groupCooldown += spawner.m_maxGroupCooldown;

                if (spawner.m_groupCount <= 0 && spawner.m_count <= 0) {
                    registry.emplace<DeadTag>(entity);
                }

                auto position = spawnerPosition.m_position;
                switch (spawner.m_type) {
                case SpawnerType::Staggered: {
                    auto const side = spawner.m_groupCount % 2;
                    position.x += spawner.m_distance * (side == 0 ? 1.0f : -1.0f);
                } break;
                case SpawnerType::Stepped: {
                    position.x += spawner.m_distance *
                                  static_cast<float>(spawner.m_maxGroupCount - (spawner.m_groupCount + 1));
                } break;
                default:
                    break;
                }

                auto enemy = CreateEnemy(registry, spawner.m_enemyName, gamedata, position,
                                         spawner.m_behaviourName, spawner.m_behaviourParameters);

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

entt::entity SystemEnemySpawner::CreateEnemy(entt::registry& registry, std::string const& name,
                                             GameData const& gamedata, canyon::FloatVec2 const& position,
                                             std::string const& behaviourName,
                                             BehaviourParameterList const& behaviourParameters) {
    auto enemy = registry.create();
    auto& entityData = registry.emplace<ComponentEntity>(enemy);
    auto& health = registry.emplace<ComponentHealth>(enemy);
    auto& drawable = registry.emplace<ComponentDrawable>(enemy);
    auto& pos = registry.emplace<ComponentPosition>(enemy);
    auto& lifetime = registry.emplace<ComponentLifetime>(enemy);
    auto& behaviour = registry.emplace<ComponentBehaviour>(enemy);
    registry.emplace<TargetTag>(enemy);

    auto const* enemyData = gamedata.GetEnemyDatabase().Get(name);

    entityData.m_team = Team::ENEMY;
    entityData.m_color = enemyData->color;
    entityData.m_radius = enemyData->radius;
    entityData.m_angle = M_PI; // TODO: read from data? spawner? behaviour?

    health.m_currentHealth = enemyData->health;
    health.m_maxHealth = enemyData->health;

    drawable.m_spriteData = enemyData->sprite;

    pos.m_position = position;

    auto* weapon = SystemWeapon::InitWeapon(registry, enemy, enemyData->weapon_name, gamedata);
    if (weapon != nullptr) {
        weapon->m_active = true;
    }

    lifetime.m_msAlive = 0;
    lifetime.m_msLeft = enemyData->lifetime;

    behaviour.m_behaviourName = behaviourName;
    behaviour.m_parameters = behaviourParameters;
    behaviour.m_offset = pos.m_position;

    return enemy;
}

entt::entity SystemEnemySpawner::CreateSpawner(entt::registry& registry, SpawnerData const& data,
                                               GameData const& gamedata, canyon::FloatVec2 const& position) {
    auto enemySpawner = registry.create();

    auto& spawner = registry.emplace<ComponentEnemySpawner>(enemySpawner);
    spawner.m_active = true;
    spawner.m_type = data.type;
    spawner.m_distance = data.distance;
    spawner.m_count = data.count;
    spawner.m_maxCooldown = data.cooldown;
    spawner.m_maxGroupCooldown = data.group_delay;
    spawner.m_maxGroupCount = data.group_count;
    spawner.m_enemyName = data.enemy_name;
    spawner.m_behaviourName = data.behaviour_name;
    spawner.m_behaviourParameters = data.behaviour_parameters;
    spawner.m_currentGroupEntity = entt::null;
    spawner.m_groupDrop = data.drop;

    auto& spawnerPos = registry.emplace<ComponentPosition>(enemySpawner);
    spawnerPos.m_position = position;

    return enemySpawner;
}
