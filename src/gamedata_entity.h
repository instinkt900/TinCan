#pragma once

#include "utils_serialization.h"

enum class Team {
    None = -1,
    Player,
    Enemy,
};
MAGIC_SERIALIZE_ENUM(Team);

enum class Affinity {
    None = -1,
    Light,
    Dark
};
MAGIC_SERIALIZE_ENUM(Affinity);

