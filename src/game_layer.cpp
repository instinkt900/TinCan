#include "game_layer.h"
#include "system_input.h"
#include "system_movement.h"
#include "system_sprite.h"
#include "system_weapon.h"
#include "system_lifetime.h"
#include "system_enemy_spawner.h"
#include "system_collision.h"
#include <canyon/events/event_window.h>
#include <canyon/platform/window.h>
#include <moth_ui/events/event_dispatch.h>
#include <moth_ui/events/event_key.h>
#include <moth_ui/layers/layer_stack.h>
#include <stdio.h>
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
    SystemEnemySpawner::Update(m_registry, ticks);
    SystemMovement::Update(m_registry, ticks);
    SystemWeapon::Update(m_registry, ticks);
    SystemCollision::Update(m_registry, ticks);
    SystemLifetime::Update(m_registry, ticks);
}

void GameLayer::Draw() {
    SystemSprite::Update(m_registry, m_graphics);

    if (auto* playerHealth = m_registry.try_get<ComponentPlayer>(m_player)) {
        m_graphics.DrawText(fmt::format("Player health: {}", playerHealth->m_health), *m_font,
                            canyon::graphics::TextHorizAlignment::Left,
                            canyon::graphics::TextVertAlignment::Middle,
                            canyon::MakeRect(0, 0, m_window.GetWidth(), 80));
    }
}

void GameLayer::OnAddedToStack(moth_ui::LayerStack* stack) {
    Layer::OnAddedToStack(stack);

    auto& surfaceContext = m_window.GetSurfaceContext();

    m_font = surfaceContext.FontFromFile("assets/font.ttf", 24);

    m_player = m_registry.create();
    m_registry.emplace<ComponentPlayer>(m_player);

    auto& playerCollision = m_registry.emplace<ComponentCollision>(m_player);
    playerCollision.m_collisionMask = CollisionGroups::PLAYER;
    playerCollision.m_collidesWithMask = 0;
    playerCollision.m_radius = 5;

    auto& position = m_registry.emplace<ComponentPosition>(m_player);
    position.m_position = { m_window.GetWidth() / 2, m_window.GetHeight() - 60 };

    auto& velocity = m_registry.emplace<ComponentVelocity>(m_player);
    velocity.m_velocity = { 0, 0 };

    m_registry.emplace<ComponentInput>(m_player);

    auto& sprite = m_registry.emplace<ComponentSprite>(m_player);
    sprite.m_sprite = surfaceContext.ImageFromFile("assets/player.png");
    sprite.m_size = { 48, 48 };

    auto& weapon = m_registry.emplace<ComponentWeapon>(m_player);
    weapon.m_player = true;
    weapon.m_color = BulletColor::WHITE;
    weapon.m_maxCooldown = 200;
    weapon.m_cooldown = 0;
    weapon.m_projectile.m_sprite = surfaceContext.ImageFromFile("assets/bullet.png");
    weapon.m_projectile.m_spriteSize = { weapon.m_projectile.m_sprite->GetWidth(),
                                         weapon.m_projectile.m_sprite->GetHeight() };
    weapon.m_projectile.m_speed = 2000;
    weapon.m_projectile.m_lifetime = 500;
    weapon.m_projectile.m_radius = 5;
    weapon.m_projectile.m_collisionMask = 0;
    weapon.m_projectile.m_collidesWithMask = CollisionGroups::ENEMY;
    weapon.m_projectile.m_onCollision = [&](entt::entity projectile, entt::entity enemy) {
      spdlog::info("Enemy hit");
    };

    m_enemySpawner = m_registry.create();
    auto& enemySpawner = m_registry.emplace<ComponentEnemySpawner>(m_enemySpawner);
    enemySpawner.m_active = true;
    enemySpawner.m_maxCooldown = 5000;
    enemySpawner.m_template.m_sprite = surfaceContext.ImageFromFile("assets/enemy.png");
    enemySpawner.m_template.m_spriteSize = { enemySpawner.m_template.m_sprite->GetWidth(),
                                             enemySpawner.m_template.m_sprite->GetHeight() };
    enemySpawner.m_template.m_speed = 200.0f;
    enemySpawner.m_template.m_weapon.m_player = false;
    enemySpawner.m_template.m_weapon.m_maxCooldown = 2000;
    enemySpawner.m_template.m_weapon.m_projectile.m_sprite =
        surfaceContext.ImageFromFile("assets/enemy_bullet.png");
    enemySpawner.m_template.m_weapon.m_projectile.m_spriteSize = {
        enemySpawner.m_template.m_weapon.m_projectile.m_sprite->GetWidth(),
        enemySpawner.m_template.m_weapon.m_projectile.m_sprite->GetHeight()
    };
    enemySpawner.m_template.m_weapon.m_projectile.m_speed = 1000;
    enemySpawner.m_template.m_weapon.m_projectile.m_collidesWithMask = CollisionGroups::PLAYER;
    enemySpawner.m_template.m_weapon.m_projectile.m_radius = 5;
    enemySpawner.m_template.m_weapon.m_projectile.m_onCollision = [&](entt::entity projectileEntity,
                                                                      entt::entity otherEntity) {
        spdlog::info("enemy bullet hit player");
        if (auto* lifetime = m_registry.try_get<ComponentLifetime>(projectileEntity)) {
            lifetime->m_lifetime = 0;
        }

        uint32_t const damage = 10;
        if (auto* player = m_registry.try_get<ComponentPlayer>(otherEntity)) {
            if (player->m_health <= damage) {
                spdlog::info("player dies");
            } else {
                player->m_health -= damage;
            }
        }
    };
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
