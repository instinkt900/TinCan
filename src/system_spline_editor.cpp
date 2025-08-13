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
    }
    auto const samples = 100;
    auto const tStep = 1.0f / static_cast<float>(samples);
    graphics.SetColor(canyon::graphics::FromARGB(0xFFFFFF00));
    for (auto [entity, spline] : view.each()) {
        if (ImGui::CollapsingHeader(spline.m_name.c_str())) {
            if (ImGui::Button("New Node")) {
                spline.m_spline.push_back({});
            }
            for (uint i = 0; i < spline.m_spline.size(); ++i) {
                auto& node = spline.m_spline[i];
                auto nodeName = fmt::format("node {}##{}", i, spline.m_name);
                ImGui::DragFloat2(nodeName.c_str(), &node.x);
            }
        }
        if (spline.m_spline.size() > 2) {
            auto t = 0.0f;
            auto lastSample = SampleCatmullRomPath(spline.m_spline, t);
            for (auto i = 1; i <= samples; ++i) {
                t = tStep * static_cast<float>(i);
                auto const newSample = SampleCatmullRomPath(spline.m_spline, t);
                graphics.DrawLineF(lastSample, newSample);
                lastSample = newSample;
            }
        }
    }
    ImGui::End();
}
