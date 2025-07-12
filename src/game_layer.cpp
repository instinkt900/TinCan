#include "game_layer.h"
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
    , m_graphics(graphics) {
    m_gameWorld = std::make_unique<GameWorld>(m_window, m_graphics);
}

bool GameLayer::OnEvent(moth_ui::Event const& event) {
    moth_ui::EventDispatch dispatch(event);
    dispatch.Dispatch(this, &GameLayer::OnRequestQuitEvent);
    dispatch.Dispatch(this, &GameLayer::OnWindowResize);
    dispatch.Dispatch(this, &GameLayer::OnKeyEvent);
    dispatch.Dispatch(m_gameWorld.get());
    bool handled = dispatch.GetHandled();
    return handled;
}

void GameLayer::Update(uint32_t ticks) {
    m_gameWorld->Update(ticks);
}

void GameLayer::Draw() {
    m_gameWorld->Draw();

    auto& worldImage = m_gameWorld->GetWorldImage();
    auto const worldRect =
        canyon::FloatVec2{ worldImage.GetWidth(), worldImage.GetHeight() };
    auto const windowRect = canyon::FloatVec2{ m_window.GetWidth(), m_window.GetHeight() };
    auto const worldScaling = windowRect / worldRect;
    auto const scaling = std::min(worldScaling.x, worldScaling.y);
    auto const destSize = worldRect * scaling;
    auto const destPos = (windowRect - destSize) / 2;
    auto const destRect = canyon::IntRect{ static_cast<canyon::IntVec2>(destPos),
                                           static_cast<canyon::IntVec2>(destPos + destSize) };
    m_graphics.DrawImage(worldImage, destRect, nullptr);

    auto const playerHealth = m_gameWorld->GetPlayerHealth();
    m_graphics.SetColor(canyon::graphics::BasicColors::White);
    m_graphics.SetBlendMode(canyon::graphics::BlendMode::Alpha);
    m_graphics.DrawText(fmt::format("Player health: {}", playerHealth), *m_font,
                        canyon::graphics::TextHorizAlignment::Left,
                        canyon::graphics::TextVertAlignment::Middle,
                        canyon::MakeRect(0, 0, m_window.GetWidth(), 80));
}

void GameLayer::OnAddedToStack(moth_ui::LayerStack* stack) {
    Layer::OnAddedToStack(stack);

    auto& surfaceContext = m_window.GetSurfaceContext();
    m_font = surfaceContext.FontFromFile("assets/font.ttf", 24);
}


bool GameLayer::OnWindowResize(canyon::EventWindowSize const& event) {
    // auto& position = m_registry.get<ComponentPosition>(m_player);
    // position.m_position = { event.GetWidth() / 2, event.GetHeight() - 60 };
    //
    // if (auto* enemySpawnerPosition = m_registry.try_get<ComponentPosition>(m_enemySpawner)) {
    //     enemySpawnerPosition->m_position.x = static_cast<float>(m_window.GetWidth()) / 2.0f;
    //     enemySpawnerPosition->m_position.y = -10;
    // }
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

    return false;
}

