#pragma once

#include "gamedata.h"
#include "utils_serialization.h"
#include <string>

// A sound effect: the file it plays and how loud. Several copies of one sound can play at once, up to its
// voices, so a burst of the same event does not cut itself off.
struct SoundData {
    static constexpr GameDataCategory Category = GameDataCategory::Sounds;

    std::string file;
    float volume = 1.0f;
    int32_t voices = 8;
};

inline void from_json(nlohmann::json const& json, SoundData& data) {
    DATA_REQUIRED(json, data, file);
    DATA_OPTIONAL(json, data, volume);
    DATA_OPTIONAL(json, data, voices);
}
