#include "gamedata_sprite.h"
#include <nlohmann/json.hpp>
#include <moth_ui/utils/vector_serialization.h>
#include <spdlog/spdlog.h>
#include <canyon/utils/vector_serialization.h>

bool SpriteImage::Load(nlohmann::json& json, canyon::graphics::SurfaceContext& surfaceContext) {
    std::filesystem::path imagePath;
    json["image"].get_to(imagePath);
    image = surfaceContext.ImageFromFile(imagePath);
    if (image == nullptr) {
        spdlog::error("Failed loading image at {}", imagePath.string());
        return false;
    }
    json["scale"].get_to(scale);
    json["offset"].get_to(offset);
    json["rotation"].get_to(rotation);
    json["blend_mode"].get_to(blend_mode);
    json["color"].get_to(color);
    return true;
}

bool SpriteData::Load(nlohmann::json& json, Gamedata const& gamedata, canyon::graphics::SurfaceContext& surfaceContext) {
    json["name"].get_to(name);
    for (auto const& [imageName, imageJson]: json["images"].items()) {
        SpriteImage image;
        if (image.Load(imageJson, surfaceContext)) {
            images.insert(std::make_pair(imageName, image));
        }
    }
    return true;
}

