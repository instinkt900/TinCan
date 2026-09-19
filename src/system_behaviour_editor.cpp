#include "system_behaviour_editor.h"
#include "system_debug_spline.h"
#include "system_enemy_spawner.h"
#include "gamedata_behaviour.h"
#include "utils_imgui.h"
#include <algorithm>

namespace {
    // A state is incomplete while it is still being built in the editor. Spawning one would put an entity
    // in the world that cannot do what its state asks.
    bool IsStateComplete(BehaviourStateData const& state) {
        switch (state.type) {
        case BehaviourType::Line:
        case BehaviourType::Hold:
        case BehaviourType::Wave:
            return true;
        case BehaviourType::Target:
            return state.target != TargetType::Undefined;
        case BehaviourType::Spline:
            return state.points.size() >= 2;
        case BehaviourType::Undefined:
            break;
        }
        return false;
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
    void InputBehaviourStateData(BehaviourStateData& data, GameData const& gamedata,
                                 moth::gfx::IGraphics& graphics) {
        ImGui::PushID(&data);
        InputText("Label", data.label);
        ImGui::InputScalar("Duration", ImGuiDataType_U32, &data.duration);
        EnumCombo("Type", data.type);

        if (data.type == BehaviourType::Hold) {
        } else if (data.type == BehaviourType::Line) {
            ImGui::InputFloat2("Velocity", static_cast<float*>(data.velocity.data));
        } else if (data.type == BehaviourType::Target) {
            EnumCombo("Target", data.target);
            ImGui::InputFloat2("Position", static_cast<float*>(data.point.data));
            ImGui::InputFloat("Speed", &data.speed);
        } else if (data.type == BehaviourType::Wave) {
            ImGui::InputFloat("Speed", &data.speed);
            ImGui::InputFloat("Frequency", &data.frequency);
            ImGui::InputFloat("Amplitude", &data.amplitude);
        } else if (data.type == BehaviourType::Spline) {
            ImGui::InputFloat("Speed", &data.speed);
            for (int i = 0; i < static_cast<int>(data.points.size()); /* skip */) {
                ImGui::PushID(i);

                ImGui::Text("Point %d", i);
                ImGui::SameLine();
                ImGui::InputFloat2("##value", static_cast<float*>(data.points[i].data));
                ImGui::SameLine();
                if (ImGui::SmallButton("X")) {
                    data.points.erase(data.points.begin() + i);
                    ImGui::PopID();
                    continue;
                }

                DrawSpline(graphics, data.points);

                ImGui::PopID();
                ++i;
            }
            static moth::gfx::FloatVec2 pendingPoint;
            ImGui::TextUnformatted("New point:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(150.0f);
            ImGui::InputFloat2("##value", static_cast<float*>(pendingPoint.data));
            ImGui::SameLine();
            if (ImGui::Button("Add")) {
                data.points.push_back(pendingPoint);
            }
        }
        ImGui::PopID();
    }

    void InputBehaviourData(BehaviourData& data, GameData const& gamedata,
                            moth::gfx::IGraphics& graphics) {
        ImGui::Text("States:");
        for (int i = 0; i < static_cast<int>(data.states.size()); /* skip */) {
            ImGui::PushID(i);
            ImGui::Text("%d.", i);
            ImGui::Indent();
            InputBehaviourStateData(data.states[i], gamedata, graphics);
            if (ImGui::Button("Delete")) {
                data.states.erase(data.states.begin() + i);
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
        InputBehaviourStateData(newStateData, gamedata, graphics);
        if (ImGui::Button("Add")) {
            data.states.push_back(newStateData);
            newStateData = {};
        }
    }
}

void SystemBehaviourEditor::Draw(GameWorld& world, moth::gfx::IGraphics& graphics) {
    auto& registry = world.GetRegistry();
    auto const& gamedata = world.GetGameData();

    static std::string staticEnemyName;
    static std::string staticBehaviourName;
    static BehaviourData staticBehaviourData;
    static moth::gfx::FloatVec2 staticPosition;

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
        }
    }

    imgui_ext::InputBehaviourData(staticBehaviourData, gamedata, graphics);

    ImGui::End();
}
