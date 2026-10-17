#pragma once

#include "gamedata.h"
#include "sound_player.h"
#include "world_view.h"
#include <moth/graphics/graphics/itarget.h>
#include <moth/graphics/platform/window.h>
#include <moth/ecs/world.h>
#include <moth/ui/events/event_key.h>
#include <moth/ui/events/event_listener.h>

// The two editor windows. Editor mode shows both, but only one owns the world view at a time: it draws
// its overlay there and the mouse edits its shape, so two sets of handles never share the same clicks.
enum class EditorWindow {
    Behaviour,
    Formation,
};

class GameWorld : public moth::ui::IEventListener {
public:
    GameWorld(moth::gfx::platform::Window& window, moth::gfx::IGraphics& graphics);
    ~GameWorld() override = default;

    GameData const& GetGameData() const { return m_gamedata; }
    moth::ecs::World& GetRegistry() { return m_registry; }
    moth::ecs::World const& GetRegistry() const { return m_registry; }
    moth::ecs::Entity GetPlayerEntity() const { return m_player; }
    SoundPlayer& GetSounds() { return *m_sounds; }
    moth::gfx::IntVec2 const& GetWorldSize() const { return m_worldSize; }

    // editor mode freezes the map scroll while it is open, so spawns hold still while editing. the
    // behaviour editor owns the world view each time it opens
    void ToggleEditorMode() {
        m_editorModeOpen = !m_editorModeOpen;
        if (m_editorModeOpen) {
            m_activeEditorWindow = EditorWindow::Behaviour;
        }
    }

    // the editor window that owns the world view: the most recently focused of the two
    EditorWindow GetActiveEditorWindow() const { return m_activeEditorWindow; }
    void SetActiveEditorWindow(EditorWindow window) { m_activeEditorWindow = window; }

    bool OnEvent(moth::ui::Event const& event) override;
    void Update(uint32_t ticks);
    void Draw();

    moth::gfx::Image GetWorldImage() const { return m_worldSurface->GetImage(); }

    // set by the draw each frame, and read by the behaviour editor to place the mouse in the world
    void SetViewTransform(WorldViewTransform const& transform) { m_viewTransform = transform; }
    WorldViewTransform const& GetViewTransform() const { return m_viewTransform; }

    // the key bindings the world loaded, for the game shell to read its own actions from
    InputData const& GetInputBindings() const;

    float GetPlayerHealth() const;
    float GetPlayerPower() const;

    GameWorld(const GameWorld&) = delete;
    GameWorld(GameWorld&&) = delete;
    GameWorld& operator=(const GameWorld&) = delete;
    GameWorld& operator=(GameWorld&&) = delete;

private:
    bool OnKeyEvent(moth::ui::EventKey const& event);
    void CreatePlayer();

    moth::gfx::platform::Window& m_window;
    moth::gfx::IGraphics& m_graphics;
    std::unique_ptr<moth::gfx::ITarget> m_worldSurface;

    GameData m_gamedata;
    std::unique_ptr<SoundPlayer> m_sounds;
    InputData const* m_input = nullptr;
    moth::ecs::World m_registry;
    moth::ecs::Entity m_player = moth::ecs::kNullEntity;

    WorldViewTransform m_viewTransform;

    bool m_paused = false;
    bool m_editorModeOpen = false;
    EditorWindow m_activeEditorWindow = EditorWindow::Behaviour;

    // the world is a fixed height, its width comes from the loaded map
    static int constexpr WorldHeight = 1080;
    moth::gfx::IntVec2 m_worldSize{ 0, WorldHeight };
};
