#include "system_targeting.h"
#include "component_body.h"
#include "component_entity.h"
#include "system_movement.h"
#include "tags.h"
#include "utils.h"

namespace {
    void UpdateTargetingNearest(entt::registry& registry, entt::entity entity,
                                ComponentEntity const& entityDetails, ComponentTargeting& targeting) {
        // nothing to do if we already have a target and it's valid
        if (targeting.m_currentTarget != entt::null && registry.valid(targeting.m_currentTarget)) {
            return;
        }

        // need the entity position if we want to find the closest
        auto const* entityPosition = registry.try_get<ComponentPosition>(entity);
        if (entityPosition == nullptr) {
            return;
        }

        // pull all targets and find the closest
        auto view = registry.view<ComponentEntity, ComponentPosition, TargetTag>(entt::exclude<DeadTag>);
        entt::entity closestEntity = entt::null;
        float closestDistance = std::numeric_limits<float>::max();
        for (auto [targetEntity, targetDetails, targetPosition] : view.each()) {
            if (entityDetails.m_team != targetDetails.m_team) {
                float const distance =
                    canyon::DistanceSq(entityPosition->m_position, targetPosition.m_position);
                if (distance < closestDistance) {
                    closestEntity = targetEntity;
                    closestDistance = distance;
                }
            }
        }

        // update the target if we found one
        if (closestEntity != entt::null) {
            targeting.m_currentTarget = closestEntity;
        }
    }

    void UpdateTargetingRandom(entt::registry& registry, entt::entity entity,
                               ComponentEntity const& entityDetails, ComponentTargeting& targeting) {
        // nothing to do if we already have a target and it's valid
        if (targeting.m_currentTarget != entt::null && registry.valid(targeting.m_currentTarget)) {
            return;
        }

        // pull all targets and pick a random one
        auto view = registry.view<ComponentEntity, TargetTag>(entt::exclude<DeadTag>);
        std::vector<entt::entity> possibleTargets;
        for (auto [targetEntity, targetDetails] : view.each()) {
            if (entityDetails.m_team != targetDetails.m_team) {
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
    auto view = registry.view<ComponentTargeting, ComponentEntity>(entt::exclude<DeadTag>);
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
