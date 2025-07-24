#pragma once

#include "gamedata.h"
#include <canyon/graphics/blend_mode.h>
#include <canyon/graphics/color.h>
#include <canyon/graphics/iimage.h>
#include <canyon/utils/math_utils.h>
#include <memory>
#include <nlohmann/detail/macro_scope.hpp>
#include "utils_serialization.h"

struct SpriteImage {
    std::shared_ptr<canyon::graphics::IImage> image;
    canyon::FloatVec2 scale;
    canyon::IntVec2 offset;
    float rotation;
    canyon::graphics::BlendMode blend_mode;
    canyon::graphics::Color color;
    int32_t zOrder;
};

inline void from_json(nlohmann::json const& json, SpriteImage& data) {
    DATA_REQUIRED(json, data, scale);
    DATA_REQUIRED(json, data, offset);
    DATA_REQUIRED(json, data, rotation);
    DATA_REQUIRED(json, data, blend_mode);
    DATA_REQUIRED(json, data, color);

    auto const imagePath = json.at("image").get<std::filesystem::path>();
    data.image = GameData::s_currentContext->surfaceContext.ImageFromFile(imagePath);
    if (data.image == nullptr) {
        spdlog::error("Failed loading image at {}", imagePath.string());
        throw std::runtime_error("Failed loading image " + imagePath.string());
    }

    data.rotation = canyon::Radians(data.rotation);
}

struct SpriteData {
    static constexpr GameDataCategory Category = GameDataCategory::Sprites;
    
    std::map<std::string, SpriteImage> images;
};

inline void from_json(nlohmann::json const& json, SpriteData& data) {
    DATA_REQUIRED(json, data, images);
}
