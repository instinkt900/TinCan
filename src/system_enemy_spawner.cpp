#include "system_enemy_spawner.h"
#include "component_body.h"
#include "component_contact.h"
#include "component_death_sprite.h"
#include "component_entity.h"
#include "component_health.h"
#include "system_behaviour.h"
#include "system_movement.h"
#include "system_drawable.h"
#include "system_weapon.h"
#include "tags.h"
#include <moth/ecs/world.h>
#include <moth/profile/profiler.h>

moth::ecs::Entity SystemEnemySpawner::SpawnEnemy(moth::ecs::World& registry, EnemyData const& data,
                                            BehaviourData const& behaviour, moth::gfx::FloatVec2 const& position,
                                            GameData const& gamedata) {
    MOTH_PROFILE_SCOPE("SpawnEnemy");
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
        registry.Emplace<ComponentSprite>(rootEntity, *data.sprite, data.spin_scale);
    }

    if (data.weapon.valid()) {
        SystemWeapon::InitWeapon(registry, rootEntity, data.weapon.ref_name(), *data.weapon, gamedata,
                                 true);
    }

    if (data.death_sprite.valid()) {
        registry.Emplace<ComponentDeathSprite>(rootEntity, data.death_sprite.get());
    }

    if (data.contact.has_value()) {
        registry.Emplace<ComponentContact>(rootEntity, *data.contact);
    }

    {
        MOTH_PROFILE_SCOPE("Enemy sections");
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
                // a section spins only if its own sprite asks for it: sections are parented by offset
                // and draw at angle 0, so the root's spin scale would set them turning on their own
                registry.Emplace<ComponentSprite>(childEntity, *section.sprite);
            }

            if (section.weapon.valid()) {
                SystemWeapon::InitWeapon(registry, childEntity, section.weapon.ref_name(), *section.weapon,
                                         gamedata, true);
            }

            if (section.death_sprite.valid()) {
                registry.Emplace<ComponentDeathSprite>(childEntity, section.death_sprite.get());
            }

            if (section.contact.has_value()) {
                registry.Emplace<ComponentContact>(childEntity, *section.contact);
            }

            rootDetails.m_children.insert(childEntity);
        }
    }

    registry.Emplace<ComponentBehaviour>(rootEntity, behaviour);

    return rootEntity;
}
