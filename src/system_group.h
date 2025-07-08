#pragma once

#include <entt/entity/fwd.hpp>

enum class GroupEndCondition {
    None,
    Killed,
    Escaped,
    Mixed
};

struct ComponentGroup {
    int32_t m_memberCount;
    GroupEndCondition m_condition;
};

struct ComponentGroupId {
    entt::entity m_group;
};

class SystemGroup {
public:
    static void Update(entt::registry& registry, uint32_t ticks);
    static entt::entity CreateGroup(entt::registry& registry);
    static void AddMember(entt::registry& registry, entt::entity group, entt::entity member);
    static void RemoveMember(entt::registry& registry, entt::entity group, GroupEndCondition condition);
};
