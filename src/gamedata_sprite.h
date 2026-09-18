#pragma once

#include "gamedata.h"
#include <moth/graphics/graphics/asset_context.h>
#include <moth/graphics/graphics/blend_mode.h>
#include <moth/graphics/graphics/color.h>
#include <moth/graphics/graphics/image.h>
#include <moth/graphics/utils/math_utils.h>
#include "utils_serialization.h"

struct SpriteData {
    static constexpr GameDataCategory Category = GameDataCategory::Sprites;

    moth::gfx::Image image;
    moth::gfx::FloatVec2 scale{ 1.0f, 1.0f };
    moth::gfx::BlendMode blend_mode = moth::gfx::BlendMode::Replace;
    moth::gfx::Color color = moth::gfx::BasicColors::White;
    float rotation = 0;
    int32_t z_order = 0;
};

inline void from_json(nlohmann::json const& json, SpriteData& data) {
    auto const imagePath = json.at("image").get<std::filesystem::path>();
    std::shared_ptr<moth::gfx::ITexture> texture =
        GameData::s_currentContext->surfaceContext.GetAssetContext().TextureFromFile(imagePath);
    if (texture == nullptr) {
        spdlog::error("Failed loading image at {}", imagePath.string());
        throw std::runtime_error("Failed loading image " + imagePath.string());
    }
    data.image = moth::gfx::Image(std::move(texture));

    DATA_OPTIONAL(json, data, scale);
    DATA_OPTIONAL(json, data, blend_mode);
    DATA_OPTIONAL(json, data, color);
    DATA_OPTIONAL(json, data, rotation);
    DATA_OPTIONAL(json, data, z_order);

    data.rotation = moth::gfx::DegToRad(data.rotation);
}
