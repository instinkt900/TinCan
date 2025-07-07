#pragma once

#include "system_lifetime.h"
#include "system_movement.h"
#include <entt/entity/fwd.hpp>
#include <map>
#include <moth_ui/utils/vector.h>
#include <variant>
#include <vector>

struct CurveSample {
    moth_ui::FloatVec2 position;
    float segment_length;
    float total_distance;
};

struct ComponentCurveCache {
    std::vector<CurveSample> samples;
    float total_length;
};

using BehaviourParameterList = std::map<std::string, std::variant<std::string, int32_t, float>>;

struct ComponentBehaviour {
    std::string m_behaviourName;
    moth_ui::FloatVec2 m_offset;
    BehaviourParameterList m_parameters;
};

class SystemBehaviour {
public:
    SystemBehaviour();
    void Update(entt::registry& registry, uint32_t ticks);

private:
    using BehaviourFunc = void(*)(entt::registry&, entt::entity, ComponentBehaviour&, ComponentPosition&, ComponentLifetime&);
    std::map<std::string, BehaviourFunc> m_behaviours;
};
