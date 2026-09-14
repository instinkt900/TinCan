#include "system_map.h"
#include "game_world.h"
#include "system_drawable.h"
#include "system_enemy_spawner.h"
#include "system_movement.h"
#include "system_world_bounds.h"
#include <moth/graphics/utils/transform.h>
#include <moth/tilemap/tile_map_loader.h>
#include <moth/tilemap/tile_map_renderer.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <stdexcept>
#include <variant>

namespace {
    // spawns trigger when they are this many pixels above the top of the screen
    float constexpr SpawnMargin = 64.0f;

    void SpawnMapEnemy(GameWorld& world, moth::tilemap::MapObject const& spawn, ComponentMapScroll const& scroll) {
        auto& registry = world.GetRegistry();
        auto const& gamedata = world.GetGameData();
        auto const enemyType = moth::tilemap::GetProperty<std::string>(spawn.properties, "enemy_type");
        if (enemyType.empty()) {
            spdlog::error("Map spawn is missing an enemy_type");
            return;
        }
        auto const* enemyData = gamedata.Get<EnemyData>(enemyType);
        if (enemyData == nullptr) {
            spdlog::error("Map spawn references unknown enemy {}", enemyType);
            return;
        }

        BehaviourData behaviour;
        BehaviourData const* behaviourData = nullptr;
        auto const behaviourName = moth::tilemap::GetProperty<std::string>(spawn.properties, "behaviour");
        if (!behaviourName.empty()) {
            behaviourData = gamedata.Get<BehaviourData>(behaviourName);
            if (behaviourData == nullptr) {
                spdlog::error("Map spawn references unknown behaviour {}", behaviourName);
            } else {
                behaviour = *behaviourData;
            }
        }

        // a speed property overrides the speed of every state in the behaviour. tiled may store it as an int or a float
        if (auto const it = spawn.properties.find("speed"); it != spawn.properties.end()) {
            auto const* floatSpeed = std::get_if<float>(&it->second);
            auto const* intSpeed = std::get_if<int>(&it->second);
            if (floatSpeed != nullptr || intSpeed != nullptr) {
                auto const speed = floatSpeed != nullptr ? *floatSpeed : static_cast<float>(*intSpeed);
                for (auto& state : behaviour.states) {
                    state.speed = speed;
                }
            }
        }

        moth::gfx::FloatVec2 const position{ spawn.position.x, spawn.position.y - scroll.m_cameraTop };
        auto enemy = SystemEnemySpawner::SpawnEnemy(registry, *enemyData, behaviour, position, gamedata);

        if (behaviourData == nullptr) {
            // move with the map so the enemy stays where it was placed
            registry.Get<ComponentVelocity>(enemy).m_velocity = { 0.0f, scroll.m_scrollSpeed };
        }

        moth::gfx::FloatRect boundsRect{ { 0.0f, 0.0f }, static_cast<moth::gfx::FloatVec2>(world.GetWorldSize()) };
        boundsRect.topLeft -= { SpawnMargin * 2.0f, SpawnMargin * 2.0f };
        boundsRect.bottomRight += { SpawnMargin * 2.0f, SpawnMargin * 2.0f };
        auto& bounds = registry.Emplace<ComponentBounds>(enemy);
        bounds.m_bounds = boundsRect;
        bounds.m_behaviour = BoundsBehaviour::Kill;
    }
}

void SystemMap::InitMap(moth::ecs::World& registry, std::filesystem::path const& path,
                        std::filesystem::path const& projectPath, moth::gfx::AssetContext& assetContext,
                        moth::gfx::IntVec2 const& viewSize) {
    auto map = moth::tilemap::LoadTileMapFromFile(path, moth::tilemap::LoadPropertyTypes(projectPath));

    // tileset image paths are relative to the map file
    std::vector<moth::gfx::Image> tilesetImages;
    for (auto const& tileset : map.tilesets) {
        auto const imagePath = path.parent_path() / tileset.imagePath;
        std::shared_ptr<moth::gfx::ITexture> texture = assetContext.TextureFromFile(imagePath);
        if (texture == nullptr) {
            spdlog::error("Failed loading tileset image at {}", imagePath.string());
            throw std::runtime_error("Failed loading tileset image " + imagePath.string());
        }
        tilesetImages.push_back(moth::gfx::Image(std::move(texture)));
    }

    auto mapEntity = registry.Create();
    auto& scroll = registry.Emplace<ComponentMapScroll>(mapEntity);
    scroll.m_map = std::move(map);
    scroll.m_tilesetImages = std::move(tilesetImages);
    scroll.m_scrollSpeed = moth::tilemap::GetProperty(scroll.m_map.properties, "scroll_speed", scroll.m_scrollSpeed);

    // start with the bottom of the map on screen and scroll upwards
    auto const mapHeight = static_cast<float>(scroll.m_map.height * scroll.m_map.tileHeight);
    scroll.m_cameraTop = std::max(0.0f, mapHeight - static_cast<float>(viewSize.y));

    for (auto const& objectLayer : scroll.m_map.objectLayers) {
        for (auto const& object : objectLayer.objects) {
            if (object.kind == moth::tilemap::ObjectKind::Point && object.type == "EnemySpawn") {
                scroll.m_pendingSpawns.push_back(object);
            }
        }
    }
    std::sort(scroll.m_pendingSpawns.begin(), scroll.m_pendingSpawns.end(),
              [](moth::tilemap::MapObject const& a, moth::tilemap::MapObject const& b) {
                  return a.position.y < b.position.y;
              });

    registry.Emplace<ComponentCamera>(mapEntity);
}

void SystemMap::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    auto const dt = static_cast<float>(ticks) / 1000.0f;
    auto view = registry.View<ComponentMapScroll>();
    for (auto [entity, scroll] : view.each()) {
        scroll.m_cameraTop = std::max(0.0f, scroll.m_cameraTop - scroll.m_scrollSpeed * dt);

        auto& spawns = scroll.m_pendingSpawns;
        while (!spawns.empty() && spawns.back().position.y >= scroll.m_cameraTop - SpawnMargin) {
            SpawnMapEnemy(world, spawns.back(), scroll);
            spawns.pop_back();
        }
    }
}

void SystemMap::Draw(GameWorld& world, moth::gfx::IGraphics& graphics) {
    auto& registry = world.GetRegistry();
    auto const viewSize = static_cast<moth::gfx::FloatVec2>(world.GetWorldSize());
    auto view = registry.View<ComponentMapScroll, ComponentCamera>();

    graphics.SetBlendMode(moth::gfx::BlendMode::Alpha);
    graphics.SetColor(moth::gfx::BasicColors::White);

    for (auto [entity, scroll, camera] : view.each()) {
        // snapped to whole pixels so the tiles don't shimmer while scrolling
        auto const shake = static_cast<moth::gfx::FloatVec2>(static_cast<moth::gfx::IntVec2>(camera.m_shakeOffset));
        auto const top = static_cast<float>(static_cast<int32_t>(scroll.m_cameraTop));

        // the renderer draws in map pixels, so shift the map to put the camera top at the top of the screen
        moth::gfx::FloatRect const viewRect{ { 0.0f, top }, { viewSize.x, top + viewSize.y } };
        graphics.PushTransform(moth::gfx::FloatMat4x4::Translation({ shake.x, shake.y - top }));
        moth::tilemap::DrawTileMap(graphics, scroll.m_map, scroll.m_tilesetImages, viewRect);
        graphics.PopTransform();
    }
}
