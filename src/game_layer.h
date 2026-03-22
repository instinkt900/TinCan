#pragma once

#include "game_world.h"
#include <moth_graphics/events/event_window.h>
#include <moth_graphics/graphics/igraphics.h>
#include <moth_ui/events/event_key.h>
#include <moth_ui/layers/layer.h>
#include <entt/entt.hpp>

class GameLayer : public moth_ui::Layer {
public:
    GameLayer(moth_graphics::platform::Window& window, moth_ui::Context& context,
              moth_graphics::graphics::IGraphics& graphics);
    ~GameLayer() override = default;

    GameLayer(const GameLayer&) = delete;
    GameLayer(GameLayer&&) = delete;
    GameLayer& operator=(const GameLayer&) = delete;
    GameLayer& operator=(GameLayer&&) = delete;

    bool OnEvent(moth_ui::Event const& event) override;
    void Update(uint32_t ticks) override;
    void Draw() override;
    void OnAddedToStack(moth_ui::LayerStack* stack) override;


private:
    bool OnKeyEvent(moth_ui::EventKey const& event);
    bool OnWindowResize(moth_graphics::EventWindowSize const& event);
    bool OnRequestQuitEvent(moth_graphics::EventRequestQuit const& event);

    void CreatePlayer();

    moth_graphics::platform::Window& m_window;
    moth_graphics::graphics::IGraphics& m_graphics;
    std::unique_ptr<GameWorld> m_gameWorld;
    std::unique_ptr<moth_graphics::graphics::IFont> m_font;
};
