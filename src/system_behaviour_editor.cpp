#include "system_behaviour_editor.h"
#include "debug_draw_path.h"
#include "system_enemy_spawner.h"
#include "gamedata_behaviour.h"
#include "utils_imgui.h"
#include <algorithm>
#include <fstream>
#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

namespace {
    // A state is incomplete while it is still being built in the editor. Spawning one would put an entity
    // in the world that cannot do what its state asks.
    bool IsStateComplete(BehaviourStateData const& state) {
        switch (state.type) {
        case BehaviourType::Hold:
            return true;
        case BehaviourType::Target:
            return state.target != TargetType::Undefined;
        case BehaviourType::Path:
            return state.waypoints.size() >= 2 &&
                   std::none_of(state.modifiers.begin(), state.modifiers.end(),
                                [](BehaviourModifierData const& modifier) {
                                    return modifier.type == ModifierType::Undefined;
                                });
        case BehaviourType::Undefined:
            break;
        }
        return false;
    }

    // where the behaviours the editor saves are kept. the whole file is read, the named behaviour is
    // replaced or added, and the file is written back, so its formatting and ordering are not preserved
    char const* const BehaviourDatabasePath = "data/behaviour_database.json";

    bool SaveBehaviour(std::string const& name, BehaviourData const& data) {
        nlohmann::json json;
        {
            std::ifstream file(BehaviourDatabasePath);
            if (file.is_open()) {
                try {
                    file >> json;
                } catch (std::exception const& e) {
                    spdlog::error("Cannot save behaviour {}: {} does not parse: {}", name,
                                  BehaviourDatabasePath, e.what());
                    return false;
                }
            }
        }
        if (!json.is_object()) {
            json = nlohmann::json::object();
        }

        // keep whatever spelling of the category the file already uses
        auto categoryName = std::string(magic_enum::enum_name(GameDataCategory::Behaviours));
        for (auto const& [key, value] : json.items()) {
            if (magic_enum::enum_cast<GameDataCategory>(key, magic_enum::case_insensitive) ==
                GameDataCategory::Behaviours) {
                categoryName = key;
                break;
            }
        }
        json[categoryName][name] = data;

        std::ofstream out(BehaviourDatabasePath);
        if (!out.is_open()) {
            spdlog::error("Cannot save behaviour {}: unable to write {}", name, BehaviourDatabasePath);
            return false;
        }
        out << json.dump(4) << "\n";
        spdlog::info("Saved behaviour {} to {}", name, BehaviourDatabasePath);
        return true;
    }

    // the waypoint the mouse has hold of, or -1
    int FindWaypoint(std::vector<moth::gfx::FloatVec2> const& waypoints,
                     moth::gfx::FloatVec2 const& position) {
        int found = -1;
        float bestDistance = WaypointHandleRadius;
        for (int i = 0; i < static_cast<int>(waypoints.size()); ++i) {
            auto const distance = moth::gfx::Length(waypoints[i] - position);
            if (distance <= bestDistance) {
                bestDistance = distance;
                found = i;
            }
        }
        return found;
    }

    bool IsBehaviourComplete(BehaviourData const& data) {
        return !data.states.empty() &&
               std::all_of(data.states.begin(), data.states.end(), IsStateComplete);
    }

    // Names one entry of a database. Returns true on the frame the selection changes.
    template <typename T>
    bool DatabaseCombo(char const* label, GameData const& gamedata, std::string& currentName) {
        bool changed = false;
        if (ImGui::BeginCombo(label, currentName.c_str())) {
            for (auto const& name : gamedata.Get<T>()->Keys()) {
                bool const selected = name == currentName;
                if (ImGui::Selectable(name.c_str(), selected) && !selected) {
                    currentName = name;
                    changed = true;
                }
            }
            ImGui::EndCombo();
        }
        return changed;
    }
}

