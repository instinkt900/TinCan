#pragma once

#include "game_over_layer.h"
#include "game_world.h"
#include <moth/graphics/events/event_window.h>
#include <moth/graphics/graphics/igraphics.h>
#include <moth/ui/events/event_key.h>
#include <moth/ui/layers/layer.h>
#include <moth/ecs/world.h>
#include <moth/profile/imgui/profiler_panel.h>
#include <unordered_set>

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
    moth::ui::Context& m_context;
    moth::gfx::IGraphics& m_graphics;
    std::unique_ptr<GameWorld> m_gameWorld;
    // pushed above this layer once, when this layer is added, and owned by the stack. it is shown and hidden rather
    // than pushed and removed, because the stack cannot change while it updates or draws its layers
    GameOverLayer* m_gameOverLayer = nullptr;
    std::unique_ptr<moth::gfx::IFont> m_font;
    moth::profile::ProfilerPanel m_profilerPanel;
    // keys that went down while an editor text field was active, and have not come up yet
    std::unordered_set<moth::ui::Key> m_keysPressedWhileTyping;
};
