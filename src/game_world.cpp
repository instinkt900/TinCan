#include "game_world.h"
#include "component_body.h"
#include "component_contact.h"
#include "component_entity.h"
#include "component_passives.h"
#include "gamedata_entity.h"
#include "gamedata_weapon.h"
#include "gamedata_input.h"
#include "gamedata_player.h"
#include "system_animation.h"
#include "system_behaviour.h"
#include "system_contact.h"
#include "system_drawable.h"
#include "system_group.h"
#include "component_health.h"
#include "system_input.h"
#include "system_lifetime.h"
#include "system_map.h"
#include "system_movement.h"
#include "system_pickup.h"
#include "system_projectile.h"
#include "system_shield.h"
#include "system_behaviour_editor.h"
#include "system_formation_editor.h"
#include "system_targeting.h"
#include "system_weapon.h"
#include "system_world_bounds.h"
#include "tags.h"
#include "system_power_weapon.h"
#include <moth/graphics/graphics/igraphics_device.h>
#include <moth/ui/events/event_dispatch.h>
#include <moth/profile/profiler.h>

GameWorld::GameWorld(moth::gfx::platform::Window& window, moth::gfx::IGraphics& graphics)
    : m_window(window)
    , m_graphics(graphics) {
    SerializeContext serializeContext{ m_gamedata, m_window.GetSurfaceContext() };
    m_gamedata.LoadDirectory("data", serializeContext);
    m_input = m_gamedata.GetInput();

    // m_worldSize = SystemMap::InitMap(m_registry, "tiled_project/maps/test.tmj", "tiled_project/tincan.tiled-project",
    m_worldSize = SystemMap::InitMap(m_registry, "tiled_project/maps/stars.tmj", "tiled_project/tincan.tiled-project",
                                     m_window.GetSurfaceContext().GetAssetContext(), WorldHeight);
    m_worldSurface = m_window.GetDevice().CreateTarget(m_worldSize.x, m_worldSize.y);
    // InitTestWorld();
    CreatePlayer();
}

bool GameWorld::OnEvent(moth::ui::Event const& event) {
    moth::ui::EventDispatch dispatch(event);
    dispatch.Dispatch(this, &GameWorld::OnKeyEvent);
    bool handled = dispatch.GetHandled();
    return handled;
}

void GameWorld::Update(uint32_t ticks) {
    if (m_paused) {
        // the presses still have to be cleared, or they fire on the frame the pause lifts
        SystemInput::EndFrame(*this);
        return;
    }
    { MOTH_PROFILE_SCOPE("SystemInput"); SystemInput::Update(*this, ticks); }
    if (!m_editorModeOpen) {
        MOTH_PROFILE_SCOPE("SystemMap::Update");
        SystemMap::Update(*this, ticks);
    }
    { MOTH_PROFILE_SCOPE("SystemLifetime"); SystemLifetime::Update(*this, ticks); }
    { MOTH_PROFILE_SCOPE("SystemBehaviour"); SystemBehaviour::Update(*this, ticks); }
    { MOTH_PROFILE_SCOPE("SystemTargeting"); SystemTargeting::Update(*this, ticks); }
    { MOTH_PROFILE_SCOPE("SystemMovement"); SystemMovement::Update(*this, ticks); }
    { MOTH_PROFILE_SCOPE("SystemWeapon"); SystemWeapon::Update(*this, ticks); }
    { MOTH_PROFILE_SCOPE("SystemPowerWeapon"); SystemPowerWeapon::Update(*this, ticks); }
    { MOTH_PROFILE_SCOPE("SystemShield"); SystemShield::Update(*this, ticks); }
    { MOTH_PROFILE_SCOPE("SystemProjectile"); SystemProjectile::Update(*this, ticks); }
    { MOTH_PROFILE_SCOPE("SystemContact"); SystemContact::Update(*this, ticks); }
    { MOTH_PROFILE_SCOPE("SystemPickup"); SystemPickup::Update(*this, ticks); }
    { MOTH_PROFILE_SCOPE("SystemGroup"); SystemGroup::Update(*this, ticks); }
    { MOTH_PROFILE_SCOPE("SystemWorldBounds"); SystemWorldBounds::Update(*this, ticks); }
    { MOTH_PROFILE_SCOPE("SystemAnimation"); SystemAnimation::Update(*this, ticks); }
    { MOTH_PROFILE_SCOPE("SystemDrawable"); SystemDrawable::Update(*this, ticks); }
    SystemInput::EndFrame(*this);
}

