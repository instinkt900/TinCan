#pragma once

#include "gamedata.h"
#include "sound_player.h"
#include <string>
#include <utility>
#include <vector>
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

// What the player brings into a level, kept so that a game over can start the level again with it.
struct LevelEntryState {
    std::vector<std::pair<std::string, int32_t>> m_weapons; // each weapon's progression and level index
    float m_power = 0.0f;
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

    float GetPlayerPower() const;
    int32_t GetPlayerScore() const;

    // makes the next ship after one was destroyed: it flies in from below the screen with the main gun at the level
    // it had, and nothing else
    void RespawnPlayer(int32_t mainGunLevel);

    // whether the last life is lost and its wait for a ship is over
    bool IsGameOver() const;
    // starts the level again after a game over: the map from its start, with the starting lives, the weapons and
    // power the player entered the level with, and no score
    void RestartLevel();

    GameWorld(const GameWorld&) = delete;
    GameWorld(GameWorld&&) = delete;
    GameWorld& operator=(const GameWorld&) = delete;
    GameWorld& operator=(GameWorld&&) = delete;

private:
    bool OnKeyEvent(moth::ui::EventKey const& event);
    void StartLevel();
    void CreatePlayer(std::vector<std::pair<std::string, int32_t>> const& weapons, float power, bool flyIn);

    moth::gfx::platform::Window& m_window;
    moth::gfx::IGraphics& m_graphics;
    std::unique_ptr<moth::gfx::ITarget> m_worldSurface;
    std::unique_ptr<moth::gfx::IFont> m_font; // for text drawn in the world, such as the floating "1UP"

    GameData m_gamedata;
    std::unique_ptr<SoundPlayer> m_sounds;
    InputData const* m_input = nullptr;
    moth::ecs::World m_registry;
    moth::ecs::Entity m_player = moth::ecs::kNullEntity;

    WorldViewTransform m_viewTransform;
    LevelEntryState m_levelEntry;

    bool m_paused = false;
    bool m_editorModeOpen = false;
    EditorWindow m_activeEditorWindow = EditorWindow::Behaviour;

    // the world is a fixed height, its width comes from the loaded map
    static int constexpr WorldHeight = 1080;
    moth::gfx::IntVec2 m_worldSize{ 0, WorldHeight };
};
