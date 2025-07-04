#include "game_layer.h"
#include "system_input.h"
#include "system_movement.h"
#include "system_drawable.h"
#include "system_weapon.h"
#include "system_lifetime.h"
#include "system_enemy_spawner.h"
#include <canyon/events/event_window.h>
#include <canyon/platform/window.h>
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
    SystemLifetime::Update(m_registry, ticks);
    SystemEnemySpawner::Update(m_registry, ticks);
    SystemMovement::Update(m_registry, ticks);
    SystemWeapon::Update(m_registry, ticks);
    SystemProjectile::Update(m_registry, ticks);
}

void GameLayer::Draw() {
    SystemDrawable::Update(m_registry, m_graphics);

    if (auto* playerHealth = m_registry.try_get<ComponentHealth>(m_player)) {
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

    CreatePlayer();

    m_enemySpawner = m_registry.create();
    auto& enemySpawner = m_registry.emplace<ComponentEnemySpawner>(m_enemySpawner);
    enemySpawner.m_active = true;
    enemySpawner.m_maxCooldown = 5000;
    Sprite enemyShipSprite;
    enemyShipSprite.m_image = surfaceContext.ImageFromFile("assets/enemy.png");
    enemyShipSprite.m_size = { enemyShipSprite.m_image->GetWidth(), enemyShipSprite.m_image->GetHeight() };
    enemyShipSprite.m_blendMode = canyon::graphics::BlendMode::Alpha;
    enemySpawner.m_enemyTemplate.m_drawable.m_sprites.push_back(enemyShipSprite);
    enemySpawner.m_enemyTemplate.m_team = Team::ENEMY;
    enemySpawner.m_enemyTemplate.m_radius = 30.0f;
    enemySpawner.m_enemyTemplate.m_maxHealth = 30;
    enemySpawner.m_enemyTemplate.m_onDeath = [&](entt::entity enemy, entt::entity damager) {
        if (auto* lifetime = m_registry.try_get<ComponentLifetime>(enemy)) {
            lifetime->m_lifetime = 0;
        }
    };
    enemySpawner.m_enemyTemplate.m_speed = 200.0f;
    enemySpawner.m_enemyTemplate.m_lifetime = 10000.0f;
    enemySpawner.m_enemyTemplate.m_weapon.m_maxCooldown = 2000;
    auto enemyProjectileImage = surfaceContext.ImageFromFile("assets/enemy_bullet.png");
    Sprite projectileSprite;
    projectileSprite.m_size = { enemyProjectileImage->GetWidth(), enemyProjectileImage->GetHeight() };
    projectileSprite.m_image = std::move(enemyProjectileImage);
    enemySpawner.m_enemyTemplate.m_weapon.m_projectileTemplate.m_drawable.m_sprites.push_back(projectileSprite);
    enemySpawner.m_enemyTemplate.m_weapon.m_projectileTemplate.m_team = Team::ENEMY;
    enemySpawner.m_enemyTemplate.m_weapon.m_projectileTemplate.m_color = ProjectileColor::WHITE;
    enemySpawner.m_enemyTemplate.m_weapon.m_projectileTemplate.m_damage = 10;
    enemySpawner.m_enemyTemplate.m_weapon.m_projectileTemplate.m_speed = 1000;
    enemySpawner.m_enemyTemplate.m_weapon.m_projectileTemplate.m_lifetime = 4000;
    enemySpawner.m_enemyTemplate.m_weapon.m_projectileTemplate.m_radius = 5;
}


bool GameLayer::OnWindowResize(canyon::EventWindowSize const& event) {
    auto& position = m_registry.get<ComponentPosition>(m_player);
    position.m_position = { event.GetWidth() / 2, event.GetHeight() - 60 };

    if (auto* enemySpawner = m_registry.try_get<ComponentEnemySpawner>(m_enemySpawner)) {
        enemySpawner->m_spawnRegion.topLeft.x = 0;
        enemySpawner->m_spawnRegion.topLeft.y = -60;
        enemySpawner->m_spawnRegion.bottomRight.x = event.GetWidth();
        enemySpawner->m_spawnRegion.bottomRight.y = 0;
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

    auto& entityData = m_registry.emplace<ComponentEntity>(m_player);
    entityData.m_team = Team::PLAYER;
    entityData.m_radius = 5.0f;

    auto& playerHealth = m_registry.emplace<ComponentHealth>(m_player);
    playerHealth.m_maxHealth = 100;
    playerHealth.m_currentHealth = 100;
    playerHealth.m_onDeath = [&](entt::entity thisEntity, entt::entity killerEntity) {
        spdlog::info("Player dies");
    };

    auto& position = m_registry.emplace<ComponentPosition>(m_player);
    position.m_position = { m_window.GetWidth() / 2, m_window.GetHeight() - 60 };

    auto& velocity = m_registry.emplace<ComponentVelocity>(m_player);
    velocity.m_velocity = { 0, 0 };

    m_registry.emplace<ComponentInput>(m_player);

    auto& surfaceContext = m_window.GetSurfaceContext();
    auto& drawable = m_registry.emplace<ComponentDrawable>(m_player);

    Sprite shipSprite;
    shipSprite.m_image = surfaceContext.ImageFromFile("assets/player.png");
    shipSprite.m_size = { shipSprite.m_image->GetWidth(), shipSprite.m_image->GetHeight() };
    shipSprite.m_blendMode = canyon::graphics::BlendMode::Alpha;

    Sprite shieldSprite;
    shieldSprite.m_image = surfaceContext.ImageFromFile("assets/shield.png");
    shieldSprite.m_size = { shieldSprite.m_image->GetWidth() + 20, shieldSprite.m_image->GetHeight() + 20 };
    shieldSprite.m_blendMode = canyon::graphics::BlendMode::Add;

    drawable.m_sprites.push_back(shipSprite);
    drawable.m_sprites.push_back(shieldSprite);

    auto& weapon = m_registry.emplace<ComponentWeapon>(m_player);
    weapon.m_active = false;
    weapon.m_cooldown = 0;
    weapon.m_maxCooldown = 100;
    auto projectileImage = surfaceContext.ImageFromFile("assets/bullet.png");
    Sprite projectileSprite;
    projectileSprite.m_size = { projectileImage->GetWidth(), projectileImage->GetHeight() };
    projectileSprite.m_image = std::move(projectileImage);
    projectileSprite.m_blendMode = canyon::graphics::BlendMode::Alpha;
    weapon.m_projectileTemplate.m_drawable.m_sprites.push_back(projectileSprite);
    weapon.m_projectileTemplate.m_team = Team::PLAYER;
    weapon.m_projectileTemplate.m_color = ProjectileColor::WHITE;
    weapon.m_projectileTemplate.m_damage = 10;
    weapon.m_projectileTemplate.m_speed = 2000;
    weapon.m_projectileTemplate.m_lifetime = 500;
    weapon.m_projectileTemplate.m_radius = 5;
}
