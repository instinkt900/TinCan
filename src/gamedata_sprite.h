#pragma once

#include "gamedata.h"
#include <canyon/graphics/blend_mode.h>
#include <canyon/graphics/color.h>
#include <canyon/graphics/iimage.h>
#include <canyon/utils/math_utils.h>
#include "utils_serialization.h"

struct SpriteSheetData {
    canyon::IntVec2 cell_dimensions;
    canyon::IntVec2 grid_dimensions;
    float fps = 0;
};

inline void from_json(nlohmann::json const& json, SpriteSheetData& data) {
    DATA_REQUIRED(json, data, cell_dimensions);
    DATA_REQUIRED(json, data, grid_dimensions);
    DATA_OPTIONAL_DEFAULT(json, data, fps, 0.0f);
}

struct SpriteData {
    static constexpr GameDataCategory Category = GameDataCategory::Sprites;
    
    std::shared_ptr<canyon::graphics::IImage> image;
    canyon::FloatVec2 scale{ 1.0f, 1.0f };
    canyon::graphics::BlendMode blend_mode = canyon::graphics::BlendMode::Replace;
    canyon::graphics::Color color = canyon::graphics::BasicColors::White;
    float rotation = 0;
    int32_t z_order = 0;
    std::optional<SpriteSheetData> sheet_data;
};

inline void from_json(nlohmann::json const& json, SpriteData& data) {
    auto const imagePath = json.at("image").get<std::filesystem::path>();
    data.image = GameData::s_currentContext->surfaceContext.ImageFromFile(imagePath);
    if (data.image == nullptr) {
        spdlog::error("Failed loading image at {}", imagePath.string());
        throw std::runtime_error("Failed loading image " + imagePath.string());
    }

    DATA_OPTIONAL(json, data, scale);
    DATA_OPTIONAL(json, data, blend_mode);
    DATA_OPTIONAL(json, data, color);
    DATA_OPTIONAL(json, data, rotation);
    DATA_OPTIONAL(json, data, z_order);
    DATA_OPTIONAL(json, data, sheet_data);

    data.rotation = canyon::Radians(data.rotation);
}
