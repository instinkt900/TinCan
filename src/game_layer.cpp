#include "game_layer.h"
#include "system_input.h"
#include "system_movement.h"
#include "system_sprite.h"
#include "system_weapon.h"
#include "system_lifetime.h"
#include <canyon/events/event_window.h>
#include <canyon/platform/window.h>
#include <moth_ui/events/event_dispatch.h>
#include <moth_ui/events/event_key.h>
#include <moth_ui/layers/layer_stack.h>
#include <stdio.h>
#include <spdlog/spdlog.h>

GameLayer::GameLayer(canyon::platform::Window& window, moth_ui::Context& context, canyon::graphics::IGraphics& graphics)
    : m_window(window)
    , m_graphics(graphics) {
}

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
    SystemMovement::Update(m_registry, ticks);
    SystemWeapon::Update(m_registry, ticks, m_window.GetSurfaceContext());
    SystemLifetime::Update(m_registry, ticks);
}

void GameLayer::Draw() {
    SystemSprite::Update(m_registry, m_graphics);
}

void GameLayer::OnAddedToStack(moth_ui::LayerStack* stack) {
    Layer::OnAddedToStack(stack);

    auto& surfaceContext = m_window.GetSurfaceContext();

    m_player = m_registry.create();
    auto& position = m_registry.emplace<ComponentPosition>(m_player);
    position.m_position = { m_window.GetWidth() / 2, m_window.GetHeight() - 30 };

    auto& velocity = m_registry.emplace<ComponentVelocity>(m_player);
    velocity.m_velocity = { 0, 0 };

    m_registry.emplace<ComponentInput>(m_player);

    auto& sprite = m_registry.emplace<ComponentSprite>(m_player);
    sprite.m_sprite = surfaceContext.ImageFromFile("assets/player.png");
    sprite.m_size = { 150, 150 };

    auto& weapon = m_registry.emplace<ComponentWeapon>(m_player);
    weapon.m_player = true;
    weapon.m_color = BulletColor::WHITE;
    weapon.m_maxCooldown = 1000;
    weapon.m_cooldown = 1000;

    spdlog::info("SIZE {} {}\n", m_window.GetWidth(), m_window.GetHeight());
    spdlog::info("POS {} {}\n", position.m_position.x, position.m_position.y);
}


bool GameLayer::OnWindowResize(canyon::EventWindowSize const& event) {
    auto& position = m_registry.get<ComponentPosition>(m_player);
    position.m_position = { event.GetWidth() / 2, event.GetHeight() - 30 };
    spdlog::info("RESIZE\n");
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
