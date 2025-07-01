#pragma once

#include <canyon/events/event_window.h>
#include <canyon/graphics/igraphics.h>
#include <moth_ui/events/event_key.h>
#include <moth_ui/layers/layer.h>

class GameLayer : public moth_ui::Layer {
public:
  GameLayer(moth_ui::Context &context, canyon::graphics::IGraphics &graphics);
  virtual ~GameLayer();

  bool OnEvent(moth_ui::Event const &event) override;
  void Update(uint32_t ticks) override;
  void Draw() override;
  void OnAddedToStack(moth_ui::LayerStack *stack) override;

private:
  bool OnKeyEvent(moth_ui::EventKey const &event);
  bool OnRequestQuitEvent(canyon::EventRequestQuit const &event);
};