namespace imgui_ext {
    void InputBehaviourStateData(BehaviourStateData& data, int selectedWaypoint) {
        ImGui::PushID(&data);
        InputText("Label", data.label);
        ImGui::InputScalar("Duration", ImGuiDataType_U32, &data.duration);
        EnumCombo("Type", data.type);

        if (data.type == BehaviourType::Hold) {
        } else if (data.type == BehaviourType::Target) {
            EnumCombo("Target", data.target);
            ImGui::InputFloat2("Position", static_cast<float*>(data.point.data));
            ImGui::InputFloat("Speed", &data.speed);
        } else if (data.type == BehaviourType::Path) {
            ImGui::InputFloat("Speed", &data.speed);
            EnumCombo("Interpolation", data.interpolation);
            for (int i = 0; i < static_cast<int>(data.waypoints.size()); /* skip */) {
                ImGui::PushID(i);

                ImGui::Text("%s Waypoint %d", i == selectedWaypoint ? ">" : " ", i);
                ImGui::SameLine();
                ImGui::InputFloat2("##value", static_cast<float*>(data.waypoints[i].data));
                ImGui::SameLine();
                if (ImGui::SmallButton("X")) {
                    data.waypoints.erase(data.waypoints.begin() + i);
                    ImGui::PopID();
                    continue;
                }

                ImGui::PopID();
                ++i;
            }
            static moth::gfx::FloatVec2 pendingPoint;
            ImGui::TextUnformatted("New waypoint:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(150.0f);
            ImGui::InputFloat2("##value", static_cast<float*>(pendingPoint.data));
            ImGui::SameLine();
            if (ImGui::Button("Add")) {
                data.waypoints.push_back(pendingPoint);
            }

            ImGui::TextUnformatted("Modifiers:");
            for (int i = 0; i < static_cast<int>(data.modifiers.size()); /* skip */) {
                ImGui::PushID(i);
                auto& modifier = data.modifiers[i];
                EnumCombo("Type", modifier.type);
                if (modifier.type == ModifierType::Wave) {
                    ImGui::InputFloat("Amplitude", &modifier.amplitude);
                    ImGui::InputFloat("Frequency", &modifier.frequency);
                } else if (modifier.type == ModifierType::Phase) {
                    ImGui::InputFloat("Cycles", &modifier.cycles);
                } else if (modifier.type == ModifierType::Ease) {
                    EnumCombo("Ease", modifier.ease);
                }
                if (ImGui::SmallButton("Remove")) {
                    data.modifiers.erase(data.modifiers.begin() + i);
                    ImGui::PopID();
                    continue;
                }
                ImGui::PopID();
                ++i;
            }
            if (ImGui::Button("Add modifier")) {
                data.modifiers.emplace_back();
            }
        }
        ImGui::PopID();
    }

    void InputBehaviourData(BehaviourData& data, int& selectedState, int selectedWaypoint) {
        EnumCombo("Space", data.space);
        ImGui::Text("States:");
        for (int i = 0; i < static_cast<int>(data.states.size()); /* skip */) {
            ImGui::PushID(i);
            ImGui::Text("%d.", i);
            if (data.states[i].type == BehaviourType::Path) {
                ImGui::SameLine();
                ImGui::RadioButton("Edit in view", &selectedState, i);
            }
            ImGui::Indent();
            InputBehaviourStateData(data.states[i], i == selectedState ? selectedWaypoint : -1);
            if (ImGui::Button("Delete")) {
                data.states.erase(data.states.begin() + i);
                if (selectedState >= static_cast<int>(data.states.size())) {
                    selectedState = -1;
                }
                ImGui::PopID();
                ImGui::Unindent();
                continue;
            }
            ImGui::PopID();
            ImGui::Unindent();
            ++i;
        }

        static BehaviourStateData newStateData;
        ImGui::Text("New state:");
        InputBehaviourStateData(newStateData, -1);
        if (ImGui::Button("Add")) {
            data.states.push_back(newStateData);
            if (newStateData.type == BehaviourType::Path) {
                // editing in the view follows the state that was just added
                selectedState = static_cast<int>(data.states.size()) - 1;
            }
            newStateData = {};
        }
    }
}

void SystemBehaviourEditor::Draw(GameWorld& world, moth::gfx::IGraphics& graphics) {
    auto& registry = world.GetRegistry();
    auto const& gamedata = world.GetGameData();

    static std::string staticEnemyName;
    static std::string staticBehaviourName;
    static std::string staticSaveName;
    static BehaviourData staticBehaviourData;
    static moth::gfx::FloatVec2 staticPosition;
    static int staticSelectedState = -1;
    static int staticSelectedWaypoint = -1;
    static int staticDraggedWaypoint = -1;

    ImGui::Begin("Behaviour Editor");
    ImGui::InputFloat2("Position", static_cast<float*>(staticPosition.data));

    auto const* enemyData = staticEnemyName.empty() ? nullptr : gamedata.Get<EnemyData>(staticEnemyName);
    bool const canSpawn = enemyData != nullptr && IsBehaviourComplete(staticBehaviourData);
    ImGui::BeginDisabled(!canSpawn);
    if (ImGui::Button("Spawn")) {
        SystemEnemySpawner::SpawnEnemy(registry, *enemyData, staticBehaviourData, staticPosition, gamedata);
    }
    ImGui::EndDisabled();
    if (!canSpawn) {
        ImGui::SameLine();
        if (enemyData == nullptr) {
            ImGui::TextUnformatted("Choose an enemy before spawning");
        } else {
            ImGui::TextUnformatted("Add a state, and finish every state, before spawning");
        }
    }

    ImGui::Separator();

    DatabaseCombo<EnemyData>("Enemy", gamedata, staticEnemyName);
    if (DatabaseCombo<BehaviourData>("Behaviour", gamedata, staticBehaviourName)) {
        // loading a named behaviour is a starting point for editing, not a live link to the database
        if (auto const* loaded = gamedata.Get<BehaviourData>(staticBehaviourName)) {
            staticBehaviourData = *loaded;
            staticSaveName = staticBehaviourName;
            staticSelectedState = -1;
            staticSelectedWaypoint = -1;
            staticDraggedWaypoint = -1;
        }
    }

    imgui_ext::InputText("Save as", staticSaveName);
    ImGui::SameLine();
    // a name that is not in the database adds a behaviour rather than replacing one
    ImGui::BeginDisabled(staticSaveName.empty());
    if (ImGui::Button("Save")) {
        SaveBehaviour(staticSaveName, staticBehaviourData);
    }
    ImGui::EndDisabled();
    ImGui::TextUnformatted("Saved behaviours are picked up the next time the game starts");

    imgui_ext::InputBehaviourData(staticBehaviourData, staticSelectedState, staticSelectedWaypoint);

    ImGui::End();

    // waypoints of the state being edited are drawn into the game view and edited with the mouse there,
    // because what is being judged is whether a movement feels right at the size the player sees it
    BehaviourStateData* editedState = nullptr;
    if (staticSelectedState >= 0 && staticSelectedState < static_cast<int>(staticBehaviourData.states.size())) {
        auto& state = staticBehaviourData.states[staticSelectedState];
        if (state.type == BehaviourType::Path) {
            editedState = &state;
        }
    }
    if (editedState == nullptr) {
        staticDraggedWaypoint = -1;
        return;
    }

    // a spawn-relative path is authored around wherever the enemy comes in, so it is drawn there too
    auto const origin = staticBehaviourData.space == BehaviourSpace::SpawnRelative
                            ? staticPosition
                            : moth::gfx::FloatVec2{ 0, 0 };
    auto& waypoints = editedState->waypoints;
    auto const& io = ImGui::GetIO();
    auto const mouseWorld =
        world.GetViewTransform().WindowToWorld({ io.MousePos.x, io.MousePos.y }) - origin;

    // a click that lands on the editor panel belongs to the panel, but a drag already under way carries on
    if (!io.WantCaptureMouse) {
        auto const hovered = FindWaypoint(waypoints, mouseWorld);
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            if (hovered >= 0) {
                staticSelectedWaypoint = hovered;
                staticDraggedWaypoint = hovered;
            } else {
                waypoints.push_back(mouseWorld);
                staticSelectedWaypoint = static_cast<int>(waypoints.size()) - 1;
            }
        } else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && hovered >= 0) {
            waypoints.erase(waypoints.begin() + hovered);
            staticSelectedWaypoint = -1;
            staticDraggedWaypoint = -1;
        }
    }

    if (staticDraggedWaypoint >= 0) {
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
            staticDraggedWaypoint < static_cast<int>(waypoints.size())) {
            waypoints[staticDraggedWaypoint] = mouseWorld;
        } else {
            staticDraggedWaypoint = -1;
        }
    }

    DrawPath(graphics, waypoints, editedState->interpolation, origin);
    DrawWaypointHandles(graphics, waypoints, origin, staticSelectedWaypoint);
}
