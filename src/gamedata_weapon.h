#pragma once

#include <canyon/graphics/surface_context.h>
#include <moth_ui/utils/vector.h>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>

class Gamedata;

struct BarrelData {
    moth_ui::IntVec2 offset;
    int32_t group = 0;
};

struct WeaponData {
    std::string name;
    int32_t cooldown;
    bool player_tracking;
    std::string projectile_name;
    std::vector<BarrelData> barrels;

    bool Load(nlohmann::json const& json, Gamedata const& gamedata, canyon::graphics::SurfaceContext& surfaceContext);
};

