#include "game_world.h"
#include "component_passives.h"
#include "system_drawable.h"
#include "system_enemy_spawner.h"
#include "system_group.h"
#include "system_health.h"
#include "system_input.h"
#include "system_level.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include "system_pickup.h"
#include "system_player_visuals.h"
#include "system_projectile.h"
#include "system_shield.h"
#include "system_weapon.h"
#include "system_world_bounds.h"
#include "tags.h"

canyon::IntVec2 const GameWorld::WorldSize{ 720, 1280 };

GameWorld::GameWorld(canyon::platform::Window& window, canyon::graphics::IGraphics& graphics)
    : m_window(window)
    , m_graphics(graphics) {
    m_worldSurface = m_graphics.CreateTarget(WorldSize.x, WorldSize.y);

    SerializeContext serializeContext{ m_gamedata, m_window.GetSurfaceContext() };
    m_gamedata.GetSpriteDatabase().Load("data/sprite_database.json", serializeContext);
    m_gamedata.GetSpriteDatabase().Load("data/sprite_database_enemy.json", serializeContext);
    m_gamedata.GetEnemyDatabase().Load("data/enemy_database.json", serializeContext);
    m_gamedata.GetWeaponDatabase().Load("data/weapon_database_player.json", serializeContext);
    m_gamedata.GetWeaponDatabase().Load("data/weapon_database_enemy.json", serializeContext);
    m_gamedata.GetProjectileDatabase().Load("data/projectile_database.json", serializeContext);
    m_gamedata.GetSpawnerDatabase().Load("data/spawner_database.json", serializeContext);
    m_gamedata.GetLevelDatabase().Load("data/level_database.json", serializeContext);
    m_gamedata.GetPickupDatabase().Load("data/pickup_database.json", serializeContext);

    SystemLevel::InitLevel(m_registry, "test", m_gamedata);
    CreatePlayer();
}

bool GameWorld::OnEvent(moth_ui::Event const& event) {
    moth_ui::EventDispatch dispatch(event);
    dispatch.Dispatch(this, &GameWorld::OnKeyEvent);
    bool handled = dispatch.GetHandled();
    return handled;
}

void GameWorld::Update(uint32_t ticks) {
    SystemLevel::Update(m_registry, ticks, m_gamedata);
    SystemLifetime::Update(m_registry, ticks);
    SystemEnemySpawner::Update(m_registry, ticks, m_gamedata);
    SystemBehaviour::Update(m_registry, ticks);
    SystemMovement::Update(m_registry, ticks);
    SystemWeapon::Update(m_registry, ticks, m_gamedata);
    SystemShield::Update(m_registry, ticks);
    SystemProjectile::Update(m_registry, ticks, m_gamedata);
    SystemPickup::Update(m_registry, ticks, m_gamedata);
    SystemPlayerVisuals::Update(m_registry, ticks);
    SystemGroup::Update(m_registry, ticks, m_gamedata);
    SystemWorldBounds::Update(
        m_registry, ticks, canyon::FloatRect{ { 0.0f, 0.0f }, static_cast<canyon::FloatVec2>(WorldSize) });
}

void GameWorld::Draw() {
    m_graphics.SetTarget(m_worldSurface.get());
    m_graphics.SetBlendMode(canyon::graphics::BlendMode::Replace);
    m_graphics.SetColor(canyon::graphics::BasicColors::White);
    SystemDrawable::Update(m_registry, m_graphics);
    m_graphics.SetTarget(nullptr);
}

float GameWorld::GetPlayerHealth() const {
    if (auto const* playerHealth = m_registry.try_get<ComponentHealth>(m_player)) {
        return playerHealth->m_currentHealth;
    }
    return 0.0f;
}

bool GameWorld::OnKeyEvent(moth_ui::EventKey const& event) {
    SystemInput::OnKey(m_registry, event);
    return false;
}

void GameWorld::CreatePlayer() {
    m_player = m_registry.create();
    m_registry.emplace<PlayerTag>(m_player);

    auto& entityData = m_registry.emplace<ComponentEntity>(m_player);
    entityData.m_team = Team::PLAYER;
    entityData.m_color = EnergyColor::WHITE;
    entityData.m_radius = 5.0f;
    entityData.m_angle = 0;

    auto& playerHealth = m_registry.emplace<ComponentHealth>(m_player);
    playerHealth.m_maxHealth = 100;
    playerHealth.m_currentHealth = 100;

    auto& position = m_registry.emplace<ComponentPosition>(m_player);
    position.m_position = { WorldSize.x / 2, WorldSize.y - 160 };

    auto& velocity = m_registry.emplace<ComponentVelocity>(m_player);
    velocity.m_velocity = { 0, 0 };

    m_registry.emplace<ComponentInput>(m_player);

    auto& drawable = m_registry.emplace<ComponentDrawable>(m_player);

    auto const* playerSprite = m_gamedata.GetSpriteDatabase().Get("player_ship");
    if (playerSprite != nullptr) {
        drawable.m_spriteData = *playerSprite;
    }

    auto& shield = m_registry.emplace<ComponentShield>(m_player);
    shield.m_radius = 56;

    auto& power = m_registry.emplace<ComponentPower>(m_player);
    power.m_power = 0;

    m_registry.emplace<ComponentPassives>(m_player);

    SystemWeapon::InitWeapon(m_registry, m_player, "player_weapon_01", m_gamedata);

    m_registry.emplace<TargetTag>(m_player);
    m_registry.emplace<BoundedTag>(m_player);
}
