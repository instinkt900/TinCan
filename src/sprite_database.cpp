#include "sprite_database.h"
#include <nlohmann/json.hpp>
#include <moth_ui/utils/vector_serialization.h>
#include <spdlog/spdlog.h>
#include <canyon/utils/vector_serialization.h>
#include <fstream>

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

bool SpriteData::Load(nlohmann::json& json, canyon::graphics::SurfaceContext& surfaceContext) {
    json["name"].get_to(name);
    for (auto const& [imageName, imageJson]: json["images"].items()) {
        SpriteImage image;
        if (image.Load(imageJson, surfaceContext)) {
            images.insert(std::make_pair(imageName, image));
        }
    }
    return true;
}

std::unique_ptr<SpriteDatabase> SpriteDatabase::Load(std::filesystem::path const& path,
                                                     canyon::graphics::SurfaceContext& surfaceContext) {
    if (!std::filesystem::exists(path)) {
        spdlog::error("Sprite database does not exist at {}", path.string());
        return nullptr;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        spdlog::error("Unable to open the sprite database at {}", path.string());
        return nullptr;
    }

    nlohmann::json json;
    try {
        file >> json;
    } catch (std::exception& e) {
        spdlog::error("Failed to load projectile database at {}", path.string());
        spdlog::error("{}", e.what());
        return nullptr;
    }

    std::unique_ptr<SpriteDatabase> db = std::unique_ptr<SpriteDatabase>(new SpriteDatabase());
    for (auto spriteJson : json) {
        SpriteData spriteData;
        if (spriteData.Load(spriteJson, surfaceContext)) {
            db->m_database.insert(std::make_pair(spriteData.name, spriteData));
        }
    }

    spdlog::info("Loaded sprite database: {}", path.string());
    return db;
}

SpriteData const* SpriteDatabase::GetSpriteData(std::string const& name) const {
    auto entry = m_database.find(name);
    if (entry == m_database.end()) {
        spdlog::error("Unable to find sprite {}", name);
        return nullptr;
    }
    return &entry->second;
}
