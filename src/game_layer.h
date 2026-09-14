#pragma once

#include "game_world.h"
#include <moth/graphics/events/event_window.h>
#include <moth/graphics/graphics/igraphics.h>
#include <moth/ui/events/event_key.h>
#include <moth/ui/layers/layer.h>
#include <moth/ecs/world.h>
#include <moth/profile/imgui/profiler_panel.h>

class GameLayer : public moth::ui::Layer {
public:
    GameLayer(moth::gfx::platform::Window& window, moth::ui::Context& context,
              moth::gfx::IGraphics& graphics);
    ~GameLayer() override = default;

    GameLayer(const GameLayer&) = delete;
    GameLayer(GameLayer&&) = delete;
    GameLayer& operator=(const GameLayer&) = delete;
    GameLayer& operator=(GameLayer&&) = delete;

    bool OnEvent(moth::ui::Event const& event) override;
    void Update(uint32_t ticks) override;
    void Draw() override;
    void OnAddedToStack(moth::ui::LayerStack* stack) override;


private:
    bool OnKeyEvent(moth::ui::EventKey const& event);
    bool OnWindowResize(moth::gfx::EventWindowSize const& event);
    bool OnRequestQuitEvent(moth::gfx::EventRequestQuit const& event);

    void CreatePlayer();

    moth::gfx::platform::Window& m_window;
    moth::gfx::IGraphics& m_graphics;
    std::unique_ptr<GameWorld> m_gameWorld;
    std::unique_ptr<moth::gfx::IFont> m_font;
    moth::profile::ProfilerPanel m_profilerPanel;
};
