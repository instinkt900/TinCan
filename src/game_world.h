#pragma once

#include "gamedata.h"
#include <moth_graphics/platform/window.h>
#include <entt/entt.hpp>
#include <moth_ui/events/event_key.h>

class GameWorld : public moth_ui::EventListener {
public:
    GameWorld(moth_graphics::platform::Window& window, moth_graphics::graphics::IGraphics& graphics);
    ~GameWorld() override = default;

    GameData const& GetGameData() const { return m_gamedata; }
    entt::registry& GetRegistry() { return m_registry; }
    entt::registry const& GetRegistry() const { return m_registry; }
    entt::entity GetPlayerEntity() const { return m_player; }
    moth_graphics::IntVec2 const& GetWorldSize() const { return WorldSize; }

    bool OnEvent(moth_ui::Event const& event) override;
    void Update(uint32_t ticks);
    void Draw();

    moth_graphics::graphics::IImage& GetWorldImage() const { return *m_worldSurface->GetImage(); }

    float GetPlayerHealth() const;
    float GetPlayerPower() const;

    GameWorld(const GameWorld&) = delete;
    GameWorld(GameWorld&&) = delete;
    GameWorld& operator=(const GameWorld&) = delete;
    GameWorld& operator=(GameWorld&&) = delete;

private:
    bool OnKeyEvent(moth_ui::EventKey const& event);
    void CreatePlayer();

    moth_graphics::platform::Window& m_window;
    moth_graphics::graphics::IGraphics& m_graphics;
    std::unique_ptr<moth_graphics::graphics::ITarget> m_worldSurface;

    GameData m_gamedata;
    entt::registry m_registry;
    entt::entity m_player = entt::null;

    bool m_paused = false;

    static moth_graphics::IntVec2 const WorldSize;
};
