#pragma once

#include <nlohmann/json.hpp>
#include <optional>

#define opt_get(json, obj, name) opt_get_to(json, #name, obj.name)

template<typename T>
inline void opt_get_to(nlohmann::json const& j, std::string const& fieldName, std::optional<T>& target) {
    auto const jobj = j.value(fieldName, nlohmann::json());
    if (!jobj.is_null()) {
        target = jobj;
    }
}

