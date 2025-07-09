#pragma once

#include "gamedata_database.h"
#include <nlohmann/json_fwd.hpp>
#include <string>

struct DropData {
    std::string sprite;

    static DropData Deserialize(nlohmann::json const& json, SerializeContext const& context);
};
