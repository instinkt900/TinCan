#pragma once

#include "system_projectile.h"
#include <canyon/graphics/color.h>

inline canyon::graphics::Color GetEnergyColor(EnergyColor const& color) {
    return color == EnergyColor::WHITE ? canyon::graphics::BasicColors::Blue
                                       : canyon::graphics::BasicColors::Red;
}

#define MAGIC_SERIALIZE_ENUM(EnumType)                                                              \
inline void to_json(nlohmann::json& j, EnumType const& type) { j = magic_enum::enum_name(type); }   \
inline void from_json(const nlohmann::json& j, EnumType& type) {                                    \
    if (auto val = magic_enum::enum_cast<EnumType>(j.get<std::string>()); val.has_value()) {        \
        type = val.value();                                                                         \
    } else {                                                                                        \
        throw std::runtime_error("Invalid enum string: " + j.get<std::string>());                   \
    }                                                                                               \
}
