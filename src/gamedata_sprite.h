#pragma once

#include "gamedata.h"
#include <moth/graphics/graphics/asset_context.h>
#include <moth/graphics/graphics/blend_mode.h>
#include <moth/graphics/graphics/color.h>
#include <moth/graphics/graphics/image.h>
#include <moth/graphics/graphics/spritesheet.h>
#include <moth/graphics/graphics/spritesheet_factory.h>
#include <moth/graphics/utils/math_utils.h>
#include "utils_serialization.h"

struct SpriteData {
    static constexpr GameDataCategory Category = GameDataCategory::Sprites;

    moth::gfx::Image image;
    std::shared_ptr<moth::gfx::SpriteSheet> sheet;
    std::string clip;
    moth::gfx::FloatVec2 scale{ 1.0f, 1.0f };
    moth::gfx::BlendMode blend_mode = moth::gfx::BlendMode::Replace;
    moth::gfx::Color color = moth::gfx::BasicColors::White;
    float rotation = 0;
    int32_t z_order = 0;
};

inline void from_json(nlohmann::json const& json, SpriteData& data) {
    auto const hasImage = json.contains("image");
    auto const hasSheet = json.contains("sheet");
    auto const sourceCount = static_cast<int>(hasImage) + static_cast<int>(hasSheet);
    if (sourceCount != 1) {
        spdlog::error("Sprite data must define exactly one of 'image' or 'sheet'");
        throw std::runtime_error("Sprite data must define exactly one of 'image' or 'sheet'");
    }

    auto& context = *GameData::s_currentContext;

    if (hasImage) {
        auto const imagePath = json.at("image").get<std::filesystem::path>();
        std::shared_ptr<moth::gfx::ITexture> texture =
            context.surfaceContext.GetAssetContext().TextureFromFile(imagePath);
        if (texture == nullptr) {
            spdlog::error("Failed loading image at {}", imagePath.string());
            throw std::runtime_error("Failed loading image " + imagePath.string());
        }
        data.image = moth::gfx::Image(std::move(texture));
    } else {
        auto const sheetPath = json.at("sheet").get<std::filesystem::path>();
        data.sheet = context.surfaceContext.GetAssetContext().GetSpriteSheetFactory().GetSpriteSheet(sheetPath);
        if (data.sheet == nullptr) {
            spdlog::error("Failed loading sprite sheet at {}", sheetPath.string());
            throw std::runtime_error("Failed loading sprite sheet " + sheetPath.string());
        }
        data.clip = json.value("clip", std::string{});
        if (data.clip.empty()) {
            if (data.sheet->GetClipCount() < 1) {
                spdlog::error("Sprite sheet {} has no clips and no clip was named", sheetPath.string());
                throw std::runtime_error("Sprite sheet has no clips: " + sheetPath.string());
            }
            data.clip = std::string(data.sheet->GetClipName(0));
        } else if (!data.sheet->GetClipDesc(data.clip)) {
            spdlog::error("Sprite sheet {} has no clip named {}", sheetPath.string(), data.clip);
            throw std::runtime_error("Sprite sheet has no clip named " + data.clip);
        }
    }

    DATA_OPTIONAL(json, data, scale);
    DATA_OPTIONAL(json, data, blend_mode);
    DATA_OPTIONAL(json, data, color);
    DATA_OPTIONAL(json, data, rotation);
    DATA_OPTIONAL(json, data, z_order);

    data.rotation = moth::gfx::DegToRad(data.rotation);
}
