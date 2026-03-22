#include "system_debug_spline.h"
#include <imgui.h>

namespace imgui_ext {
    void Details(ComponentDebugSpline& spline) {
        ImGui::PushID(&spline);
        ImGui::Checkbox("Visible", &spline.m_visible);

        int insertAfter = -1;
        moth_graphics::FloatVec2 newVal;
        float const iterStep = 1.0f / static_cast<float>(spline.m_spline.size());
        for (uint i = 0; i < spline.m_spline.size(); ++i) {
            auto const t = iterStep * static_cast<float>(i);
            auto& node = spline.m_spline[i];
            auto nodeName = fmt::format("node {}", i);
            ImGui::PushID(&node);
            ImGui::DragFloat2(nodeName.c_str(), &node.x);
            ImGui::SameLine();
            auto const buttonName = fmt::format("+");
            if (ImGui::Button(buttonName.c_str())) {
                insertAfter = static_cast<int>(i);
                newVal = SampleCatmullRomPath(spline.m_spline, t + (iterStep / 2.0f));
            }
            ImGui::PopID();
        }
        if (insertAfter >= 0) {
            spline.m_spline.insert(spline.m_spline.begin() + insertAfter + 1, newVal);
        }

        nlohmann::json j = spline.m_spline;
        std::string s = j.dump();
        ImGui::InputTextMultiline("json", s.data(), s.size(), {}, ImGuiInputTextFlags_ReadOnly);

        ImGui::PopID();
    }
}

void DrawSpline(moth_graphics::graphics::IGraphics& graphics, std::vector<moth_graphics::FloatVec2> const& points,
                moth_graphics::FloatVec2 const& offset) {
    auto const samples = 100;
    auto const tStep = 1.0f / static_cast<float>(samples);
    auto t = 0.0f;
    auto lastSample = SampleCatmullRomPath(points, t);
    graphics.SetColor(moth_graphics::graphics::FromARGB(0xFFFFFF00));
    for (auto i = 1; i <= samples; ++i) {
        t = tStep * static_cast<float>(i);
        auto const newSample = SampleCatmullRomPath(points, t) + offset;
        graphics.DrawLineF(lastSample, newSample);
        lastSample = newSample;
    }
    graphics.SetColor(moth_graphics::graphics::FromARGB(0xFFFF0000));
    for (uint i = 0; i < points.size(); ++i) {
        auto const pos = points[i] + offset;
        auto const v1 = pos - moth_graphics::FloatVec2{ 0.0f, 6.0f };
        auto const v2 = pos + moth_graphics::FloatVec2{ 0.0f, 6.0f };
        auto const h1 = pos - moth_graphics::FloatVec2{ 6.0f, 0.0f };
        auto const h2 = pos + moth_graphics::FloatVec2{ 6.0f, 0.0f };
        graphics.DrawLineF(v1, v2);
        graphics.DrawLineF(h1, h2);
    }
}

void SystemDebugSpline::Draw(GameWorld& world, moth_graphics::graphics::IGraphics& graphics) {
    auto& registry = world.GetRegistry();
    auto view = registry.view<ComponentDebugSpline>();

    for (auto [entity, spline] : view.each()) {
        if (spline.m_visible && spline.m_spline.size() > 2) {
            DrawSpline(graphics, spline.m_spline);
        }
    }
}
