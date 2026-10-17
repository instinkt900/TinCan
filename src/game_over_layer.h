#pragma once

#include <moth/ui/events/event_animation.h>
#include <moth/ui/events/event_key.h>
#include <moth/ui/layers/layer.h>
#include <moth/ui/nodes/group.h>
#include <memory>

struct InputData;

// The game over overlay. It sits above the game for the whole run and shows nothing until the game is over.
// Then "GAME OVER" shows at once while a black background fades in over the game, which carries on behind
// it. Once the fade is done the game stops, and the overlay waits for Fire to retry or Quit to leave.
class GameOverLayer : public moth::ui::Layer {
public:
    GameOverLayer(moth::ui::Context& context, InputData const& bindings);

    void Show();
    void Hide();
    bool IsShown() const { return m_shown; }
    // whether the fade is done, so the game behind should stop
    bool IsFaded() const { return m_faded; }
    // whether Fire was pressed to retry since the last call
    bool TakeRetry();

    bool OnEvent(moth::ui::Event const& event) override;
    void Update(uint32_t ticks) override;
    void Draw() override;
    void OnAddedToStack(moth::ui::LayerStack* stack) override;

private:
    bool OnKeyEvent(moth::ui::EventKey const& event);
    bool OnAnimationStopped(moth::ui::EventAnimationStopped const& event);

    moth::ui::Context& m_context;
    InputData const& m_bindings;
    std::shared_ptr<moth::ui::Group> m_root;
    int m_width = 0;
    int m_height = 0;
    bool m_shown = false;
    bool m_faded = false;
    bool m_retry = false;
};
