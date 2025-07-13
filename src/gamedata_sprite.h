#pragma once

#include "gamedata_database.h"
#include <canyon/graphics/blend_mode.h>
#include <canyon/graphics/color.h>
#include <canyon/graphics/surface_context.h>
#include <nlohmann/json.hpp>

struct SpriteImage {
    std::shared_ptr<canyon::graphics::IImage> image;
    canyon::FloatVec2 scale;
    canyon::IntVec2 offset;
    float rotation;
    canyon::graphics::BlendMode blend_mode;
    canyon::graphics::Color color;
    int32_t zOrder;

    static SpriteImage Deserialize(nlohmann::json const& json, SerializeContext const& context);
};

struct SpriteData {
    std::map<std::string, SpriteImage> images;

    static SpriteData Deserialize(nlohmann::json const& json, SerializeContext const& context);
};
