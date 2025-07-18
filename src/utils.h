#pragma once

#include <canyon/graphics/color.h>
#include <optional>
#include <nlohmann/json.hpp>
#include <magic_enum.hpp>

#define MAGIC_SERIALIZE_ENUM(EnumType)                                                              \
inline void to_json(nlohmann::json& j, EnumType const& type) { j = magic_enum::enum_name(type); }   \
inline void from_json(const nlohmann::json& j, EnumType& type) {                                    \
    if (auto val = magic_enum::enum_cast<EnumType>(j.get<std::string>()); val.has_value()) {        \
        type = val.value();                                                                         \
    } else {                                                                                        \
        throw std::runtime_error("Invalid enum string: " + j.get<std::string>());                   \
    }                                                                                               \
}

namespace nlohmann {
    template<typename T>
    struct adl_serializer<std::optional<T>> {
        static void to_json(json& j, const std::optional<T>& opt) {
            if (opt.has_value()) {
                j = *opt;
            } else {
                j = nullptr;
            }
        }

        static void from_json(const json& j, std::optional<T>& opt) {
            if (j.is_null()) {
                opt = std::nullopt;
            } else {
                opt = j.get<T>();
            }
        }
    };
}

