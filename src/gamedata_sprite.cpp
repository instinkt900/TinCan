#include "gamedata_sprite.h"
#include <canyon/utils/math_utils.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SpriteImage, scale, offset, rotation, blend_mode, color);

SpriteImage SpriteImage::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
    SpriteImage data = json.get<SpriteImage>();

    std::filesystem::path imagePath;
    json["image"].get_to(imagePath);
    data.image = context.surfaceContext.ImageFromFile(imagePath);
    if (data.image == nullptr) {
        spdlog::error("Failed loading image at {}", imagePath.string());
        throw std::runtime_error("Failed loading image " + imagePath.string());
    }

    data.rotation = canyon::Radians(data.rotation);

    return data;
}

SpriteData SpriteData::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
    SpriteData data;
    for (auto const& [imageName, imageJson] : json["images"].items()) {
        data.images.insert(std::make_pair(imageName, SpriteImage::Deserialize(imageJson, context)));
    }
    return data;
}