void GameWorld::Draw() {
    { MOTH_PROFILE_SCOPE("Set world target"); m_graphics.SetTarget(m_worldSurface.get()); }
    m_graphics.SetColor(moth::gfx::BasicColors::Black);
    m_graphics.Clear();
    m_graphics.SetBlendMode(moth::gfx::BlendMode::Replace);
    m_graphics.SetColor(moth::gfx::BasicColors::White);
    if (m_editorModeOpen) {
        SystemBehaviourEditor::Draw(*this, m_graphics);
        SystemFormationEditor::Draw(*this, m_graphics);
    }
    { MOTH_PROFILE_SCOPE("SystemMap::Draw"); SystemMap::Draw(*this, m_graphics); }
    { MOTH_PROFILE_SCOPE("SystemDrawable"); SystemDrawable::Draw(*this, m_graphics); }
    { MOTH_PROFILE_SCOPE("Reset target"); m_graphics.SetTarget(nullptr); }
}

float GameWorld::GetPlayerHealth() const {
    if (auto const* playerHealth = m_registry.TryGet<ComponentHealth>(m_player)) {
        return playerHealth->m_currentHealth;
    }
    return 0.0f;
}

float GameWorld::GetPlayerPower() const {
    if (auto const* playerPower = m_registry.TryGet<ComponentPower>(m_player)) {
        return playerPower->m_power;
    }
    return 0.0f;
}

InputData const& GameWorld::GetInputBindings() const {
    // A missing input database is reported when it is looked up. Nothing is bound in that case, so
    // the game takes no input rather than every caller needing a null check.
    static InputData const unbound;
    return m_input != nullptr ? *m_input : unbound;
}

bool GameWorld::OnKeyEvent(moth::ui::EventKey const& event) {
    auto const& bindings = GetInputBindings();
    if (event.GetAction() == moth::ui::KeyAction::Down
        && bindings.IsBound(InputAction::Pause, event.GetKey())) {
        m_paused = !m_paused;
    }
    SystemInput::OnKey(m_registry, bindings, event);
    return false;
}

void GameWorld::CreatePlayer() {
    auto const* playerData = m_gamedata.GetPlayer();
    if (playerData == nullptr) {
        // GetPlayer logs the missing entry. Without it there is nothing to build
        // the player from, so leave m_player null rather than guessing values.
        return;
    }

    m_player = m_registry.Create();

    m_registry.Emplace<ComponentEntity>(m_player, Team::Player, playerData->affinity);

    auto const position = moth::gfx::FloatVec2(m_worldSize.x / 2, m_worldSize.y - 160);
    m_registry.Emplace<ComponentPosition>(m_player, position);
    m_registry.Emplace<ComponentBody>(m_player, playerData->radius);

    if (playerData->sprite.valid()) {
        m_registry.Emplace<ComponentSprite>(m_player, *playerData->sprite);
    }
    m_registry.Emplace<ComponentHealth>(m_player, playerData->health);

    auto& velocity = m_registry.Emplace<ComponentVelocity>(m_player);
    velocity.m_velocity = { 0, 0 };
    m_registry.Emplace<ComponentMovement>(m_player, playerData->movement);

    m_registry.Emplace<ComponentInput>(m_player);

    auto& power = m_registry.Emplace<ComponentPower>(m_player);
    power.m_power = 0;

    m_registry.Emplace<ComponentPassives>(m_player);

    if (playerData->weapon.valid()) {
        SystemWeapon::InitWeapon(m_registry, m_player, *playerData->weapon, m_gamedata);
    }
    SystemPowerWeapon::InitWeapon(m_registry, m_player, "dummy", m_gamedata);

    if (playerData->contact.has_value()) {
        m_registry.Emplace<ComponentContact>(m_player, *playerData->contact);
    }

    m_registry.Emplace<PlayerTag>(m_player);
    m_registry.Emplace<TargetTag>(m_player);

    auto& bounds = m_registry.Emplace<ComponentBounds>(m_player);
    bounds.m_behaviour = BoundsBehaviour::Limit;
    bounds.m_bounds =
        moth::gfx::MakeRect(0.0f, 0.0f, static_cast<float>(m_worldSize.x), static_cast<float>(m_worldSize.y));

    if (playerData->shield_sprite.valid()) {
        auto shield = m_registry.Create();
        m_registry.Emplace<ComponentShield>(shield);
        m_registry.Emplace<ComponentPosition>(shield, position);
        m_registry.Emplace<ComponentBody>(shield, playerData->shield_radius);
        m_registry.Emplace<ComponentParenting>(shield, m_player);
        m_registry.Emplace<ComponentSprite>(shield, *playerData->shield_sprite);
    }
}
