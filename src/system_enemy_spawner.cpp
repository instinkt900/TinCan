#include "system_enemy_spawner.h"
#include "component_body.h"
#include "component_death_sprite.h"
#include "component_entity.h"
#include "component_health.h"
#include "system_behaviour.h"
#include "system_group.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include "system_drawable.h"
#include "system_weapon.h"
#include "system_world_bounds.h"
#include "tags.h"
#include <moth/ecs/world.h>
#include <spdlog/spdlog.h>
#include "game_world.h"

moth::ecs::Entity SystemEnemySpawner::SpawnEnemy(moth::ecs::World& registry, EnemyData const& data,
                                            BehaviourData const& behaviour, moth::gfx::FloatVec2 const& position,
                                            GameData const& gamedata) {
    auto rootEntity = registry.Create();
    auto& rootDetails = registry.Emplace<ComponentEntity>(rootEntity, Team::Enemy, data.affinity);
    registry.Emplace<ComponentPosition>(rootEntity, position);
    registry.Emplace<ComponentVelocity>(rootEntity);

    if (data.radius > 0.0f) {
        registry.Emplace<ComponentBody>(rootEntity, data.radius);
    }

    if (data.health > 0.0f) {
        registry.Emplace<ComponentHealth>(rootEntity, data.health);
        registry.Emplace<TargetTag>(rootEntity);
    }

    if (data.sprite.valid()) {
        registry.Emplace<ComponentSprite>(rootEntity, *data.sprite);
    }

    if (data.weapon.valid()) {
        SystemWeapon::InitWeapon(registry, rootEntity, *data.weapon, gamedata, true);
    }

    if (data.death_sprite.valid()) {
        registry.Emplace<ComponentDeathSprite>(rootEntity, data.death_sprite.get());
    }

    for (auto const& [sectionName, section] : data.sections) {
        auto childEntity = registry.Create();
        registry.Emplace<TargetTag>(childEntity);
        registry.Emplace<ComponentParenting>(childEntity, rootEntity,
                                             static_cast<moth::gfx::FloatVec2>(section.offset));
        registry.Emplace<ComponentPosition>(childEntity,
                                            position + static_cast<moth::gfx::FloatVec2>(section.offset));
        if (section.radius > 0.0f) {
            registry.Emplace<ComponentBody>(childEntity, section.radius);
        }
        if (section.health > 0) {
            registry.Emplace<ComponentHealth>(childEntity, section.health);
        }
        if (section.sprite.valid()) {
            registry.Emplace<ComponentSprite>(childEntity, *section.sprite);
        }

        if (section.weapon.valid()) {
            SystemWeapon::InitWeapon(registry, childEntity, *section.weapon, gamedata, true);
        }

        if (section.death_sprite.valid()) {
            registry.Emplace<ComponentDeathSprite>(childEntity, section.death_sprite.get());
        }

        rootDetails.m_children.insert(childEntity);
    }

    registry.Emplace<ComponentBehaviour>(rootEntity, behaviour);

    return rootEntity;
}

ComponentEnemySpawner::ComponentEnemySpawner(SpawnerData const& data)
    : m_active(true)
    , m_type(data.type)
    , m_offsetStep(data.offset_step)
    , m_formationPositions(data.formation_positions)
    , m_count(data.count)
    , m_maxCooldown(data.cooldown)
    , m_maxGroupCount(data.group_count)
    , m_maxGroupCooldown(data.group_delay)
    , m_groupDrop(data.drop)
    , m_currentGroupEntity(moth::ecs::kNullEntity)
    , m_killType(data.kill_type)
    , m_lifetime(data.lifetime)
    , m_boundsBorder(data.bounds_border)
    , m_tickOffset(data.tick_offset) {
    if (data.enemy == nullptr) {
        spdlog::error("Spawner with bad enemy reference");
    } else {
        m_enemy = *data.enemy;
    }
    if (data.behaviour == nullptr) {
        spdlog::error("Spawner with bad behaviour reference");
    } else {
        m_behaviour = *data.behaviour;
    }
}

void SystemEnemySpawner::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    auto const& gamedata = world.GetGameData();
    auto view = registry.View<ComponentEnemySpawner, ComponentPosition>();

    for (auto [entity, spawner, spawnerPosition] : view.each()) {
        if (spawner.m_count > 0) {
            if (spawner.m_cooldown > 0) {
                spawner.m_cooldown -= static_cast<int32_t>(ticks);
            }

            if (spawner.m_cooldown <= 0) {
                spawner.m_groupCount = spawner.m_maxGroupCount;
                if (spawner.m_maxGroupCount > 1) {
                    spawner.m_currentGroupEntity = moth::ecs::kNullEntity;
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
                    registry.Emplace<DeadTag>(entity);
                }

                auto const spawnIndex = spawner.m_maxGroupCount - (spawner.m_groupCount + 1);
                auto const fWorldSize = static_cast<moth::gfx::FloatVec2>(world.GetWorldSize());
                auto position = spawnerPosition.m_position;
                switch (spawner.m_type) {
                case SpawnerType::Staggered: {
                    auto const offsetStep = spawner.m_offsetStep * fWorldSize;
                    auto const side = spawner.m_groupCount % 2;
                    position += offsetStep * (side == 0 ? 1.0f : -1.0f);
                } break;
                case SpawnerType::Stepped: {
                    auto const offsetStep = spawner.m_offsetStep * fWorldSize;
                    position += offsetStep * static_cast<float>(spawnIndex);
                } break;
                case SpawnerType::Formation: {
                    position += spawner.m_formationPositions[spawnIndex] * fWorldSize;
                } break;
                default:
                    break;
                }

                auto enemy = SpawnEnemy(registry, spawner.m_enemy, spawner.m_behaviour, position, gamedata);


                if (spawner.m_killType == EnemyKillType::Time) {
                    auto& lifetime = registry.Emplace<ComponentLifetime>(enemy);
                    lifetime.m_msLeft = spawner.m_lifetime.value();
                } else if (spawner.m_killType == EnemyKillType::Bounds) {
                    auto& bounds = registry.Emplace<ComponentBounds>(enemy);
                    moth::gfx::FloatRect boundsRect{ { 0.0f, 0.0f },
                                                  static_cast<moth::gfx::FloatVec2>(world.GetWorldSize()) };
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
                    if (spawner.m_currentGroupEntity == moth::ecs::kNullEntity) {
                        spawner.m_currentGroupEntity =
                            SystemGroup::CreateGroup(registry, spawner.m_groupDrop);
                    }
                    SystemGroup::AddMember(registry, spawner.m_currentGroupEntity, enemy);
                }
            }
        }
    }
}

moth::ecs::Entity SystemEnemySpawner::CreateSpawner(moth::ecs::World& registry, SpawnerData const& data,
                                               GameData const& gamedata, moth::gfx::FloatVec2 const& position) {
    auto enemySpawner = registry.Create();
    registry.Emplace<ComponentEnemySpawner>(enemySpawner, data);
    registry.Emplace<ComponentPosition>(enemySpawner, position);
    return enemySpawner;
}
