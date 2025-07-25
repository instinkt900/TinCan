#include "game_world.h"
#include "component_entity.h"
#include "component_passives.h"
#include "system_animation.h"
#include "system_drawable.h"
#include "system_enemy_spawner.h"
#include "system_group.h"
#include "component_health.h"
#include "system_input.h"
#include "system_level.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include "system_pickup.h"
#include "system_player_visuals.h"
#include "system_projectile.h"
#include "system_shield.h"
#include "system_targeting.h"
#include "system_weapon.h"
#include "system_world_bounds.h"
#include "tags.h"
#include "system_power_weapon.h"

canyon::IntVec2 const GameWorld::WorldSize{ 600, 800 };

GameWorld::GameWorld(canyon::platform::Window& window, canyon::graphics::IGraphics& graphics)
    : m_window(window)
    , m_graphics(graphics) {
    m_worldSurface = m_graphics.CreateTarget(WorldSize.x, WorldSize.y);

    SerializeContext serializeContext{ m_gamedata, m_window.GetSurfaceContext() };
    m_gamedata.LoadDirectory("data", serializeContext);

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
    SystemLevel::Update(*this, ticks);
    SystemLifetime::Update(*this, ticks);
    SystemEnemySpawner::Update(*this, ticks);
    SystemBehaviour::Update(*this, ticks);
    SystemTargeting::Update(*this, ticks);
    SystemMovement::Update(*this, ticks);
    SystemWeapon::Update(*this, ticks);
    SystemPowerWeapon::Update(*this, ticks);
    SystemShield::Update(*this, ticks);
    SystemProjectile::Update(*this, ticks);
    SystemPickup::Update(*this, ticks);
    SystemPlayerVisuals::Update(*this, ticks);
    SystemGroup::Update(*this, ticks);
    SystemWorldBounds::Update(*this, ticks);
    SystemAnimation::Update(*this, ticks);
    SystemDrawable::Update(*this, ticks);
}

void GameWorld::Draw() {
    m_graphics.SetTarget(m_worldSurface.get());
    m_graphics.SetBlendMode(canyon::graphics::BlendMode::Replace);
    m_graphics.SetColor(canyon::graphics::BasicColors::White);
    SystemDrawable::Draw(*this, m_graphics);
    m_graphics.SetTarget(nullptr);
}

float GameWorld::GetPlayerHealth() const {
    if (auto const* playerHealth = m_registry.try_get<ComponentHealth>(m_player)) {
        return playerHealth->m_currentHealth;
    }
    return 0.0f;
}

float GameWorld::GetPlayerPower() const {
    if (auto const* playerPower = m_registry.try_get<ComponentPower>(m_player)) {
        return playerPower->m_power;
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
    entityData.m_color = EnergyColor::Blue;
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

    auto const* playerSprite = m_gamedata.Get<SpriteData>("player_ship");
    if (playerSprite != nullptr) {
        auto& drawable = m_registry.emplace<ComponentDrawable>(m_player, *playerSprite);
        drawable.m_sprites.at("ship").m_fps = 18;
    }

    auto& shield = m_registry.emplace<ComponentShield>(m_player);
    shield.m_radius = 39;

    auto& power = m_registry.emplace<ComponentPower>(m_player);
    power.m_power = 0;

    m_registry.emplace<ComponentPassives>(m_player);

    auto const* weaponData = m_gamedata.Get<WeaponData>("player_weapon_01");
    SystemWeapon::InitWeapon(m_registry, m_player, *weaponData, m_gamedata);
    SystemPowerWeapon::InitWeapon(m_registry, m_player, "dummy", m_gamedata);

    m_registry.emplace<TargetTag>(m_player);

    auto& bounds = m_registry.emplace<ComponentBounds>(m_player);
    bounds.m_behaviour = BoundsBehaviour::Limit;
    bounds.m_bounds =
        canyon::MakeRect(0.0f, 0.0f, static_cast<float>(WorldSize.x), static_cast<float>(WorldSize.y));
}
