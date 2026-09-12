#include "system_targeting.h"
#include "component_entity.h"
#include "component_health.h"
#include "system_movement.h"
#include "tags.h"
#include "utils.h"
#include "utils_entity.h"

namespace {
    void UpdateTargetingNearest(moth::ecs::World& registry, moth::ecs::Entity entity,
                                ComponentEntity const& entityDetails, ComponentTargeting& targeting) {
        // nothing to do if we already have a target and it's valid
        if (targeting.m_currentTarget != moth::ecs::kNullEntity && registry.Valid(targeting.m_currentTarget)) {
            return;
        }

        // need the entity position if we want to find the closest
        auto const* entityPosition = registry.TryGet<ComponentPosition>(entity);
        if (entityPosition == nullptr) {
            return;
        }

        // pull all targets and find the closest
        auto view = registry.View<ComponentHealth, ComponentPosition, TargetTag>(moth::ecs::Exclude<DeadTag>);
        moth::ecs::Entity closestEntity = moth::ecs::kNullEntity;
        float closestDistance = std::numeric_limits<float>::max();
        for (auto [targetEntity, targetHealth, targetPosition] : view.each()) {
            auto const* targetDetails = GetEntityDetails(registry, targetEntity);
            if (targetDetails == nullptr) {
                continue;
            }
            if (entityDetails.m_team != targetDetails->m_team) {
                float const distance =
                    moth::gfx::DistanceSq(entityPosition->m_position, targetPosition.m_position);
                if (distance < closestDistance) {
                    closestEntity = targetEntity;
                    closestDistance = distance;
                }
            }
        }

        // update the target if we found one
        if (closestEntity != moth::ecs::kNullEntity) {
            targeting.m_currentTarget = closestEntity;
        }
    }

    void UpdateTargetingRandom(moth::ecs::World& registry, moth::ecs::Entity entity,
                               ComponentEntity const& entityDetails, ComponentTargeting& targeting) {
        // nothing to do if we already have a target and it's valid
        if (targeting.m_currentTarget != moth::ecs::kNullEntity && registry.Valid(targeting.m_currentTarget)) {
            return;
        }

        // pull all targets and pick a random one
        auto view = registry.View<ComponentHealth, TargetTag>(moth::ecs::Exclude<DeadTag>);
        std::vector<moth::ecs::Entity> possibleTargets;
        for (auto [targetEntity, targetHealth] : view.each()) {
            auto const* targetDetails = GetEntityDetails(registry, targetEntity);
            if (targetDetails == nullptr) {
                continue;
            }
            if (entityDetails.m_team != targetDetails->m_team) {
                possibleTargets.push_back(targetEntity);
            }
        }

        // update the target to a random one if we have targets
        if (!possibleTargets.empty()) {
            auto const maxIndex = static_cast<int>(possibleTargets.size() - 1);
            targeting.m_currentTarget = possibleTargets[Random(0, maxIndex)];
        }
    }
}

void SystemTargeting::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    auto view = registry.View<ComponentTargeting, ComponentEntity>(moth::ecs::Exclude<DeadTag>);
    for (auto [entity, targeting, entityDetails] : view.each()) {
        //
        switch (targeting.m_targeting) {
        default:
        case Targeting::Nearest:
            UpdateTargetingNearest(registry, entity, entityDetails, targeting);
            break;
        case Targeting::Random:
            UpdateTargetingRandom(registry, entity, entityDetails, targeting);
        }
    }
}
