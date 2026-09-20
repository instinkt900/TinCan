#include "system_formation_editor.h"
#include "debug_draw_path.h"
#include "formation_offsets.h"
#include "gamedata_enemy.h"
#include "gamedata_formation.h"
#include "system_drawable.h"
#include "utils_imgui.h"
#include <algorithm>
#include <array>
#include <fstream>
#include <magic_enum.hpp>
#include <optional>
#include <spdlog/spdlog.h>

namespace {
    // the formations the offsets are generated from rather than read from the database. they have no slots
    // to edit, and their size comes from a spacing instead of a scale
    constexpr std::array<char const*, 4> BuiltInFormations{ "line", "column", "v", "grid" };

    // where the formations the editor saves are kept. the whole file is read, the named formation is
    // replaced or added, and the file is written back, so its formatting and ordering are not preserved
    char const* const FormationDatabasePath = "data/formation_database.json";

    bool SaveFormation(std::string const& name, FormationData const& data) {
        nlohmann::json json;
        {
            std::ifstream file(FormationDatabasePath);
            if (file.is_open()) {
                try {
                    file >> json;
                } catch (std::exception const& e) {
                    spdlog::error("Cannot save formation {}: {} does not parse: {}", name,
                                  FormationDatabasePath, e.what());
                    return false;
                }
            }
        }
        if (!json.is_object()) {
            json = nlohmann::json::object();
        }

        // keep whatever spelling of the category the file already uses
        auto categoryName = std::string(magic_enum::enum_name(GameDataCategory::Formations));
        for (auto const& [key, value] : json.items()) {
            if (magic_enum::enum_cast<GameDataCategory>(key, magic_enum::case_insensitive) ==
                GameDataCategory::Formations) {
                categoryName = key;
                break;
            }
        }
        json[categoryName][name] = data;

        std::ofstream out(FormationDatabasePath);
        if (!out.is_open()) {
            spdlog::error("Cannot save formation {}: unable to write {}", name, FormationDatabasePath);
            return false;
        }
        out << json.dump(4) << "\n";
        spdlog::info("Saved formation {} to {}", name, FormationDatabasePath);
        return true;
    }

    // Names one formation: the blank entry that starts a new one, then the entries of the database, then
    // the built-in generators that no entry has taken the name of. Returns true on the frame the selection
    // changes, and every time the blank entry is picked, so that it can be used to clear the form again.
    bool FormationCombo(GameData const& gamedata, std::string& currentName) {
        bool changed = false;
        if (ImGui::BeginCombo("Formation", currentName.c_str())) {
            if (ImGui::Selectable(imgui_ext::NewEntryLabel, currentName == imgui_ext::NewEntryLabel)) {
                currentName = imgui_ext::NewEntryLabel;
                changed = true;
            }
            auto const keys = gamedata.Get<FormationData>()->Keys();
            for (auto const& name : keys) {
                bool const selected = name == currentName;
                if (ImGui::Selectable(name.c_str(), selected) && !selected) {
                    currentName = name;
                    changed = true;
                }
            }
            for (auto const* builtIn : BuiltInFormations) {
                if (std::find(keys.begin(), keys.end(), builtIn) != keys.end()) {
                    continue;
                }
                bool const selected = currentName == builtIn;
                if (ImGui::Selectable(builtIn, selected) && !selected) {
                    currentName = builtIn;
                    changed = true;
                }
            }
            ImGui::EndCombo();
        }
        return changed;
    }

