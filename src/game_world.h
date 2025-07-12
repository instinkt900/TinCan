#pragma once

#include "gamedata.h"
#include <canyon/platform/window.h>
#include <entt/entt.hpp>
#include <moth_ui/events/event_key.h>

class GameWorld : public moth_ui::EventListener {
public:
    GameWorld(canyon::platform::Window& window, canyon::graphics::IGraphics& graphics);
    ~GameWorld() override = default;

    bool OnEvent(moth_ui::Event const& event) override;
    void Update(uint32_t ticks);
    void Draw();

    canyon::graphics::IImage& GetWorldImage() const { return *m_worldSurface->GetImage(); }

    float GetPlayerHealth() const;

    GameWorld(const GameWorld&) = delete;
    GameWorld(GameWorld&&) = delete;
    GameWorld& operator=(const GameWorld&) = delete;
    GameWorld& operator=(GameWorld&&) = delete;

private:
    bool OnKeyEvent(moth_ui::EventKey const& event);
    void CreatePlayer();

    canyon::platform::Window& m_window;
    canyon::graphics::IGraphics& m_graphics;
    std::unique_ptr<canyon::graphics::ITarget> m_worldSurface;
    entt::registry m_registry;
    entt::entity m_player = entt::null;
    entt::entity m_enemySpawner = entt::null;

    static canyon::IntVec2 const WorldSize;

    Gamedata m_gamedata;
};
