#include "system_map.h"
#include "behaviour_transform.h"
#include "formation_offsets.h"
#include "game_world.h"
#include "system_drawable.h"
#include "system_enemy_spawner.h"
#include "system_group.h"
#include "system_movement.h"
#include "system_pickup.h"
#include "system_world_bounds.h"
#include <moth/graphics/utils/transform.h>
#include <moth/profile/profiler.h>
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

    void SpawnMapEnemy(GameWorld& world, moth::tilemap::MapObject const& spawn, ComponentMapScroll const& scroll) {
        MOTH_PROFILE_SCOPE("SpawnMapEnemy");
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
        BehaviourOverride spawnOverride;
        float rotationStep = 0.0f;
        int count = 1;
        std::string formation;
        float spacing = 32.0f;
        moth::gfx::FloatVec2 scale{ 1.0f, 1.0f };
        DataRef<PickupData> drop;
        {
            // reading the spawn's tiled properties: string lookups, a behaviour copy and a data ref load
            MOTH_PROFILE_SCOPE("Spawn properties");
            auto const behaviourName = moth::tilemap::GetProperty<std::string>(spawn.properties, "behaviour");
            if (!behaviourName.empty()) {
                behaviourData = gamedata.Get<BehaviourData>(behaviourName);
                if (behaviourData == nullptr) {
                    spdlog::error("Map spawn references unknown behaviour {}", behaviourName);
                } else {
                    behaviour = *behaviourData;
                }
            }

            // what this spawn changes about the behaviour it uses. tiled may store any of these as an int
            // or a float, and a property that is not set leaves the behaviour as it was authored
            if (auto const it = spawn.properties.find("speed"); it != spawn.properties.end()) {
                auto const* floatSpeed = std::get_if<float>(&it->second);
                auto const* intSpeed = std::get_if<int>(&it->second);
                if (floatSpeed != nullptr || intSpeed != nullptr) {
                    spawnOverride.speed =
                        floatSpeed != nullptr ? *floatSpeed : static_cast<float>(*intSpeed);
                }
            }
            spawnOverride.rotation = GetFloatProperty(spawn.properties, "behaviour_rotation", 0.0f);
            rotationStep = GetFloatProperty(spawn.properties, "behaviour_rotation_step", 0.0f);

            count = std::max(1, moth::tilemap::GetProperty(spawn.properties, "count", 1));
            formation = moth::tilemap::GetProperty<std::string>(spawn.properties, "formation", "line");
            spacing = GetFloatProperty(spawn.properties, "spacing", 32.0f);
            // a formation from the game data is a shape, sized by scale rather than by spacing
            scale = { GetFloatProperty(spawn.properties, "scale_x", 1.0f),
                      GetFloatProperty(spawn.properties, "scale_y", 1.0f) };

            // "none" is the enum's way of saying no drop, since tiled cannot clear an enum property
            auto const dropName = moth::tilemap::GetProperty<std::string>(spawn.properties, "drop");
            if (!dropName.empty() && dropName != "none") {
                if (gamedata.Get<PickupData>(dropName) == nullptr) {
                    spdlog::error("Map spawn references unknown pickup {}", dropName);
                } else {
                    drop.load_ref(dropName, gamedata);
                }
            }
        }

        // the spawn's members share a group, so the drop appears once the whole formation has been killed
        auto group = moth::ecs::kNullEntity;
        if (count > 1 || drop.valid()) {
            group = SystemGroup::CreateGroup(registry, drop);
        }

        moth::gfx::FloatRect boundsRect{ { 0.0f, 0.0f }, static_cast<moth::gfx::FloatVec2>(world.GetWorldSize()) };
        boundsRect.topLeft -= { SpawnMargin * 2.0f, SpawnMargin * 2.0f };
        boundsRect.bottomRight += { SpawnMargin * 2.0f, SpawnMargin * 2.0f };

        uint32_t phaseIndex = 0;
        auto const offsets = GetFormationOffsets(gamedata, formation, count, spacing, scale, spawn.rotation);
        MOTH_PROFILE_SCOPE("Spawn formation");
        for (auto const& offset : offsets) {
            moth::gfx::FloatVec2 const position{ spawn.position.x + offset.x,
                                                 spawn.position.y - scroll.m_cameraTop + offset.y };
            // each member gets its own copy of the behaviour, because the rotation steps along the
            // formation so that a group fans out
            auto memberBehaviour = behaviour;
            auto memberOverride = spawnOverride;
            memberOverride.rotation += rotationStep * static_cast<float>(phaseIndex);
            ApplyBehaviourOverride(memberBehaviour, memberOverride);

            auto enemy =
                SystemEnemySpawner::SpawnEnemy(registry, *enemyData, memberBehaviour, position, gamedata);

            // a Phase modifier staggers the formation by each member's place in it
            registry.Get<ComponentBehaviour>(enemy).m_phaseIndex = phaseIndex++;

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

    void SpawnMapPickup(GameWorld& world, moth::tilemap::MapObject const& spawn, ComponentMapScroll const& scroll) {
        MOTH_PROFILE_SCOPE("SpawnMapPickup");
        auto& registry = world.GetRegistry();
        auto const& gamedata = world.GetGameData();
        auto const pickupName = moth::tilemap::GetProperty<std::string>(spawn.properties, "pickup");
        if (pickupName.empty()) {
            spdlog::error("Map pickup spawn is missing a pickup");
            return;
        }
        auto const* pickupData = gamedata.Get<PickupData>(pickupName);
        if (pickupData == nullptr) {
            spdlog::error("Map pickup spawn references unknown pickup {}", pickupName);
            return;
        }

        moth::gfx::FloatVec2 const position{ spawn.position.x, spawn.position.y - scroll.m_cameraTop };
        SystemPickup::CreatePickup(registry, position, *pickupData, gamedata);
    }
}

moth::gfx::IntVec2 SystemMap::InitMap(moth::ecs::World& registry, std::filesystem::path const& path,
                                      std::filesystem::path const& projectPath,
                                      moth::gfx::AssetContext& assetContext, int viewHeight) {
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
        // an image collection tileset has no atlas image of its own: each tile carries its own,
        // which is how loose decoration images placed as tile objects reach the renderer
        loadImage(tileset.imagePath);
        for (auto const& tileImage : tileset.tileImages) {
            loadImage(tileImage.second.imagePath);
        }
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
    scroll.m_cameraTop = std::max(0.0f, mapHeight - static_cast<float>(viewHeight));

    for (auto const& objectLayer : scroll.m_map.objectLayers) {
        for (auto const& object : objectLayer.objects) {
            if (object.kind == moth::tilemap::ObjectKind::Point &&
                (object.type == "EnemySpawn" || object.type == "PickupSpawn")) {
                scroll.m_pendingSpawns.push_back(object);
            }
        }
    }
    std::sort(scroll.m_pendingSpawns.begin(), scroll.m_pendingSpawns.end(),
              [](moth::tilemap::MapObject const& a, moth::tilemap::MapObject const& b) {
                  return a.position.y < b.position.y;
              });

    registry.Emplace<ComponentCamera>(mapEntity);

    return { scroll.m_map.width * scroll.m_map.tileWidth, viewHeight };
}

void SystemMap::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    auto const dt = static_cast<float>(ticks) / 1000.0f;
    auto view = registry.View<ComponentMapScroll>();
    for (auto [entity, scroll] : view.each()) {
        scroll.m_cameraTop = std::max(0.0f, scroll.m_cameraTop - scroll.m_scrollSpeed * dt);

        auto& spawns = scroll.m_pendingSpawns;
        auto const spawnLine = scroll.m_cameraTop - SpawnMargin;
        auto const spawnDue = [&]() { return !spawns.empty() && spawns.back().position.y >= spawnLine; };
        // only timed on the frames that actually spawn, so the profiler shows the spike and not a row of zeroes
        if (!spawnDue()) {
            continue;
        }
        MOTH_PROFILE_SCOPE("Map spawns");
        while (spawnDue()) {
            if (spawns.back().type == "PickupSpawn") {
                SpawnMapPickup(world, spawns.back(), scroll);
            } else {
                SpawnMapEnemy(world, spawns.back(), scroll);
            }
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
        {
            MOTH_PROFILE_SCOPE("DrawTileMap");
            moth::tilemap::DrawTileMap(graphics, scroll.m_map, resolveImage, viewRect, 0, cameraCentre);
        }
        graphics.PopTransform();
    }
}
