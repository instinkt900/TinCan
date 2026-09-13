#include "system_map.h"
#include "game_world.h"
#include "system_drawable.h"
#include <algorithm>

namespace {
    // tiled stores flip/rotation flags in the top bits of each gid
    uint32_t constexpr GidMask = 0x1FFFFFFF;

    TiledTileset const* FindTileset(TiledMap const& map, uint32_t gid) {
        TiledTileset const* found = nullptr;
        for (auto const& tileset : map.tilesets) {
            if (tileset.firstGid <= gid) {
                found = &tileset;
            }
        }
        return found;
    }
}

void SystemMap::InitMap(moth::ecs::World& registry, TiledMap map, moth::gfx::IntVec2 const& viewSize) {
    auto mapEntity = registry.Create();
    auto& scroll = registry.Emplace<ComponentMapScroll>(mapEntity);
    scroll.m_map = std::move(map);

    // start with the bottom of the map on screen and scroll upwards
    auto const mapHeight = static_cast<float>(scroll.m_map.height * scroll.m_map.tileHeight);
    scroll.m_cameraTop = std::max(0.0f, mapHeight - static_cast<float>(viewSize.y));

    registry.Emplace<ComponentCamera>(mapEntity);
}

void SystemMap::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    auto const dt = static_cast<float>(ticks) / 1000.0f;
    auto view = registry.View<ComponentMapScroll>();
    for (auto [entity, scroll] : view.each()) {
        scroll.m_cameraTop = std::max(0.0f, scroll.m_cameraTop - scroll.m_map.scrollSpeed * dt);
    }
}

void SystemMap::Draw(GameWorld& world, moth::gfx::IGraphics& graphics) {
    auto& registry = world.GetRegistry();
    auto const& viewSize = world.GetWorldSize();
    auto view = registry.View<ComponentMapScroll, ComponentCamera>();

    graphics.SetBlendMode(moth::gfx::BlendMode::Alpha);
    graphics.SetColor(moth::gfx::BasicColors::White);

    for (auto [entity, scroll, camera] : view.each()) {
        auto const& map = scroll.m_map;
        auto const shake = static_cast<moth::gfx::IntVec2>(camera.m_shakeOffset);
        auto const top = static_cast<int32_t>(scroll.m_cameraTop);
        auto const firstRow = std::max(0, top / map.tileHeight);
        auto const lastRow = std::min(map.height - 1, (top + viewSize.y) / map.tileHeight);

        for (auto const& layer : map.layers) {
            if (!layer.visible) {
                continue;
            }
            for (int32_t row = firstRow; row <= lastRow; ++row) {
                for (int32_t col = 0; col < map.width; ++col) {
                    auto const gid = layer.gids[static_cast<size_t>(row * map.width + col)] & GidMask;
                    if (gid == 0) {
                        continue;
                    }
                    auto const* tileset = FindTileset(map, gid);
                    if (tileset == nullptr) {
                        continue;
                    }

                    auto const localId = static_cast<int32_t>(gid - tileset->firstGid);
                    auto const srcX = tileset->margin + (localId % tileset->columns) * (tileset->tileWidth + tileset->spacing);
                    auto const srcY = tileset->margin + (localId / tileset->columns) * (tileset->tileHeight + tileset->spacing);
                    moth::gfx::IntRect const srcRect{ { srcX, srcY },
                                                      { srcX + tileset->tileWidth, srcY + tileset->tileHeight } };

                    auto const destX = col * map.tileWidth + shake.x;
                    auto const destY = row * map.tileHeight - top + shake.y;
                    moth::gfx::IntRect const destRect{ { destX, destY },
                                                       { destX + tileset->tileWidth, destY + tileset->tileHeight } };

                    graphics.DrawImage(tileset->image, destRect, &srcRect);
                }
            }
        }
    }
}
