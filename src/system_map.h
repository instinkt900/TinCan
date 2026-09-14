#pragma once

#include <moth/ecs/world.h>
#include <moth/graphics/graphics/asset_context.h>
#include <moth/graphics/graphics/igraphics.h>
#include <moth/graphics/graphics/image.h>
#include <moth/tilemap/tile_map.h>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

class GameWorld;

struct ComponentMapScroll {
    moth::tilemap::TileMap m_map;
    std::map<std::string, moth::gfx::Image> m_images; // tileset and image layer images, keyed by path relative to the map
    float m_scrollSpeed = 60.0f; // pixels per second, from the map's "scroll_speed" property
    float m_cameraTop = 0; // map pixel row currently at the top of the screen
    std::vector<moth::tilemap::MapObject> m_pendingSpawns; // ascending y, so the next to scroll into view is at the back
};

class SystemMap {
public:
    // loads a tiled json map (.tmj), with class defaults from its tiled project, and creates the map entity
    // along with the camera
    static void InitMap(moth::ecs::World& registry, std::filesystem::path const& path,
                        std::filesystem::path const& projectPath, moth::gfx::AssetContext& assetContext,
                        moth::gfx::IntVec2 const& viewSize);
    static void Update(GameWorld& world, uint32_t ticks);
    static void Draw(GameWorld& world, moth::gfx::IGraphics& graphics);
};
