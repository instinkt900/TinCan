#pragma once

#include "gamedata.h"
#include <moth/anim/anim_set.h>
#include <moth/graphics/graphics/asset_context.h>
#include <moth/graphics/graphics/blend_mode.h>
#include <moth/graphics/graphics/color.h>
#include <moth/graphics/graphics/image.h>
#include <moth/graphics/graphics/spritesheet.h>
#include <moth/graphics/graphics/spritesheet_factory.h>
#include <moth/graphics/utils/math_utils.h>
#include "utils_serialization.h"

#include <fstream>
#include <iterator>
#include <optional>

struct SpriteData {
    static constexpr GameDataCategory Category = GameDataCategory::Sprites;

    moth::gfx::Image image;
    std::shared_ptr<moth::gfx::SpriteSheet> sheet;
    std::string clip;
    std::optional<moth::anim::AnimSet> animSet;
    moth::gfx::FloatVec2 scale{ 1.0f, 1.0f };
    moth::gfx::BlendMode blend_mode = moth::gfx::BlendMode::Replace;
    moth::gfx::Color color = moth::gfx::BasicColors::White;
    float rotation = 0;
    int32_t z_order = 0;
};

inline void from_json(nlohmann::json const& json, SpriteData& data) {
    auto const hasImage = json.contains("image");
    auto const hasSheet = json.contains("sheet");
    auto const hasAnim = json.contains("anim");
    auto const sourceCount =
        static_cast<int>(hasImage) + static_cast<int>(hasSheet) + static_cast<int>(hasAnim);
    if (sourceCount != 1) {
        spdlog::error("Sprite data must define exactly one of 'image', 'sheet' or 'anim'");
        throw std::runtime_error("Sprite data must define exactly one of 'image', 'sheet' or 'anim'");
    }

    auto& context = *GameData::s_currentContext;
    auto& assetContext = context.surfaceContext.GetAssetContext();

    if (hasImage) {
        auto const imagePath = json.at("image").get<std::filesystem::path>();
        std::shared_ptr<moth::gfx::ITexture> texture = assetContext.TextureFromFile(imagePath);
        if (texture == nullptr) {
            spdlog::error("Failed loading image at {}", imagePath.string());
            throw std::runtime_error("Failed loading image " + imagePath.string());
        }
        data.image = moth::gfx::Image(std::move(texture));
    } else if (hasSheet) {
        auto const sheetPath = json.at("sheet").get<std::filesystem::path>();
        data.sheet = assetContext.GetSpriteSheetFactory().GetSpriteSheet(sheetPath);
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
    } else {
        auto const animPath = json.at("anim").get<std::filesystem::path>();
        std::ifstream animFile(animPath);
        if (!animFile.is_open()) {
            spdlog::error("Failed loading animation set at {}", animPath.string());
            throw std::runtime_error("Failed loading animation set " + animPath.string());
        }
        auto const animText = std::string(std::istreambuf_iterator<char>(animFile), std::istreambuf_iterator<char>{});
        auto const parsed = moth::anim::ParseAnimSet(animText);
        if (!parsed) {
            spdlog::error("Failed to parse animation set at {}", animPath.string());
            throw std::runtime_error("Failed to parse animation set " + animPath.string());
        }
        auto const flipbookPath = animPath.parent_path() / parsed->flipbook;
        data.sheet = assetContext.GetSpriteSheetFactory().GetSpriteSheet(flipbookPath);
        if (data.sheet == nullptr) {
            spdlog::error("Failed loading sprite sheet at {}", flipbookPath.string());
            throw std::runtime_error("Failed loading sprite sheet " + flipbookPath.string());
        }
        for (auto const& state : parsed->states) {
            if (!data.sheet->GetClipDesc(state.clip)) {
                spdlog::error("Animation state {} references missing clip {}", state.id, state.clip);
                throw std::runtime_error("Animation state " + state.id + " references missing clip " + state.clip);
            }
        }
        for (auto const& transition : parsed->transitions) {
            if (transition.clip && !data.sheet->GetClipDesc(*transition.clip)) {
                spdlog::error("Animation transition {} references missing clip {}", transition.id, *transition.clip);
                throw std::runtime_error("Animation transition " + transition.id + " references missing clip " +
                                         *transition.clip);
            }
        }
        data.animSet = *parsed;
    }

    DATA_OPTIONAL(json, data, scale);
    DATA_OPTIONAL(json, data, blend_mode);
    DATA_OPTIONAL(json, data, color);
    DATA_OPTIONAL(json, data, rotation);
    DATA_OPTIONAL(json, data, z_order);

    data.rotation = moth::gfx::DegToRad(data.rotation);
}