    // Names one enemy. The formation editor keeps its own choice, separate from the behaviour editor's.
    // Returns true on the frame the selection changes.
    bool EnemyCombo(GameData const& gamedata, std::string& currentName) {
        bool changed = false;
        if (ImGui::BeginCombo("Enemy", currentName.c_str())) {
            for (auto const& name : gamedata.Get<EnemyData>()->Keys()) {
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

    // the slot the mouse has hold of, or -1. picked in world units, where the handles are drawn
    int FindSlot(std::vector<moth::gfx::FloatVec2> const& positions, moth::gfx::FloatVec2 const& position) {
        int found = -1;
        float bestDistance = WaypointHandleRadius;
        for (int i = 0; i < static_cast<int>(positions.size()); ++i) {
            auto const distance = moth::gfx::Length(positions[i] - position);
            if (distance <= bestDistance) {
                bestDistance = distance;
                found = i;
            }
        }
        return found;
    }

    // A point in the world turned back into the unit space the slots are stored in, so that a slot lands
    // under the mouse at whatever scale and rotation the preview is using.
    moth::gfx::FloatVec2 WorldToSlot(moth::gfx::FloatVec2 const& world, moth::gfx::FloatVec2 const& scale,
                                     float rotation) {
        auto const unturned = moth::gfx::Rotate2D(world, -rotation);
        return { scale.x == 0.0f ? 0.0f : unturned.x / scale.x,
                 scale.y == 0.0f ? 0.0f : unturned.y / scale.y };
    }

    void DrawSlotMarker(moth::gfx::IGraphics& graphics, moth::gfx::FloatVec2 const& position, float size) {
        graphics.DrawLineF(position - moth::gfx::FloatVec2{ 0.0f, size },
                           position + moth::gfx::FloatVec2{ 0.0f, size });
        graphics.DrawLineF(position - moth::gfx::FloatVec2{ size, 0.0f },
                           position + moth::gfx::FloatVec2{ size, 0.0f });
    }
}

void SystemFormationEditor::Draw(GameWorld& world, moth::gfx::IGraphics& graphics) {
    auto const& gamedata = world.GetGameData();

    static std::string staticFormationName;
    static std::string staticSaveName;
    static FormationData staticFormationData;
    static int staticCount = 5;
    static float staticSpacing = 32.0f;
    static moth::gfx::FloatVec2 staticScale{ 32.0f, 32.0f };
    static float staticRotationDegrees = 0.0f;
    static int staticSelectedSlot = -1;
    static int staticDraggedSlot = -1;
    // whether the selection is a shape with slots, which is a database entry or a new one, rather than a
    // built-in generator. a new formation is not in the database, so a lookup cannot answer this
    static bool staticEditable = false;
    static std::string staticEnemyName;

    ImGui::Begin("Formation Editor");

    // the most recently focused editor window owns the world view. a click in the world view lands on no
    // imgui window, so it focuses neither and the owner does not change
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
        world.SetActiveEditorWindow(EditorWindow::Formation);
    }

    if (FormationCombo(gamedata, staticFormationName)) {
        // loading a named formation is a starting point for editing, not a live link to the database, and
        // the blank entry is the same starting point with no slots in it
        staticFormationData = FormationData{};
        auto const* loaded = gamedata.Get<FormationData>()->Find(staticFormationName);
        if (loaded != nullptr) {
            staticFormationData = *loaded;
        }
        staticEditable = loaded != nullptr || staticFormationName == imgui_ext::NewEntryLabel;
        staticSaveName = loaded != nullptr ? staticFormationName : std::string();
        staticSelectedSlot = -1;
        staticDraggedSlot = -1;
    }

    EnemyCombo(gamedata, staticEnemyName);

    bool const builtIn = !staticEditable;

    // the three things a map spawn applies to a formation, so that what is previewed is what it would place
    ImGui::InputInt("Count", &staticCount);
    staticCount = std::max(1, staticCount);
    if (builtIn) {
        ImGui::InputFloat("Spacing (px)", &staticSpacing);
    } else {
        ImGui::InputFloat2("Scale (px per unit)", static_cast<float*>(staticScale.data));
    }
    ImGui::SliderFloat("Rotation (deg, clockwise)", &staticRotationDegrees, -180.0f, 180.0f);

    if (staticFormationName.empty()) {
        ImGui::TextUnformatted("Choose a formation to preview or edit, or <new> to start one");
    } else if (builtIn) {
        ImGui::TextUnformatted("A built-in formation is generated from the count and the spacing, so it\n"
                               "has no slots to edit and cannot be saved");
    } else {
        ImGui::TextUnformatted("Slots (units):");
        auto& slots = staticFormationData.slots;
        for (int i = 0; i < static_cast<int>(slots.size()); /* skip */) {
            ImGui::PushID(i);

            ImGui::Text("%s Slot %d", i == staticSelectedSlot ? ">" : " ", i);
            ImGui::SameLine();
            ImGui::InputFloat2("##value", static_cast<float*>(slots[i].data));
            ImGui::SameLine();
            if (ImGui::SmallButton("X")) {
                slots.erase(slots.begin() + i);
                staticSelectedSlot = -1;
                staticDraggedSlot = -1;
                ImGui::PopID();
                continue;
            }

            ImGui::PopID();
            ++i;
        }
        if (ImGui::Button("Add slot")) {
            slots.emplace_back();
            staticSelectedSlot = static_cast<int>(slots.size()) - 1;
        }
    }

    ImGui::Separator();
    imgui_ext::InputText("Save as", staticSaveName);
    ImGui::SameLine();
    // a name that is not in the database adds a formation rather than replacing one
    ImGui::BeginDisabled(builtIn || staticSaveName.empty());
    if (ImGui::Button("Save")) {
        SaveFormation(staticSaveName, staticFormationData);
    }
    ImGui::EndDisabled();
    ImGui::TextUnformatted("Saved formations are picked up the next time the game starts");

    ImGui::End();

    // the world view belongs to the other editor window: this one is a panel only until it is focused again
    if (world.GetActiveEditorWindow() != EditorWindow::Formation) {
        return;
    }

    auto const rotation = moth::gfx::DegToRad(staticRotationDegrees);
    // the shape is previewed around the middle of the world, where there is room for it on every side
    auto const origin = static_cast<moth::gfx::FloatVec2>(world.GetWorldSize()) / 2.0f;

    // the whole shape is laid out, so that the slots beyond the count still show what it grows into
    std::vector<moth::gfx::FloatVec2> offsets;
    if (builtIn) {
        if (!staticFormationName.empty()) {
            offsets = GetFormationOffsets(gamedata, staticFormationName, staticCount, staticSpacing,
                                          staticScale, rotation);
        }
    } else {
        offsets = GetFormationOffsets(staticFormationData, static_cast<int>(staticFormationData.slots.size()),
                                      staticScale, rotation);
    }

    std::vector<moth::gfx::FloatVec2> positions;
    positions.reserve(offsets.size());
    for (auto const& offset : offsets) {
        positions.push_back(origin + offset);
    }

    if (!builtIn) {
        auto& slots = staticFormationData.slots;
        auto const& io = ImGui::GetIO();
        auto const mouseWorld = world.GetViewTransform().WindowToWorld({ io.MousePos.x, io.MousePos.y });

        // a click that lands on the editor panel belongs to the panel, but a drag already under way carries on
        if (!io.WantCaptureMouse) {
            auto const hovered = FindSlot(positions, mouseWorld);
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                if (hovered >= 0) {
                    staticSelectedSlot = hovered;
                    staticDraggedSlot = hovered;
                } else {
                    slots.push_back(WorldToSlot(mouseWorld - origin, staticScale, rotation));
                    staticSelectedSlot = static_cast<int>(slots.size()) - 1;
                }
            } else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && hovered >= 0) {
                slots.erase(slots.begin() + hovered);
                staticSelectedSlot = -1;
                staticDraggedSlot = -1;
            }
        }

        if (staticDraggedSlot >= 0) {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
                staticDraggedSlot < static_cast<int>(slots.size())) {
                slots[staticDraggedSlot] = WorldToSlot(mouseWorld - origin, staticScale, rotation);
            } else {
                staticDraggedSlot = -1;
            }
        }

