#pragma once

#include <entt/entity/fwd.hpp>
#include <canyon/utils/vector.h>
#include <optional>

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
    canyon::FloatVec2 m_position;
    std::optional<std::string> m_drop;
};

struct ComponentGroupId {
    entt::entity m_group;
};

class SystemGroup {
public:
    static void Update(GameWorld& world, uint32_t ticks);
    static entt::entity CreateGroup(entt::registry& registry, std::optional<std::string> drop);
    static void AddMember(entt::registry& registry, entt::entity group, entt::entity member);
    static void RemoveMember(entt::registry& registry, entt::entity group, entt::entity member, GroupEndCondition condition);
};
