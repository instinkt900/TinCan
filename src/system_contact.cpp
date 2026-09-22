#include "system_contact.h"
#include "component_body.h"
#include "component_contact.h"
#include "component_entity.h"
#include "game_world.h"
#include "system_damage.h"
#include "system_movement.h"
#include "tags.h"
#include "utils_collision.h"
#include "utils_entity.h"
#include <algorithm>
#include <vector>

namespace {
    // A snapshot of one contact-capable entity. The values are copied out of the pools before any
    // damage lands, because applying damage adds components and so may move the pools underneath us.
    struct Contactant {
        moth::ecs::Entity entity;
        Team team;
        float radius;
        moth::gfx::FloatVec2 position;
        moth::gfx::FloatVec2 lastPosition;
        float damage;
        float selfDamage;
    };

    struct Hit {
        moth::ecs::Entity entity;
        float damage;
    };
}

void SystemContact::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    auto const& gamedata = world.GetGameData();

    std::vector<Contactant> contactants;

    auto view = registry.View<ComponentContact, ComponentBody, ComponentPosition>(moth::ecs::Exclude<DeadTag>);
    for (auto [entity, contact, body, position] : view.each()) {
        if (contact.m_cooldownLeft > 0) {
            contact.m_cooldownLeft -= std::min(contact.m_cooldownLeft, static_cast<int32_t>(ticks));
            // still recovering, so it neither deals nor takes contact damage this frame
            continue;
        }

        auto const* entityDetails = GetEntityDetails(registry, entity);
        if (entityDetails == nullptr) {
            continue;
        }

        contactants.push_back({ entity, entityDetails->m_team, body.m_radius, position.m_position,
                                position.m_lastPosition, contact.m_damage, contact.m_selfDamage });
    }

    std::vector<Hit> hits;
    std::vector<moth::ecs::Entity> touched;

    for (size_t i = 0; i < contactants.size(); ++i) {
        for (size_t j = i + 1; j < contactants.size(); ++j) {
            auto const& first = contactants[i];
            auto const& second = contactants[j];

            if (first.team == second.team) {
                continue;
            }

            auto const t = SweepTest(first.position, first.lastPosition, first.radius, second.position,
                                     second.lastPosition, second.radius);
            if (t >= 1.0f) {
                continue;
            }

            // each side deals its own damage to the other, and to itself if it is the sort of thing
            // that comes off worse for the collision
            if (first.damage > 0.0f) {
                hits.push_back({ second.entity, first.damage });
            }
            if (second.damage > 0.0f) {
                hits.push_back({ first.entity, second.damage });
            }
            if (first.selfDamage > 0.0f) {
                hits.push_back({ first.entity, first.selfDamage });
            }
            if (second.selfDamage > 0.0f) {
                hits.push_back({ second.entity, second.selfDamage });
            }

            touched.push_back(first.entity);
            touched.push_back(second.entity);
        }
    }

    // start the cooldowns before any damage lands, while the pools are still where we left them
    for (auto const entity : touched) {
        if (auto* contact = registry.TryGet<ComponentContact>(entity)) {
            contact->m_cooldownLeft = contact->m_cooldown;
        }
    }

    for (auto const& hit : hits) {
        if (registry.Valid(hit.entity) && !registry.Has<DeadTag>(hit.entity)) {
            SystemDamage::ApplyDamage(registry, hit.entity, hit.damage, gamedata);
        }
    }
}
