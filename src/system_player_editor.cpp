#include "system_player_editor.h"
#include "gamedata_weapon.h"
#include "system_shield.h"
#include "system_weapon.h"
#include <algorithm>
#include <cfloat>
#include <imgui.h>
#include <string>

namespace {
    // Names one weapon from the database, so that it can be given to the player. Unlike the editors
    // that write data back, there is no <new> entry: only a weapon the game can look up can be added.
    void WeaponCombo(GameData const& gamedata, std::string& currentName) {
        // nothing is picked until the panel is used, and a blank preview reads as a broken combo
        char const* const preview = currentName.empty() ? "<none>" : currentName.c_str();
        if (!ImGui::BeginCombo("Weapon", preview)) {
            return;
        }
        auto const keys = gamedata.Get<WeaponData>()->Keys();
        for (auto const& name : keys) {
            bool const selected = name == currentName;
            if (ImGui::Selectable(name.c_str(), selected)) {
                currentName = name;
            }
        }
        ImGui::EndCombo();
    }

    void DrawPower(moth::ecs::World& registry, moth::ecs::Entity player) {
        auto* power = registry.TryGet<ComponentPower>(player);
        if (power == nullptr) {
            ImGui::TextUnformatted("The player has no power component");
            return;
        }

        // power has no ceiling in the game, so this is a drag rather than a slider
        ImGui::DragFloat("Power", &power->m_power, 1.0f, 0.0f, FLT_MAX);
        power->m_power = std::max(0.0f, power->m_power);
    }

    void DrawWeapons(GameWorld& world, moth::ecs::World& registry, moth::ecs::Entity player) {
        auto& weapons = registry.GetOrEmplace<ComponentWeapon>(player);

        if (weapons.m_weapons.empty()) {
            ImGui::TextUnformatted("The player is carrying nothing");
        }

        // the row that asked to go is removed after the list is drawn, so the vector is not resized
        // underneath the loop
        int removeIndex = -1;
        for (size_t i = 0; i < weapons.m_weapons.size(); ++i) {
            auto const& weapon = weapons.m_weapons[i];
            ImGui::PushID(static_cast<int>(i));
            if (ImGui::SmallButton("Remove")) {
                removeIndex = static_cast<int>(i);
            }
            ImGui::SameLine();
            // a weapon written into a player or enemy entry rather than named has nothing to show
            ImGui::TextUnformatted(weapon.m_name.empty() ? "<unnamed>" : weapon.m_name.c_str());
            ImGui::PopID();
        }
        if (removeIndex >= 0) {
            weapons.m_weapons.erase(weapons.m_weapons.begin() + removeIndex);
        }

        static std::string staticWeaponName;
        WeaponCombo(world.GetGameData(), staticWeaponName);
        ImGui::SameLine();
        if (ImGui::Button("Add") && !staticWeaponName.empty()) {
            auto const& gamedata = world.GetGameData();
            auto const* weaponData = gamedata.Get<WeaponData>(staticWeaponName);
            if (weaponData != nullptr) {
                // AddWeapon ignores a weapon the player already carries
                SystemWeapon::AddWeapon(registry, player, staticWeaponName, *weaponData, gamedata);
            }
        }
    }
}

void SystemPlayerEditor::Draw(GameWorld& world) {
    ImGui::Begin("Player Editor");

    auto& registry = world.GetRegistry();
    auto const player = world.GetPlayerEntity();
    // the player entity is destroyed on death and only remade with the world, so there are stretches
    // with no player to edit
    if (!registry.Valid(player)) {
        ImGui::TextUnformatted("There is no player in the world");
        ImGui::End();
        return;
    }

    DrawPower(registry, player);

    ImGui::Separator();
    DrawWeapons(world, registry, player);

    ImGui::End();
}
