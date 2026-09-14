#include "system_map.h"
#include "game_world.h"
#include "system_drawable.h"
#include "system_enemy_spawner.h"
#include "system_group.h"
#include "system_movement.h"
#include "system_world_bounds.h"
#include <moth/graphics/utils/transform.h>
#include <moth/tilemap/tile_map_loader.h>
#include <moth/tilemap/tile_map_renderer.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string_view>
#include <variant>

namespace {
    // spawns trigger when they are this many pixels above the top of the screen
    float constexpr SpawnMargin = 64.0f;

    // tiled may store a float property as an int
    float GetFloatProperty(moth::tilemap::Properties const& properties, std::string_view name, float fallback) {
        if (auto const it = properties.find(std::string(name)); it != properties.end()) {
            if (auto const* floatValue = std::get_if<float>(&it->second)) {
                return *floatValue;
            }
            if (auto const* intValue = std::get_if<int>(&it->second)) {
                return static_cast<float>(*intValue);
            }
        }
        return fallback;
    }

    // offsets of each formation member from the spawn point, in map pixels. the spawn point is the front of the
    // formation and members further back are placed above it, so they scroll into view after the front
    std::vector<moth::gfx::FloatVec2> GetFormationOffsets(std::string const& formation, int count, float spacing) {
        std::vector<moth::gfx::FloatVec2> offsets;
        offsets.reserve(static_cast<size_t>(count));
        if (formation == "line") {
            for (int i = 0; i < count; ++i) {
                offsets.push_back({ (static_cast<float>(i) - static_cast<float>(count - 1) / 2.0f) * spacing, 0.0f });
            }
        } else if (formation == "column") {
            for (int i = 0; i < count; ++i) {
                offsets.push_back({ 0.0f, -static_cast<float>(i) * spacing });
            }
        } else if (formation == "v") {
            // the leader is on the point, the rest alternate stepping back to the left and right
            for (int i = 0; i < count; ++i) {
                auto const step = static_cast<float>((i + 1) / 2);
                auto const side = i % 2 == 1 ? -1.0f : 1.0f;
                offsets.push_back({ side * step * spacing, -step * spacing });
            }
        } else if (formation == "grid") {
            // rows fill front to back, each centred on the point
            auto const columns = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(count))));
            for (int i = 0; i < count; ++i) {
                auto const row = i / columns;
                auto const rowCount = std::min(columns, count - (row * columns));
                auto const column = i % columns;
                offsets.push_back({ (static_cast<float>(column) - static_cast<float>(rowCount - 1) / 2.0f) * spacing,
                                    -static_cast<float>(row) * spacing });
            }
        } else {
            spdlog::error("Map spawn has unknown formation {}", formation);
            offsets.push_back({ 0.0f, 0.0f });
        }
        return offsets;
    }

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

        auto const count = std::max(1, moth::tilemap::GetProperty(spawn.properties, "count", 1));
        auto const formation = moth::tilemap::GetProperty<std::string>(spawn.properties, "formation", "line");
        auto const spacing = GetFloatProperty(spawn.properties, "spacing", 32.0f);

        // formation members share a group so that killing the whole formation can give a drop. no drop is set yet
        auto group = moth::ecs::kNullEntity;
        if (count > 1) {
            group = SystemGroup::CreateGroup(registry, DataRef<PickupData>{});
        }

        moth::gfx::FloatRect boundsRect{ { 0.0f, 0.0f }, static_cast<moth::gfx::FloatVec2>(world.GetWorldSize()) };
        boundsRect.topLeft -= { SpawnMargin * 2.0f, SpawnMargin * 2.0f };
        boundsRect.bottomRight += { SpawnMargin * 2.0f, SpawnMargin * 2.0f };

        for (auto const& offset : GetFormationOffsets(formation, count, spacing)) {
            moth::gfx::FloatVec2 const position{ spawn.position.x + offset.x,
                                                 spawn.position.y - scroll.m_cameraTop + offset.y };
            auto enemy = SystemEnemySpawner::SpawnEnemy(registry, *enemyData, behaviour, position, gamedata);

            if (behaviourData == nullptr) {
                // move with the map so the enemy stays where it was placed
                registry.Get<ComponentVelocity>(enemy).m_velocity = { 0.0f, scroll.m_scrollSpeed };
            }

            // members behind the front spawn further above the screen, so their bounds reach up that far too
            auto& bounds = registry.Emplace<ComponentBounds>(enemy);
            bounds.m_bounds = boundsRect;
            bounds.m_bounds.topLeft.y += offset.y;
            bounds.m_behaviour = BoundsBehaviour::Kill;

            if (group != moth::ecs::kNullEntity) {
                SystemGroup::AddMember(registry, group, enemy);
            }
        }
    }
}

void SystemMap::InitMap(moth::ecs::World& registry, std::filesystem::path const& path,
                        std::filesystem::path const& projectPath, moth::gfx::AssetContext& assetContext,
                        moth::gfx::IntVec2 const& viewSize) {
    auto map = moth::tilemap::LoadTileMapFromFile(path, moth::tilemap::LoadPropertyTypes(projectPath));

    // tileset and image layer image paths are relative to the map file
    std::map<std::string, moth::gfx::Image> images;
    auto const loadImage = [&](std::string const& relativePath) {
        if (relativePath.empty() || images.count(relativePath) != 0) {
            return;
        }
        auto const imagePath = path.parent_path() / relativePath;
        std::shared_ptr<moth::gfx::ITexture> texture = assetContext.TextureFromFile(imagePath);
        if (texture == nullptr) {
            spdlog::error("Failed loading map image at {}", imagePath.string());
            throw std::runtime_error("Failed loading map image " + imagePath.string());
        }
        images.emplace(relativePath, moth::gfx::Image(std::move(texture)));
    };
    for (auto const& tileset : map.tilesets) {
        loadImage(tileset.imagePath);
    }
    for (auto const& imageLayer : map.imageLayers) {
        loadImage(imageLayer.imagePath);
    }

    auto mapEntity = registry.Create();
    auto& scroll = registry.Emplace<ComponentMapScroll>(mapEntity);
    scroll.m_map = std::move(map);
    scroll.m_images = std::move(images);
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
        // the view centre drives the layer parallax
        moth::gfx::FloatVec2 const cameraCentre{ viewSize.x / 2.0f, top + (viewSize.y / 2.0f) };
        auto const& images = scroll.m_images;
        auto const resolveImage = [&images](std::string const& imagePath) {
            auto const it = images.find(imagePath);
            return it != images.end() ? it->second : moth::gfx::Image{};
        };
        graphics.PushTransform(moth::gfx::FloatMat4x4::Translation({ shake.x, shake.y - top }));
        moth::tilemap::DrawTileMap(graphics, scroll.m_map, resolveImage, viewRect, 0, cameraCentre);
        graphics.PopTransform();
    }
}
