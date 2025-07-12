#pragma once

#include "game_world.h"
#include <canyon/events/event_window.h>
#include <canyon/graphics/igraphics.h>
#include <moth_ui/events/event_key.h>
#include <moth_ui/layers/layer.h>
#include <entt/entt.hpp>

class GameLayer : public moth_ui::Layer {
public:
    GameLayer(canyon::platform::Window& window, moth_ui::Context& context,
              canyon::graphics::IGraphics& graphics);
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
    bool OnWindowResize(canyon::EventWindowSize const& event);
    bool OnRequestQuitEvent(canyon::EventRequestQuit const& event);

    void CreatePlayer();

    canyon::platform::Window& m_window;
    canyon::graphics::IGraphics& m_graphics;
    std::unique_ptr<GameWorld> m_gameWorld;
    std::unique_ptr<canyon::graphics::IFont> m_font;
};
