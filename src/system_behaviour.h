#pragma once

#include "system_movement.h"
#include <entt/entity/fwd.hpp>
#include <map>
#include <moth_ui/utils/vector.h>
#include <variant>

struct ComponentBehaviour {
    std::string m_behaviourName;
    moth_ui::FloatVec2 m_offset;
    std::map<std::string, std::variant<std::string, int32_t, float>> m_parameters;
};

class SystemBehaviour {
public:
    SystemBehaviour();
    void Update(entt::registry& registry, uint32_t ticks);

private:
    using BehaviourFunc = void(*)(entt::entity, ComponentBehaviour&, ComponentPosition&, uint32_t);
    std::map<std::string, BehaviourFunc> m_behaviours;
};
