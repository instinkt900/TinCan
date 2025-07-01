#include "game_layer.h"
#include <canyon/events/event_window.h>
#include <moth_ui/events/event_dispatch.h>
#include <moth_ui/events/event_key.h>
#include <moth_ui/layers/layer_stack.h>

GameLayer::GameLayer(moth_ui::Context &context,
                     canyon::graphics::IGraphics &graphics) {}

GameLayer::~GameLayer() {}

bool GameLayer::OnEvent(moth_ui::Event const &event) {
  moth_ui::EventDispatch dispatch(event);
  dispatch.Dispatch(this, &GameLayer::OnRequestQuitEvent);
  dispatch.Dispatch(this, &GameLayer::OnKeyEvent);
  bool handled = dispatch.GetHandled();
  // if (!handled && m_root) {
  //     handled = m_root->SendEvent(event,
  //     moth_ui::Node::EventDirection::Down);
  // }
  return handled;
}

void GameLayer::Update(uint32_t ticks) {}

void GameLayer::Draw() {}

void GameLayer::OnAddedToStack(moth_ui::LayerStack *stack) {
  Layer::OnAddedToStack(stack);
}

bool GameLayer::OnRequestQuitEvent(canyon::EventRequestQuit const &event) {
  m_layerStack->BroadcastEvent(canyon::EventQuit());
  return true;
}

bool GameLayer::OnKeyEvent(moth_ui::EventKey const &event) {
  if (event.GetAction() == moth_ui::KeyAction::Up) {
    if (event.GetKey() == moth_ui::Key::Escape) {
      m_layerStack->BroadcastEvent(canyon::EventQuit());
      return true;
    }
  }
  return false;
}
