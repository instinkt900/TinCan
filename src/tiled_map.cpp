#include "tiled_map.h"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <fstream>
#include <stdexcept>

namespace {
    nlohmann::json LoadJson(std::filesystem::path const& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            spdlog::error("Unable to open {}", path.string());
            throw std::runtime_error("Unable to open " + path.string());
        }
        return nlohmann::json::parse(file);
    }

    // paths inside tiled files are relative to the file they appear in
    TiledTileset LoadTileset(nlohmann::json tilesetJson, std::filesystem::path baseDir,
                             moth::gfx::AssetContext& assetContext) {
        TiledTileset tileset;
        tileset.firstGid = tilesetJson.at("firstgid").get<uint32_t>();

        if (tilesetJson.contains("source")) {
            auto const sourcePath = baseDir / tilesetJson.at("source").get<std::string>();
            if (sourcePath.extension() != ".tsj" && sourcePath.extension() != ".json") {
                spdlog::error("Tileset {} is not json. Export it from tiled as .tsj", sourcePath.string());
                throw std::runtime_error("Unsupported tileset format " + sourcePath.string());
            }
            tilesetJson = LoadJson(sourcePath);
            baseDir = sourcePath.parent_path();
        }

        tileset.tileWidth = tilesetJson.at("tilewidth").get<int32_t>();
        tileset.tileHeight = tilesetJson.at("tileheight").get<int32_t>();
        tileset.columns = tilesetJson.at("columns").get<int32_t>();
        tileset.margin = tilesetJson.value("margin", 0);
        tileset.spacing = tilesetJson.value("spacing", 0);

        auto const imagePath = baseDir / tilesetJson.at("image").get<std::string>();
        std::shared_ptr<moth::gfx::ITexture> texture = assetContext.TextureFromFile(imagePath);
        if (texture == nullptr) {
            spdlog::error("Failed loading tileset image at {}", imagePath.string());
            throw std::runtime_error("Failed loading tileset image " + imagePath.string());
        }
        tileset.image = moth::gfx::Image(std::move(texture));

        return tileset;
    }
}

TiledMap LoadTiledMap(std::filesystem::path const& path, moth::gfx::AssetContext& assetContext) {
    auto const json = LoadJson(path);
    auto const baseDir = path.parent_path();

    if (json.value("infinite", false)) {
        spdlog::error("Map {} is infinite, which is not supported", path.string());
        throw std::runtime_error("Infinite maps are not supported " + path.string());
    }

    TiledMap map;
    map.width = json.at("width").get<int32_t>();
    map.height = json.at("height").get<int32_t>();
    map.tileWidth = json.at("tilewidth").get<int32_t>();
    map.tileHeight = json.at("tileheight").get<int32_t>();

    if (json.contains("properties")) {
        for (auto const& property : json.at("properties")) {
            if (property.at("name") == "scroll_speed") {
                map.scrollSpeed = property.at("value").get<float>();
            }
        }
    }

    for (auto const& tilesetJson : json.at("tilesets")) {
        map.tilesets.push_back(LoadTileset(tilesetJson, baseDir, assetContext));
    }

    for (auto const& layerJson : json.at("layers")) {
        // object, image and group layers are ignored for now
        if (layerJson.at("type") != "tilelayer") {
            continue;
        }
        if (layerJson.contains("encoding") && layerJson.at("encoding") != "csv") {
            spdlog::error("Layer in {} is compressed. Set the tile layer format to CSV in tiled", path.string());
            throw std::runtime_error("Unsupported layer encoding in " + path.string());
        }

        TiledTileLayer layer;
        layer.name = layerJson.value("name", "");
        layer.visible = layerJson.value("visible", true);
        layer.gids = layerJson.at("data").get<std::vector<uint32_t>>();
        if (layer.gids.size() != static_cast<size_t>(map.width) * static_cast<size_t>(map.height)) {
            spdlog::error("Layer {} in {} does not match the map size", layer.name, path.string());
            throw std::runtime_error("Layer size mismatch in " + path.string());
        }
        map.layers.push_back(std::move(layer));
    }

    return map;
}
