#pragma once

#include "system_behaviour.h"
#include <canyon/graphics/surface_context.h>
#include <nlohmann/json_fwd.hpp>

class Gamedata;

struct SpawnerData {
    std::string name;
    std::string enemy_name;
    std::string behaviour_name;
    BehaviourParameterList behaviour_parameters;
    int32_t count;
    int32_t cooldown;
    int32_t group_count;
    int32_t group_delay;

    bool Load(nlohmann::json const& json, Gamedata const& gamedata, canyon::graphics::SurfaceContext& surfaceContext);
};
