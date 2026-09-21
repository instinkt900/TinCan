#pragma once

#include "gamedata.h"
#include "input_action.h"
#include <moth/ui/events/event_key.h>
#include <algorithm>
#include <map>
#include <vector>

inline constexpr char kDefaultInputName[] = "default";

// The key bindings, held the way a key event needs them: a key gives the actions it was bound to.
// The data is authored the other way round, an action listing its keys, which is how a player
// thinks about it and lets one action take more than one key.
struct InputData {
    static constexpr GameDataCategory Category = GameDataCategory::Input;

    // The actions a key was bound to. Empty for an unbound key.
    std::vector<InputAction> const& Actions(moth::ui::Key key) const {
        static std::vector<InputAction> const none;
        auto const entry = m_keyActions.find(key);
        return entry == m_keyActions.end() ? none : entry->second;
    }

    bool IsBound(InputAction action, moth::ui::Key key) const {
        auto const& actions = Actions(key);
        return std::find(actions.begin(), actions.end(), action) != actions.end();
    }

    std::map<moth::ui::Key, std::vector<InputAction>> m_keyActions;
};

inline void from_json(nlohmann::json const& json, InputData& data) {
    // A binding that does not parse is skipped rather than taken as fatal, the same way a bad
    // database entry is: the rest of the bindings still work.
    for (auto const& [actionName, keysJson] : json.items()) {
        auto const action = magic_enum::enum_cast<InputAction>(actionName, magic_enum::case_insensitive);
        if (!action.has_value() || action.value() == InputAction::Count) {
            spdlog::error("Unknown input action {}", actionName);
            continue;
        }
        if (!keysJson.is_array()) {
            spdlog::error("Input action {} is not a list of keys", actionName);
            continue;
        }
        for (auto const& keyJson : keysJson) {
            auto const keyName = keyJson.get<std::string>();
            auto const key = magic_enum::enum_cast<moth::ui::Key>(keyName, magic_enum::case_insensitive);
            if (!key.has_value() || key.value() == moth::ui::Key::Unknown
                || key.value() == moth::ui::Key::Count) {
                spdlog::error("Unknown key {} bound to input action {}", keyName, actionName);
                continue;
            }
            data.m_keyActions[key.value()].push_back(action.value());
        }
    }
}
