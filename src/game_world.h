#pragma once

#include "gamedata.h"
#include <moth/graphics/graphics/itarget.h>
#include <moth/graphics/platform/window.h>
#include <moth/ecs/world.h>
#include <moth/ui/events/event_key.h>
#include <moth/ui/events/event_listener.h>

class GameWorld : public moth::ui::IEventListener {
public:
    GameWorld(moth::gfx::platform::Window& window, moth::gfx::IGraphics& graphics);
    ~GameWorld() override = default;

    GameData const& GetGameData() const { return m_gamedata; }
    moth::ecs::World& GetRegistry() { return m_registry; }
    moth::ecs::World const& GetRegistry() const { return m_registry; }
    moth::ecs::Entity GetPlayerEntity() const { return m_player; }
    moth::gfx::IntVec2 const& GetWorldSize() const { return m_worldSize; }

    // the behaviour editor freezes the map scroll while it is open, so spawns hold still while editing
    void ToggleBehaviourEditor() { m_behaviourEditorOpen = !m_behaviourEditorOpen; }

    bool OnEvent(moth::ui::Event const& event) override;
    void Update(uint32_t ticks);
    void Draw();

    moth::gfx::Image GetWorldImage() const { return m_worldSurface->GetImage(); }

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
    moth::ecs::World m_registry;
    moth::ecs::Entity m_player = moth::ecs::kNullEntity;

    bool m_paused = false;
    bool m_behaviourEditorOpen = false;

    // the world is a fixed height, its width comes from the loaded map
    static int constexpr WorldHeight = 1080;
    moth::gfx::IntVec2 m_worldSize{ 0, WorldHeight };
};
