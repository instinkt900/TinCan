#include "game_over_layer.h"
#include "gamedata_input.h"
#include <moth/graphics/events/event_window.h>
#include <moth/ui/events/event_dispatch.h>
#include <moth/ui/node_factory.h>
#include <spdlog/spdlog.h>

namespace {
    char const* const LayoutPath = "assets/layouts/game_over.mothui";
    char const* const FadeClip = "fade_in";
}

GameOverLayer::GameOverLayer(moth::ui::Context& context, InputData const& bindings)
    : m_context(context)
    , m_bindings(bindings) {
}

void GameOverLayer::OnAddedToStack(moth::ui::LayerStack* stack) {
    Layer::OnAddedToStack(stack);
    m_width = GetWidth();
    m_height = GetHeight();
    auto [root, result] = moth::ui::NodeFactory::Get().Create(m_context, LayoutPath, m_width, m_height);
    if (result != moth::ui::Layout::LoadResult::Success || root == nullptr) {
        spdlog::error("Unable to load the game over layout {}", LayoutPath);
        return;
    }
    m_root = root;
    m_root->SetEventHandler([this](moth::ui::Node* /*node*/, moth::ui::Event const& event) {
        moth::ui::EventDispatch dispatch(event);
        dispatch.Dispatch(this, &GameOverLayer::OnAnimationStopped);
        return dispatch.GetHandled();
    });
}

void GameOverLayer::Show() {
    m_shown = true;
    m_faded = false;
    m_retry = false;
    if (m_root == nullptr || !m_root->SetAnimation(FadeClip)) {
        // with nothing to fade, the game stops at once rather than never
        m_faded = true;
    }
}

void GameOverLayer::Hide() {
    m_shown = false;
    m_faded = false;
    m_retry = false;
}

bool GameOverLayer::TakeRetry() {
    bool const retry = m_retry;
    m_retry = false;
    return retry;
}

bool GameOverLayer::OnEvent(moth::ui::Event const& event) {
    if (!m_shown) {
        return false;
    }
    moth::ui::EventDispatch dispatch(event);
    dispatch.Dispatch(this, &GameOverLayer::OnKeyEvent);
    return dispatch.GetHandled();
}

bool GameOverLayer::OnKeyEvent(moth::ui::EventKey const& event) {
    // every key belongs to the overlay while it shows, so the game behind cannot be paused or played. the keys only
    // work once the fade is done
    if (!m_faded) {
        return true;
    }
    if (event.GetAction() == moth::ui::KeyAction::Down && m_bindings.IsBound(InputAction::Fire, event.GetKey())) {
        m_retry = true;
    } else if (event.GetAction() == moth::ui::KeyAction::Up && m_bindings.IsBound(InputAction::Quit, event.GetKey())) {
        FireEvent(moth::gfx::EventQuit());
    }
    return true;
}

bool GameOverLayer::OnAnimationStopped(moth::ui::EventAnimationStopped const& event) {
    if (event.GetClipName() == FadeClip) {
        m_faded = true;
        return true;
    }
    return false;
}

void GameOverLayer::Update(uint32_t ticks) {
    if (m_root == nullptr) {
        return;
    }
    // the layout follows the window when it is resized
    if (GetWidth() != m_width || GetHeight() != m_height) {
        m_width = GetWidth();
        m_height = GetHeight();
        m_root->SetScreenRect(moth::ui::IntRect{ { 0, 0 }, { m_width, m_height } });
    }
    if (m_shown) {
        m_root->Update(ticks);
    }
}

void GameOverLayer::Draw() {
    if (m_shown && m_root != nullptr) {
        m_root->Draw();
    }
}
