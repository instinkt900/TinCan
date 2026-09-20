#pragma once

#include <moth/bridge/application.h>
#include <moth/profile/profiler.h>

// The profiler frame runs from one Tick to the next, so it covers the whole loop
// iteration: the fixed updates, the draw, the present and the ticker's sleep.
// Whatever is left untracked is the sleep and the loop overhead around these scopes.
class GameApplication : public moth::bridge::Application {
public:
    using moth::bridge::Application::Application;

    void TickFixed(uint32_t ticks) override {
        MOTH_PROFILE_SCOPE("TickFixed");
        auto* uiWindow = GetUiWindow();
        // window update pumps the platform event queue and dispatches input
        { MOTH_PROFILE_SCOPE("Window update"); uiWindow->GetWindow().Update(ticks); }
        { MOTH_PROFILE_SCOPE("Layer update"); uiWindow->GetLayerStack().Update(ticks); }
    }

    void Tick(uint32_t /*ticks*/) override {
        MOTH_PROFILE_FRAME();
        MOTH_PROFILE_SCOPE("Tick");
        auto* uiWindow = GetUiWindow();
        auto* imgui = uiWindow->HasImGuiContext() ? &uiWindow->GetImGuiContext() : nullptr;
        if (imgui != nullptr) {
            MOTH_PROFILE_SCOPE("ImGui new frame");
            imgui->NewFrame();
        }
        // begin frame acquires the swapchain image, so it can block on the presentation queue
        { MOTH_PROFILE_SCOPE("Begin frame"); uiWindow->BeginFrame(); }
        { MOTH_PROFILE_SCOPE("Layer draw"); uiWindow->GetLayerStack().Draw(); }
        if (imgui != nullptr) {
            MOTH_PROFILE_SCOPE("ImGui render");
            imgui->Render(uiWindow->GetGraphics());
        }
        // end frame submits and presents
        { MOTH_PROFILE_SCOPE("End frame"); uiWindow->EndFrame(); }
    }
};
