#include "system_behaviour_editor.h"
#include "behaviour_transform.h"
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

    // Where a waypoint added from the panel goes. It carries on in the direction the path was already
    // going, so that it lands clear of the handle before it and can be picked up and dragged, rather than
    // stacking on top of another one where neither can be grabbed.
    moth::gfx::FloatVec2 PlaceholderWaypoint(std::vector<moth::gfx::FloatVec2> const& waypoints) {
        moth::gfx::FloatVec2 const fallback{ 0.0f, 100.0f };
        if (waypoints.empty()) {
            return { 0.0f, 0.0f };
        }
        auto step = fallback;
        if (waypoints.size() >= 2) {
            auto const last = waypoints.back() - waypoints[waypoints.size() - 2];
            if (moth::gfx::LengthSq(last) > 0.0f) {
                step = last;
            }
        }
        return waypoints.back() + step;
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

    // Another behaviour loaded beside the one being edited and drawn as a reference, so that paths can be
    // put side by side. Read-only: the mouse code only ever touches the edited state's waypoints, so a
    // guide is never picked, placed or dragged. It shows the version in the database, not unsaved edits.
    struct BehaviourGuide {
        std::string name;
        float rotation{ 0.0f };
        moth::gfx::FloatVec2 anchor{ 0, 0 };
    };

    // Names one entry of a database, and with includeNew the blank entry that starts a new one. Returns
    // true on the frame the selection changes, and every time the blank entry is picked, so that it can be
    // used to clear the form again.
    template <typename T>
    bool DatabaseCombo(char const* label, GameData const& gamedata, std::string& currentName,
                       bool includeNew = false) {
        bool changed = false;
        if (ImGui::BeginCombo(label, currentName.c_str())) {
            if (includeNew &&
                ImGui::Selectable(imgui_ext::NewEntryLabel, currentName == imgui_ext::NewEntryLabel)) {
                currentName = imgui_ext::NewEntryLabel;
                changed = true;
            }
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
        ImGui::InputScalar("Duration (ms, 0 = until it completes)", ImGuiDataType_U32, &data.duration);
        EnumCombo("Type", data.type);

        if (data.type == BehaviourType::Hold) {
        } else if (data.type == BehaviourType::Target) {
            EnumCombo("Target", data.target);
            ImGui::InputFloat2("Position (px)", static_cast<float*>(data.point.data));
            ImGui::InputFloat("Speed (px/s)", &data.speed);
        } else if (data.type == BehaviourType::Path) {
            ImGui::InputFloat("Speed (px/s)", &data.speed);
            EnumCombo("Interpolation", data.interpolation);
            EnumCombo("Entry", data.entry.type);
            if (data.entry.type != TransitionType::Snap) {
                ImGui::InputFloat("Entry speed (px/s, 0 = this state's)", &data.entry.speed);
            }
            ImGui::TextUnformatted("Waypoints (px):");
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
            if (ImGui::Button("Add waypoint")) {
                data.waypoints.push_back(PlaceholderWaypoint(data.waypoints));
            }

            ImGui::TextUnformatted("Modifiers:");
            for (int i = 0; i < static_cast<int>(data.modifiers.size()); /* skip */) {
                ImGui::PushID(i);
                auto& modifier = data.modifiers[i];
                EnumCombo("Type", modifier.type);
                if (modifier.type == ModifierType::Wave) {
                    ImGui::InputFloat("Amplitude (px)", &modifier.amplitude);
                    ImGui::InputFloat("Frequency (cycles per path)", &modifier.frequency);
                } else if (modifier.type == ModifierType::Phase) {
                    ImGui::InputFloat("Cycles (per place in the formation)", &modifier.cycles);
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

        // a state is added empty and filled in where it sits in the list, rather than filled in first
        if (ImGui::Button("Add state")) {
            data.states.emplace_back();
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
    static float staticPreviewProgress = 0.0f;
    static bool staticPreviewPlaying = false;
    static float staticSpawnRotation = 0.0f;
    static std::vector<BehaviourGuide> staticGuides;
    static std::string staticGuideName;
    // whether the spawn position has been typed in by hand. until it has, it follows the anchor
    static bool staticPositionEdited = false;

    ImGui::Begin("Behaviour Editor");

    // the most recently focused editor window owns the world view. a click in the world view lands on no
    // imgui window, so it focuses neither and the owner does not change
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
        world.SetActiveEditorWindow(EditorWindow::Behaviour);
    }

    // what to spawn, and where
    DatabaseCombo<EnemyData>("Enemy", gamedata, staticEnemyName);

    // a spawn-relative path is re-anchored onto the spawn point when it spawns, so the spawn position
    // follows the anchor and the enemy flies the line that was just drawn. a value typed in by hand means
    // "fly this shape from somewhere else", and it stays put until another behaviour is loaded
    auto const anchor = BehaviourAnchor(staticBehaviourData);
    if (!anchor.has_value()) {
        staticPositionEdited = false;
    } else if (staticBehaviourData.space == BehaviourSpace::SpawnRelative && !staticPositionEdited) {
        staticPosition = anchor.value();
    }
    if (ImGui::InputFloat2("Position (px)", static_cast<float*>(staticPosition.data))) {
        staticPositionEdited = true;
    }

    auto const* enemyData = staticEnemyName.empty() ? nullptr : gamedata.Get<EnemyData>(staticEnemyName);
    bool const canSpawn = enemyData != nullptr && IsBehaviourComplete(staticBehaviourData);
    ImGui::BeginDisabled(!canSpawn);
    if (ImGui::Button("Spawn")) {
        // the enemy flies its own copy, anchored and turned the way a map spawn's copy would be
        auto spawnData = staticBehaviourData;
        TransformBehaviour(spawnData, staticSpawnRotation);
        SystemEnemySpawner::SpawnEnemy(registry, *enemyData, spawnData, staticPosition, gamedata);
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

    // the rotation feeds the spawn only: the path is authored unturned, and this is how a turned one is
    // watched flying. ctrl+click on the slider types a value into it
    ImGui::SliderFloat("Spawn rotation (deg, clockwise)", &staticSpawnRotation, -180.0f, 180.0f);

    ImGui::Separator();

    if (DatabaseCombo<BehaviourData>("Behaviour", gamedata, staticBehaviourName, true)) {
        // loading a named behaviour is a starting point for editing, not a live link to the database, and
        // the blank entry is the same starting point with nothing in it
        bool loadedSomething = false;
        if (staticBehaviourName == imgui_ext::NewEntryLabel) {
            staticBehaviourData = BehaviourData{};
            staticSaveName.clear();
            loadedSomething = true;
        } else if (auto const* loaded = gamedata.Get<BehaviourData>(staticBehaviourName)) {
            staticBehaviourData = *loaded;
            staticSaveName = staticBehaviourName;
            loadedSomething = true;
        }
        if (loadedSomething) {
            staticSelectedState = -1;
            staticSelectedWaypoint = -1;
            staticDraggedWaypoint = -1;
            staticPositionEdited = false;
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

    // waypoints of the state being edited are drawn into the game view and edited with the mouse there,
    // because what is being judged is whether a movement feels right at the size the player sees it
    BehaviourStateData* editedState = nullptr;
    if (staticSelectedState >= 0 && staticSelectedState < static_cast<int>(staticBehaviourData.states.size())) {
        auto& state = staticBehaviourData.states[staticSelectedState];
        if (state.type == BehaviourType::Path) {
            editedState = &state;
        }
    }
    // the arc-length table the edited state would build when it is entered, so that the preview and the
    // flight agree. rebuilt every frame, because the path is being edited every frame
    ArcLengthLUT previewLut{ { 0.0f }, { 0.0f } };
    if (editedState == nullptr) {
        staticDraggedWaypoint = -1;
        staticPreviewPlaying = false;
    } else {
        if (editedState->waypoints.size() >= 2) {
            previewLut = BuildArcLengthTable(editedState->waypoints, editedState->interpolation);
        }
        auto const previewLength = previewLut.distances.back();

        ImGui::Separator();
        ImGui::TextUnformatted("Preview:");
        if (ImGui::Button(staticPreviewPlaying ? "Pause" : "Play")) {
            staticPreviewPlaying = !staticPreviewPlaying;
        }
        ImGui::SameLine();
        ImGui::SliderFloat("Scrub (fraction of path)", &staticPreviewProgress, 0.0f, 1.0f);
        if (staticPreviewPlaying) {
            if (editedState->speed > 0.0f && previewLength > 0.0f) {
                // playback runs at the speed the state would fly at, which is what tells you how it feels
                staticPreviewProgress += (editedState->speed * ImGui::GetIO().DeltaTime) / previewLength;
                if (staticPreviewProgress > 1.0f) {
                    staticPreviewProgress -= 1.0f;
                }
            } else {
                ImGui::TextUnformatted("The state needs a speed and two waypoints before it can play");
            }
        }
        if (previewLength > 0.0f && editedState->speed > 0.0f) {
            ImGui::Text("%.2f seconds along the path, %.2f in all",
                        (staticPreviewProgress * previewLength) / editedState->speed,
                        previewLength / editedState->speed);
        }
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Guides (drawn only, never edited or saved):");
    DatabaseCombo<BehaviourData>("Guide behaviour", gamedata, staticGuideName);
    ImGui::BeginDisabled(staticGuideName.empty());
    if (ImGui::Button("Add guide")) {
        staticGuides.push_back({ staticGuideName, 0.0f, { 0.0f, 0.0f } });
    }
    ImGui::EndDisabled();
    for (int i = 0; i < static_cast<int>(staticGuides.size()); /* skip */) {
        ImGui::PushID(i);
        auto& guide = staticGuides[i];
        ImGui::TextUnformatted(guide.name.c_str());
        ImGui::SliderFloat("Rotation (deg, clockwise)", &guide.rotation, -180.0f, 180.0f);
        ImGui::InputFloat2("Anchor (px)", static_cast<float*>(guide.anchor.data));
        if (ImGui::SmallButton("Remove")) {
            staticGuides.erase(staticGuides.begin() + i);
            ImGui::PopID();
            continue;
        }
        ImGui::PopID();
        ++i;
    }

    ImGui::End();

    // the world view belongs to the other editor window: this one is a panel only until it is focused again
    if (world.GetActiveEditorWindow() != EditorWindow::Behaviour) {
        return;
    }

    // guides are drawn first, so the behaviour being edited sits on top of them. each one is transformed
    // the way it would be at a spawn, so what is drawn is the path it would fly
    for (auto const& guide : staticGuides) {
        auto const* loaded = gamedata.Get<BehaviourData>(guide.name);
        if (loaded == nullptr) {
            continue;
        }
        auto guideData = *loaded;
        TransformBehaviour(guideData, guide.rotation);
        auto const guideOrigin = guideData.space == BehaviourSpace::SpawnRelative
                                     ? guide.anchor
                                     : moth::gfx::FloatVec2{ 0, 0 };
        for (auto const& state : guideData.states) {
            if (state.type != BehaviourType::Path || state.waypoints.size() < 2) {
                continue;
            }
            auto const lut = BuildArcLengthTable(state.waypoints, state.interpolation);
            DrawFlownPath(graphics, state, lut, guideOrigin, GuidePathColor());
        }
    }

    // the canvas is pure authoring space: waypoints are drawn and picked at the coordinates they hold. a
    // spawn-relative path is moved onto the spawn point when it spawns, not while it is being authored
    moth::gfx::FloatVec2 const origin{ 0, 0 };

    // every other path in the behaviour is drawn faintly, so a path can be placed against the whole of it
    for (int i = 0; i < static_cast<int>(staticBehaviourData.states.size()); ++i) {
        auto const& state = staticBehaviourData.states[i];
        if (i == staticSelectedState || state.type != BehaviourType::Path || state.waypoints.size() < 2) {
            continue;
        }
        auto const lut = BuildArcLengthTable(state.waypoints, state.interpolation);
        DrawFlownPath(graphics, state, lut, origin, ReferencePathColor());
        DrawEntryApproach(graphics, staticBehaviourData, i, origin, ReferencePathColor());
    }

    if (editedState == nullptr) {
        return;
    }

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
    DrawEntryApproach(graphics, staticBehaviourData, staticSelectedState, origin, EntryPathColor());
    DrawPathStatePreview(graphics, *editedState, previewLut, origin, staticPreviewProgress);
    DrawWaypointHandles(graphics, waypoints, origin, staticSelectedWaypoint);
}
