#pragma once

#include "gamedata.h"
#include <moth_graphics/graphics/blend_mode.h>
#include <moth_graphics/graphics/color.h>
#include <moth_graphics/graphics/iimage.h>
#include <moth_graphics/utils/math_utils.h>
#include "utils_serialization.h"

struct SpriteSheetData {
    moth_graphics::IntVec2 cell_dimensions;
    moth_graphics::IntVec2 grid_dimensions;
    float fps = 0;
};

inline void from_json(nlohmann::json const& json, SpriteSheetData& data) {
    DATA_REQUIRED(json, data, cell_dimensions);
    DATA_REQUIRED(json, data, grid_dimensions);
    DATA_OPTIONAL_DEFAULT(json, data, fps, 0.0f);
}

struct SpriteData {
    static constexpr GameDataCategory Category = GameDataCategory::Sprites;
    
    std::shared_ptr<moth_graphics::graphics::IImage> image;
    moth_graphics::FloatVec2 scale{ 1.0f, 1.0f };
    moth_graphics::graphics::BlendMode blend_mode = moth_graphics::graphics::BlendMode::Replace;
    moth_graphics::graphics::Color color = moth_graphics::graphics::BasicColors::White;
    float rotation = 0;
    int32_t z_order = 0;
    std::optional<SpriteSheetData> sheet_data;
};

inline void from_json(nlohmann::json const& json, SpriteData& data) {
    auto const imagePath = json.at("image").get<std::filesystem::path>();
    data.image = GameData::s_currentContext->surfaceContext.GetAssetContext().ImageFromFile(imagePath);
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

    data.rotation = moth_graphics::Radians(data.rotation);
}
