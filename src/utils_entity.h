#pragma once

#include "component_entity.h"
#include "system_movement.h"
#include <entt/entt.hpp>

inline ComponentEntity const* GetEntityDetails(entt::registry& registry, entt::entity entity) {
    if (auto const* entityDetails = registry.try_get<ComponentEntity>(entity)) {
        return entityDetails;
    }

    if (auto const* parenting = registry.try_get<ComponentParenting>(entity)) {
        return GetEntityDetails(registry, parenting->m_parent);
    }

    return nullptr;
}
