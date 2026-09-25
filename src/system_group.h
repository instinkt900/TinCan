#pragma once

#include "gamedata_pickup.h"
#include <moth/ecs/world.h>
#include <moth/graphics/utils/vector.h>
#include <vector>

class GameWorld;

enum class GroupEndCondition {
    None,
    Killed,
    Escaped,
    Mixed
};

struct ComponentGroup {
    int32_t m_memberCount = 0;
    GroupEndCondition m_condition = GroupEndCondition::None;
    moth::gfx::FloatVec2 m_position;
    DataRef<PickupData> m_drop;
};

// the groups an entity is a member of. a death spawn's pieces join their parent's groups, and a group of
// their own when their spawn has a drop
struct ComponentGroupId {
    std::vector<moth::ecs::Entity> m_groups;
};

class SystemGroup {
public:
    static void Update(GameWorld& world, uint32_t ticks);
    static moth::ecs::Entity CreateGroup(moth::ecs::World& registry, DataRef<PickupData> drop);
    static void AddMember(moth::ecs::World& registry, moth::ecs::Entity group, moth::ecs::Entity member);
    static void RemoveMember(moth::ecs::World& registry, moth::ecs::Entity group, moth::ecs::Entity member, GroupEndCondition condition);
};
