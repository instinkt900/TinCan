#pragma once

#include <moth/graphics/graphics/asset_context.h>
#include <moth/graphics/graphics/image.h>
#include <filesystem>
#include <string>
#include <vector>

struct TiledTileset {
    uint32_t firstGid = 1;
    int32_t tileWidth = 0;
    int32_t tileHeight = 0;
    int32_t columns = 1;
    int32_t margin = 0;
    int32_t spacing = 0;
    moth::gfx::Image image;
};

struct TiledTileLayer {
    std::string name;
    bool visible = true;
    std::vector<uint32_t> gids; // width * height, row by row
};

struct TiledMap {
    int32_t width = 0; // in tiles
    int32_t height = 0;
    int32_t tileWidth = 0;
    int32_t tileHeight = 0;
    float scrollSpeed = 60.0f; // pixels per second, from the map's "scroll_speed" property
    std::vector<TiledTileset> tilesets; // ascending firstGid, as written by tiled
    std::vector<TiledTileLayer> layers;
};

// loads a tiled json map (.tmj). tilesets must be embedded or external json (.tsj)
// and tile layers must use the CSV layer format.
TiledMap LoadTiledMap(std::filesystem::path const& path, moth::gfx::AssetContext& assetContext);
