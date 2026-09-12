#pragma once

#include "component_entity.h"
#include "system_movement.h"
#include <moth/ecs/world.h>

inline ComponentEntity const* GetEntityDetails(moth::ecs::World& registry, moth::ecs::Entity entity) {
    if (auto const* entityDetails = registry.TryGet<ComponentEntity>(entity)) {
        return entityDetails;
    }

    if (auto const* parenting = registry.TryGet<ComponentParenting>(entity)) {
        return GetEntityDetails(registry, parenting->m_parent);
    }

    return nullptr;
}