        // the mouse may have added, moved or removed a slot, so the shape is laid out again before it is
        // drawn, rather than showing where it was at the top of the frame
        offsets = GetFormationOffsets(staticFormationData, static_cast<int>(slots.size()), staticScale,
                                      rotation);
        positions.clear();
        for (auto const& offset : offsets) {
            positions.push_back(origin + offset);
        }
    }

    // The sprite the filled slots are drawn with. Nothing is spawned: this is the picture only, and it is
    // built for the frame rather than kept, so that no texture outlives the graphics device.
    std::optional<ComponentSprite> previewSprite;
    if (auto const* enemyData = staticEnemyName.empty() ? nullptr : gamedata.Get<EnemyData>(staticEnemyName);
        enemyData != nullptr && enemyData->sprite.valid()) {
        previewSprite.emplace(*enemyData->sprite);
    }

    // A slot the count reaches is drawn as the enemy that would fill it, so that the shape is judged as it
    // will look on the screen. One beyond the count keeps its marker, faint and small, so the whole shape
    // is still visible at a smaller count, and a filled slot falls back to its own marker when there is no
    // enemy to draw.
    for (int i = 0; i < static_cast<int>(positions.size()); ++i) {
        bool const filled = i < staticCount;
        if (filled && previewSprite.has_value()) {
            DrawSprite(graphics, *previewSprite, positions[i], 0.0f);
            continue;
        }
        graphics.SetBlendMode(moth::gfx::BlendMode::Replace);
        if (i == staticSelectedSlot) {
            graphics.SetColor(moth::gfx::FromARGB(0xFF00FF00));
        } else {
            graphics.SetColor(filled ? moth::gfx::FromARGB(0xFFFFAA00) : ReferencePathColor());
        }
        DrawSlotMarker(graphics, positions[i], WaypointHandleRadius / (filled ? 2.0f : 4.0f));
    }
}
