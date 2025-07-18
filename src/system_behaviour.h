#pragma once

#include <entt/entity/fwd.hpp>
#include <map>
#include <canyon/utils/vector.h>
#include <variant>
#include <vector>

struct CurveSample {
    canyon::FloatVec2 position;
    float segment_length = 0;
    float total_distance = 0;
};

struct ComponentCurveCache {
    std::vector<CurveSample> samples;
    float total_length;
};

using BehaviourParameterList = std::map<std::string, std::variant<std::string, int32_t, float>>;

struct ComponentBehaviour {
    std::string m_behaviourName;
    canyon::FloatVec2 m_offset;
    BehaviourParameterList m_parameters;
};

class SystemBehaviour {
public:
    static void Update(entt::registry& registry, uint32_t ticks);
};
