#include "game_layer.h"
#include <moth_graphics/events/event_window.h>
#include <moth_graphics/platform/window.h>
#include <cmath>
#include <moth_ui/events/event_dispatch.h>
#include <moth_ui/events/event_key.h>
#include <moth_ui/layers/layer_stack.h>
#include <spdlog/spdlog.h>

GameLayer::GameLayer(moth_graphics::platform::Window& window, moth_ui::Context& context,
                     moth_graphics::graphics::IGraphics& graphics)
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

void GameLayer::Update(uint32_t ticks) { m_gameWorld->Update(ticks); }

void GameLayer::Draw() {
    // background fill
    m_graphics.SetBlendMode(moth_graphics::graphics::BlendMode::Replace);
    m_graphics.SetColor(moth_graphics::graphics::BasicColors::Blue);
    m_graphics.DrawFillRectF(moth_graphics::MakeRect(0.0f, 0.0f, static_cast<float>(m_window.GetWidth()),
                                              static_cast<float>(m_window.GetHeight())));

    // let the world draw
    m_gameWorld->Draw();

    // draw the game world
    m_graphics.SetColor(moth_graphics::graphics::BasicColors::White);
    m_graphics.SetBlendMode(moth_graphics::graphics::BlendMode::Replace);
    auto& worldImage = m_gameWorld->GetWorldImage();
    auto const worldRect = moth_graphics::FloatVec2{ worldImage.GetWidth(), worldImage.GetHeight() };
    auto const windowRect = moth_graphics::FloatVec2{ m_window.GetWidth(), m_window.GetHeight() };
    auto const worldScaling = windowRect / worldRect;
    auto const scaling = std::min(worldScaling.x, worldScaling.y);
    auto const destSize = worldRect * scaling;
    auto const destPos = (windowRect - destSize) / 2;
    auto const destRect = moth_graphics::IntRect{ static_cast<moth_graphics::IntVec2>(destPos),
                                           static_cast<moth_graphics::IntVec2>(destPos + destSize) };
    m_graphics.DrawImage(worldImage, destRect, nullptr, 0);

    // draw the ui
    auto const playerHealth = m_gameWorld->GetPlayerHealth();
    auto const playerPower = m_gameWorld->GetPlayerPower();
    m_graphics.SetColor(moth_graphics::graphics::BasicColors::White);
    m_graphics.SetBlendMode(moth_graphics::graphics::BlendMode::Alpha);
    m_graphics.DrawText(fmt::format("Player health: {}", playerHealth), *m_font,
                        moth_graphics::MakeRect(0, 0, m_window.GetWidth(), 80),
                        moth_graphics::graphics::TextHorizAlignment::Left,
                        moth_graphics::graphics::TextVertAlignment::Middle);
    m_graphics.DrawText(fmt::format("Player power: {}", playerPower), *m_font,
                        moth_graphics::MakeRect(0, 0, m_window.GetWidth(), 120),
                        moth_graphics::graphics::TextHorizAlignment::Left,
                        moth_graphics::graphics::TextVertAlignment::Middle);
}

void GameLayer::OnAddedToStack(moth_ui::LayerStack* stack) {
    Layer::OnAddedToStack(stack);

    auto& surfaceContext = m_window.GetSurfaceContext();
    m_font = surfaceContext.GetAssetContext().FontFromFile("assets/font.ttf", 24);
}


bool GameLayer::OnWindowResize(moth_graphics::EventWindowSize const& event) {
    // auto& position = m_registry.get<ComponentPosition>(m_player);
    // position.m_position = { event.GetWidth() / 2, event.GetHeight() - 60 };
    //
    // if (auto* enemySpawnerPosition = m_registry.try_get<ComponentPosition>(m_enemySpawner)) {
    //     enemySpawnerPosition->m_position.x = static_cast<float>(m_window.GetWidth()) / 2.0f;
    //     enemySpawnerPosition->m_position.y = -10;
    // }
    return false;
}

bool GameLayer::OnRequestQuitEvent(moth_graphics::EventRequestQuit const& event) {
    m_layerStack->FireEvent(moth_graphics::EventQuit());
    return true;
}

bool GameLayer::OnKeyEvent(moth_ui::EventKey const& event) {
    if (event.GetAction() == moth_ui::KeyAction::Up) {
        if (event.GetKey() == moth_ui::Key::Escape) {
            m_layerStack->FireEvent(moth_graphics::EventQuit());
            return true;
        }
    }

    return false;
}
