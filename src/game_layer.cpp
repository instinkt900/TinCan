#include "game_layer.h"
#include <moth/graphics/events/event_window.h>
#include <moth/graphics/platform/window.h>
#include <cmath>
#include <moth/ui/events/event_dispatch.h>
#include <moth/ui/events/event_key.h>
#include <moth/ui/layers/layer_stack.h>
#include <moth/profile/profiler.h>
#include <spdlog/spdlog.h>

GameLayer::GameLayer(moth::gfx::platform::Window& window, moth::ui::Context& context,
                     moth::gfx::IGraphics& graphics)
    : m_window(window)
    , m_graphics(graphics) {
    m_gameWorld = std::make_unique<GameWorld>(m_window, m_graphics);
}

bool GameLayer::OnEvent(moth::ui::Event const& event) {
    moth::ui::EventDispatch dispatch(event);
    dispatch.Dispatch(this, &GameLayer::OnRequestQuitEvent);
    dispatch.Dispatch(this, &GameLayer::OnWindowResize);
    dispatch.Dispatch(this, &GameLayer::OnKeyEvent);
    dispatch.Dispatch(m_gameWorld.get());
    bool handled = dispatch.GetHandled();
    return handled;
}

void GameLayer::Update(uint32_t ticks) {
    MOTH_PROFILE_SCOPE("Update");
    m_gameWorld->Update(ticks);
}

void GameLayer::Draw() {
    // a profiler frame runs from one draw to the next, so it also covers the updates in between
    MOTH_PROFILE_FRAME();
    {
        MOTH_PROFILE_SCOPE("Profiler panel");
        m_profilerPanel.Draw();
    }
    MOTH_PROFILE_SCOPE("Draw");

    // m_graphics.SetLogicalSize({1920,1080});

    // background fill
    m_graphics.SetBlendMode(moth::gfx::BlendMode::Replace);
    m_graphics.SetColor(moth::gfx::BasicColors::Blue);
    m_graphics.DrawFillRectF(moth::gfx::MakeRect(0.0f, 0.0f, static_cast<float>(m_window.GetWidth()),
                                              static_cast<float>(m_window.GetHeight())));

    // let the world draw
    {
        MOTH_PROFILE_SCOPE("World");
        m_gameWorld->Draw();
    }

    // draw the game world
    m_graphics.SetColor(moth::gfx::BasicColors::White);
    m_graphics.SetBlendMode(moth::gfx::BlendMode::Replace);
    auto const worldImage = m_gameWorld->GetWorldImage();
    auto const worldRect = moth::gfx::FloatVec2{ worldImage.GetWidth(), worldImage.GetHeight() };
    auto const windowRect = moth::gfx::FloatVec2{ m_window.GetWidth(), m_window.GetHeight() };
    auto const worldScaling = windowRect / worldRect;
    auto const scaling = std::min(worldScaling.x, worldScaling.y);
    auto const destSize = worldRect * scaling;
    auto const destPos = (windowRect - destSize) / 2;
    auto const destRect = moth::gfx::IntRect{ static_cast<moth::gfx::IntVec2>(destPos),
                                           static_cast<moth::gfx::IntVec2>(destPos + destSize) };
    m_graphics.DrawImage(worldImage, destRect);

    // draw the ui
    auto const playerHealth = m_gameWorld->GetPlayerHealth();
    auto const playerPower = m_gameWorld->GetPlayerPower();
    m_graphics.SetColor(moth::gfx::BasicColors::White);
    m_graphics.SetBlendMode(moth::gfx::BlendMode::Alpha);
    m_graphics.DrawText(fmt::format("Player health: {}", playerHealth), *m_font,
                        moth::gfx::MakeRect(0, 0, m_window.GetWidth(), 80),
                        moth::gfx::TextHorizAlignment::Left,
                        moth::gfx::TextVertAlignment::Middle);
    m_graphics.DrawText(fmt::format("Player power: {}", playerPower), *m_font,
                        moth::gfx::MakeRect(0, 0, m_window.GetWidth(), 120),
                        moth::gfx::TextHorizAlignment::Left,
                        moth::gfx::TextVertAlignment::Middle);
}

void GameLayer::OnAddedToStack(moth::ui::LayerStack* stack) {
    Layer::OnAddedToStack(stack);

    auto& surfaceContext = m_window.GetSurfaceContext();
    m_font = surfaceContext.GetAssetContext().FontFromFile("assets/font.ttf", 24);
}


bool GameLayer::OnWindowResize(moth::gfx::EventWindowSize const& event) {
    // auto& position = m_registry.Get<ComponentPosition>(m_player);
    // position.m_position = { event.GetWidth() / 2, event.GetHeight() - 60 };
    //
    // if (auto* enemySpawnerPosition = m_registry.TryGet<ComponentPosition>(m_enemySpawner)) {
    //     enemySpawnerPosition->m_position.x = static_cast<float>(m_window.GetWidth()) / 2.0f;
    //     enemySpawnerPosition->m_position.y = -10;
    // }
    return false;
}

bool GameLayer::OnRequestQuitEvent(moth::gfx::EventRequestQuit const& event) {
    FireEvent(moth::gfx::EventQuit());
    return true;
}

bool GameLayer::OnKeyEvent(moth::ui::EventKey const& event) {
    if (event.GetAction() == moth::ui::KeyAction::Up) {
        if (event.GetKey() == moth::ui::Key::Escape) {
            FireEvent(moth::gfx::EventQuit());
            return true;
        }
        if (event.GetKey() == moth::ui::Key::F1) {
            m_profilerPanel.ToggleOpen();
            return true;
        }
    }

    return false;
}
