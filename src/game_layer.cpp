#include "game_layer.h"
#include "system_health.h"
#include "system_input.h"
#include "system_level.h"
#include "system_movement.h"
#include "system_drawable.h"
#include "system_pickup.h"
#include "system_player_visuals.h"
#include "system_shield.h"
#include "system_weapon.h"
#include "system_lifetime.h"
#include "system_enemy_spawner.h"
#include "system_group.h"
#include "tags.h"
#include <canyon/events/event_window.h>
#include <canyon/platform/window.h>
#include <cmath>
#include <moth_ui/events/event_dispatch.h>
#include <moth_ui/events/event_key.h>
#include <moth_ui/layers/layer_stack.h>
#include <spdlog/spdlog.h>

GameLayer::GameLayer(canyon::platform::Window& window, moth_ui::Context& context,
                     canyon::graphics::IGraphics& graphics)
    : m_window(window)
    , m_graphics(graphics) {}

GameLayer::~GameLayer() {}

bool GameLayer::OnEvent(moth_ui::Event const& event) {
    moth_ui::EventDispatch dispatch(event);
    dispatch.Dispatch(this, &GameLayer::OnRequestQuitEvent);
    dispatch.Dispatch(this, &GameLayer::OnWindowResize);
    dispatch.Dispatch(this, &GameLayer::OnKeyEvent);
    bool handled = dispatch.GetHandled();
    return handled;
}

void GameLayer::Update(uint32_t ticks) {
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
    SystemGroup::Update(m_registry, ticks);
}

void GameLayer::Draw() {
    SystemDrawable::Update(m_registry, m_graphics);

    if (auto* playerHealth = m_registry.try_get<ComponentHealth>(m_player)) {
        m_graphics.SetColor(canyon::graphics::BasicColors::White);
        m_graphics.SetBlendMode(canyon::graphics::BlendMode::Alpha);
        m_graphics.DrawText(fmt::format("Player health: {}", playerHealth->m_currentHealth), *m_font,
                            canyon::graphics::TextHorizAlignment::Left,
                            canyon::graphics::TextVertAlignment::Middle,
                            canyon::MakeRect(0, 0, m_window.GetWidth(), 80));
    }
}

void GameLayer::OnAddedToStack(moth_ui::LayerStack* stack) {
    Layer::OnAddedToStack(stack);

    auto& surfaceContext = m_window.GetSurfaceContext();
    m_font = surfaceContext.FontFromFile("assets/font.ttf", 24);

    SerializeContext serializeContext{ m_gamedata, surfaceContext };
    m_gamedata.GetSpriteDatabase().Load("data/sprite_database.json", serializeContext);
    m_gamedata.GetEnemyDatabase().Load("data/enemy_database.json", serializeContext);
    m_gamedata.GetWeaponDatabase().Load("data/weapon_database_player.json", serializeContext);
    m_gamedata.GetWeaponDatabase().Load("data/weapon_database_enemy.json", serializeContext);
    m_gamedata.GetProjectileDatabase().Load("data/projectile_database.json", serializeContext);
    m_gamedata.GetSpawnerDatabase().Load("data/spawner_database.json", serializeContext);
    m_gamedata.GetLevelDatabase().Load("data/level_database.json", serializeContext);

    SystemLevel::InitLevel(m_registry, "test", m_gamedata);
    CreatePlayer();
}


bool GameLayer::OnWindowResize(canyon::EventWindowSize const& event) {
    auto& position = m_registry.get<ComponentPosition>(m_player);
    position.m_position = { event.GetWidth() / 2, event.GetHeight() - 60 };

    if (auto* enemySpawnerPosition = m_registry.try_get<ComponentPosition>(m_enemySpawner)) {
        enemySpawnerPosition->m_position.x = static_cast<float>(m_window.GetWidth()) / 2.0f;
        enemySpawnerPosition->m_position.y = -10;
    }
    return false;
}

bool GameLayer::OnRequestQuitEvent(canyon::EventRequestQuit const& event) {
    m_layerStack->BroadcastEvent(canyon::EventQuit());
    return true;
}

bool GameLayer::OnKeyEvent(moth_ui::EventKey const& event) {
    if (event.GetAction() == moth_ui::KeyAction::Up) {
        if (event.GetKey() == moth_ui::Key::Escape) {
            m_layerStack->BroadcastEvent(canyon::EventQuit());
            return true;
        }
    }

    SystemInput::OnKey(m_registry, event);

    return false;
}

void GameLayer::CreatePlayer() {
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
    position.m_position = { m_window.GetWidth() / 2, m_window.GetHeight() - 60 };

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

    SystemWeapon::InitWeapon(m_registry, m_player, "player_weapon_01", m_gamedata);

    m_registry.emplace<TargetTag>(m_player);
}
