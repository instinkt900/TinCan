#pragma once

#include <canyon/graphics/blend_mode.h>
#include <nlohmann/json.hpp>
#include <magic_enum.hpp>

#define MAGIC_SERIALIZE_ENUM(EnumType)                                                                       \
    inline void to_json(nlohmann::json& j, EnumType const& type) { j = magic_enum::enum_name(type); }        \
    inline void from_json(const nlohmann::json& j, EnumType& type) {                                         \
        if (auto val = magic_enum::enum_cast<EnumType>(j.get<std::string>()); val.has_value()) {             \
            type = val.value();                                                                              \
        } else {                                                                                             \
            throw std::runtime_error("Invalid enum string: " + j.get<std::string>());                        \
        }                                                                                                    \
    }

#define DATA_REQUIRED(json_, obj, name_)                                                                     \
    {                                                                                                        \
        try {                                                                                                \
            json_.at(#name_).get_to(obj.name_);                                                              \
        } catch (nlohmann::json::out_of_range const& e) {                                                    \
            spdlog::error("Data type {} is missing required field {}.", typeid(obj).name(), #name_);         \
            throw;                                                                                           \
        }                                                                                                    \
    }
#define DATA_OPTIONAL_DEFAULT(json, obj, name, default_) obj.name = json.value(#name, default_);
#define DATA_OPTIONAL(json_, obj, name)                                                                      \
    {                                                                                                        \
        auto const v = json_.value(#name, nlohmann::json());                                                 \
        if (!v.is_null()) {                                                                                  \
            v.get_to(obj.name);                                                                              \
        }                                                                                                    \
    }

namespace nlohmann {
    template <typename T>
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

    template<>
    struct adl_serializer<canyon::graphics::BlendMode> {
        static void to_json(json& j, canyon::graphics::BlendMode const& blendMode) {
            j = magic_enum::enum_name(blendMode);
        }

        static void from_json(const json& j, canyon::graphics::BlendMode& blendMode) {
            if (auto val = magic_enum::enum_cast<canyon::graphics::BlendMode>(j.get<std::string>()); val.has_value()) {
                blendMode = val.value();
            } else {
                throw std::runtime_error("Invalid enum string: " + j.get<std::string>());
            }
        }
    };
}
