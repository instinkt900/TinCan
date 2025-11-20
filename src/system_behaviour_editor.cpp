#include "system_behaviour_editor.h"
#include "system_enemy_spawner.h"
#include "gamedata_behaviour.h"
#include "utils_imgui.h"

namespace imgui_ext {
    void InputEnemyData(EnemyData& data, GameData const& gamedata) {
        static std::string currentSpriteName;
        currentSpriteName = "";
        if (data.sprite.valid()) {
            currentSpriteName = data.sprite.ref_name();
        }
        if (ImGui::BeginCombo("Sprite", currentSpriteName.c_str())) {
            auto const& spriteDb = gamedata.Get<SpriteData>();
            for (auto const& spriteName : spriteDb->Keys()) {
                bool selected = spriteName == currentSpriteName;
                if (ImGui::Selectable(spriteName.c_str(), &selected)) {
                    data.sprite.load_ref(spriteName, gamedata);
                }
            }
            ImGui::EndCombo();
        }
        ImGui::InputFloat("Radius", &data.radius);
        ImGui::InputFloat("Health", &data.health);
    }

    void InputBehaviourStateData(BehaviourStateData& data, GameData const& gamedata) {
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

                ImGui::PopID();
                ++i;
            }
            static canyon::FloatVec2 pendingPoint;
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

    void InputBehaviourData(BehaviourData& data, GameData const& gamedata) {
        ImGui::Text("States:");
        for (int i = 0; i < static_cast<int>(data.states.size()); /* skip */) {
            ImGui::PushID(i);
            ImGui::Text("%d.", i);
            ImGui::Indent();
            InputBehaviourStateData(data.states[i], gamedata);
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
        InputBehaviourStateData(newStateData, gamedata);
        if (ImGui::Button("Add")) {
            data.states.push_back(newStateData);
            newStateData = {};
        }
    }
}

void SystemBehaviourEditor::Draw(GameWorld& world, canyon::graphics::IGraphics& graphics) {
    auto& registry = world.GetRegistry();
    auto const& gamedata = world.GetGameData();

    static EnemyData staticEnemyData;
    static BehaviourData staticBehaviourData;
    static canyon::FloatVec2 staticPosition;

    ImGui::Begin("Behaviour Editor");
    ImGui::InputFloat2("Position", static_cast<float*>(staticPosition.data));
    if (ImGui::Button("Spawn")) {
        SystemEnemySpawner::SpawnEnemy(registry, staticEnemyData, staticBehaviourData, staticPosition,
                                       gamedata);
    }

    ImGui::Separator();

    imgui_ext::InputEnemyData(staticEnemyData, gamedata);
    imgui_ext::InputBehaviourData(staticBehaviourData, gamedata);

    ImGui::End();
}
