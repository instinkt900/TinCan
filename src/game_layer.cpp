#include "game_layer.h"
#include "gamedata_input.h"
#include <moth/graphics/events/event_window.h>
#include <moth/graphics/platform/window.h>
#include <cmath>
#include <imgui.h>
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
    {
        MOTH_PROFILE_SCOPE("Profiler panel");
        m_profilerPanel.Draw();
    }
    MOTH_PROFILE_SCOPE("Draw");

    // m_graphics.SetLogicalSize({1920,1080});

    // where the world image will sit in the window. worked out before the world draws, so that the
    // behaviour editor can place the mouse in the world on the frame it runs
    {
        auto const worldImage = m_gameWorld->GetWorldImage();
        auto const worldSize = moth::gfx::FloatVec2{ worldImage.GetWidth(), worldImage.GetHeight() };
        auto const windowRect =
            moth::gfx::FloatVec2{ static_cast<float>(m_window.GetWidth()), static_cast<float>(m_window.GetHeight()) };
        WorldViewTransform transform;
        if (worldSize.x > 0.0f && worldSize.y > 0.0f) {
            auto const worldScaling = windowRect / worldSize;
            transform.m_scale = std::min(worldScaling.x, worldScaling.y);
            transform.m_offset = (windowRect - (worldSize * transform.m_scale)) / 2;
        }
        m_gameWorld->SetViewTransform(transform);
    }

    // background fill
    m_graphics.SetBlendMode(moth::gfx::BlendMode::Replace);
    m_graphics.SetColor(moth::gfx::BasicColors::Black);
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
    auto const& transform = m_gameWorld->GetViewTransform();
    auto const destSize = worldRect * transform.m_scale;
    auto const destPos = transform.m_offset;
    auto const destRect = moth::gfx::IntRect{ static_cast<moth::gfx::IntVec2>(destPos),
                                           static_cast<moth::gfx::IntVec2>(destPos + destSize) };
    m_graphics.DrawImage(worldImage, destRect);

    // draw the ui
    auto const playerPower = m_gameWorld->GetPlayerPower();
    auto const playerLives = m_gameWorld->GetPlayerLives();
    m_graphics.SetColor(moth::gfx::BasicColors::White);
    m_graphics.SetBlendMode(moth::gfx::BlendMode::Alpha);
    m_graphics.DrawText(fmt::format("Player power: {}", playerPower), *m_font,
                        moth::gfx::MakeRect(0, 0, m_window.GetWidth(), 120),
                        moth::gfx::TextHorizAlignment::Left,
                        moth::gfx::TextVertAlignment::Middle);
    m_graphics.DrawText(fmt::format("Lives: {}", playerLives), *m_font,
                        moth::gfx::MakeRect(0, 0, m_window.GetWidth(), 160),
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
    // while an editor text field is active the keys are the field's. a press is kept from the game
    // entirely, but a release still reaches the world so that a game key held before typing started
    // is let go. a key pressed while typing fires no binding when it comes up, even once the field has
    // closed, so that the Escape that ends an edit does not also quit
    bool const typing = ImGui::GetIO().WantTextInput;
    if (event.GetAction() == moth::ui::KeyAction::Down) {
        if (typing) {
            m_keysPressedWhileTyping.insert(event.GetKey());
            return true;
        }
    } else if (m_keysPressedWhileTyping.erase(event.GetKey()) > 0 || typing) {
        return false;
    }

    if (event.GetAction() == moth::ui::KeyAction::Up) {
        auto const& bindings = m_gameWorld->GetInputBindings();
        if (bindings.IsBound(InputAction::Quit, event.GetKey())) {
            FireEvent(moth::gfx::EventQuit());
            return true;
        }
        if (bindings.IsBound(InputAction::ToggleProfiler, event.GetKey())) {
            m_profilerPanel.ToggleOpen();
            return true;
        }
        if (bindings.IsBound(InputAction::ToggleEditor, event.GetKey())) {
            m_gameWorld->ToggleEditorMode();
            return true;
        }
    }

    return false;
}
