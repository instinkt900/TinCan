#include "system_spline_editor.h"
#include "system_behaviour.h"
#include <fmt/format.h>
#include <imgui.h>

void SystemSplineEditor::Draw(GameWorld& world, canyon::graphics::IGraphics& graphics) {
    auto& registry = world.GetRegistry();
    auto view = registry.view<ComponentSpline>();

    ImGui::Begin("Splines");
    static char nameBuf[1024] = { 0 };
    ImGui::InputText("Spline name", nameBuf, 1024);
    if (ImGui::Button("Create")) {
        auto entity = registry.create();
        auto& spline = registry.emplace<ComponentSpline>(entity);
        spline.m_name = nameBuf;
        spline.m_spline.push_back({0.0f, 0.0f});
        spline.m_spline.push_back({600.0f, 800.0f});
    }

    for (auto [entity, spline] : view.each()) {
        if (ImGui::CollapsingHeader(spline.m_name.c_str())) {
            if (ImGui::Button("New Node")) {
                spline.m_spline.push_back({});
            }
            int insertAfter = -1;
            canyon::FloatVec2 newVal;
            float const iterStep = 1.0f / static_cast<float>(spline.m_spline.size());
            for (uint i = 0; i < spline.m_spline.size(); ++i) {
                auto const t = iterStep * static_cast<float>(i);
                auto& node = spline.m_spline[i];
                auto nodeName = fmt::format("node {}##{}", i, spline.m_name);
                ImGui::DragFloat2(nodeName.c_str(), &node.x);
                ImGui::SameLine();
                auto const buttonName = fmt::format("+##{}", nodeName);
                if (ImGui::Button(buttonName.c_str())) {
                    insertAfter = static_cast<int>(i);
                    newVal = SampleCatmullRomPath(spline.m_spline, t + (iterStep / 2.0f));
                }
            }
            if (insertAfter >= 0) {
                spline.m_spline.insert(spline.m_spline.begin() + insertAfter + 1, newVal);
            }
        }
        if (spline.m_spline.size() > 2) {
            auto const samples = 100;
            auto const tStep = 1.0f / static_cast<float>(samples);
            auto t = 0.0f;
            auto lastSample = SampleCatmullRomPath(spline.m_spline, t);
            graphics.SetColor(canyon::graphics::FromARGB(0xFFFFFF00));
            for (auto i = 1; i <= samples; ++i) {
                t = tStep * static_cast<float>(i);
                auto const newSample = SampleCatmullRomPath(spline.m_spline, t);
                graphics.DrawLineF(lastSample, newSample);
                lastSample = newSample;
            }
            graphics.SetColor(canyon::graphics::FromARGB(0xFFFF0000));
            for (uint i = 0; i < spline.m_spline.size(); ++i) {
                auto const pos = spline.m_spline[i];
                auto const v1 = pos - canyon::FloatVec2{0.0f, 6.0f};
                auto const v2 = pos + canyon::FloatVec2{0.0f, 6.0f};
                auto const h1 = pos - canyon::FloatVec2{6.0f, 0.0f};
                auto const h2 = pos + canyon::FloatVec2{6.0f, 0.0f};
                graphics.DrawLineF(v1, v2);
                graphics.DrawLineF(h1, h2);
            }
        }
    }
    ImGui::End();
}
