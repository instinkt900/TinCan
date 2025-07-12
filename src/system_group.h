#pragma once

#include "gamedata.h"
#include <entt/entity/fwd.hpp>
#include <moth_ui/utils/vector.h>
#include <optional>

enum class GroupEndCondition {
    None,
    Killed,
    Escaped,
    Mixed
};

struct ComponentGroup {
    int32_t m_memberCount = 0;
    GroupEndCondition m_condition = GroupEndCondition::None;
    moth_ui::FloatVec2 m_position;
    std::optional<std::string> m_drop;
};

struct ComponentGroupId {
    entt::entity m_group;
};

class SystemGroup {
public:
    static void Update(entt::registry& registry, uint32_t ticks, Gamedata const& gamedata);
    static entt::entity CreateGroup(entt::registry& registry, std::optional<std::string> drop);
    static void AddMember(entt::registry& registry, entt::entity group, entt::entity member);
    static void RemoveMember(entt::registry& registry, entt::entity group, entt::entity member, GroupEndCondition condition);
};
