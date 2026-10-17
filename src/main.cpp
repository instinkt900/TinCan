#include <moth/graphics/platform/glfw/glfw_platform.h>
#include "game_application.h"
#include "game_layer.h"

void start(moth::gfx::platform::IPlatform& platform) {
    auto application = std::make_unique<GameApplication>(platform, "TinCan", 640, 480);
    application->Init();
    auto* uiWindow = application->GetUiWindow();
    auto& mothContext = uiWindow->GetMothContext();
    auto& graphics = uiWindow->GetGraphics();

    // the fonts are loaded before any layout that names them
    mothContext.GetFontFactory().LoadProject("assets/fonts.json");

    uiWindow->PushLayer(std::make_unique<GameLayer>(uiWindow->GetWindow(), mothContext, graphics));

    // start the application
    application->Run();
}

int main(int argc, char* argv[]) {
    // auto platform = std::make_unique<moth::gfx::platform::sdl::Platform>();
    auto platform = std::make_unique<moth::gfx::platform::glfw::Platform>();
    platform->Startup();

    // startExampleApp(*platform);
    // exampleMain(*platform);
    start(*platform);

    platform->Shutdown();
    return 0;
}

