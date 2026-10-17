#include <moth/graphics/platform/glfw/glfw_platform.h>
#include "game_application.h"
#include "game_layer.h"
#include <string>
#include <vector>

void start(moth::gfx::platform::IPlatform& platform, std::string const& mapPath) {
    auto application = std::make_unique<GameApplication>(platform, "TinCan", 640, 480);
    application->Init();
    auto* uiWindow = application->GetUiWindow();
    auto& mothContext = uiWindow->GetMothContext();
    auto& graphics = uiWindow->GetGraphics();

    // the fonts are loaded before any layout that names them
    mothContext.GetFontFactory().LoadProject("assets/fonts.json");

    uiWindow->PushLayer(std::make_unique<GameLayer>(uiWindow->GetWindow(), mothContext, graphics, mapPath));

    // start the application
    application->Run();
}

int main(int argc, char* argv[]) {
    // --map <path> plays another map, such as the test map
    std::string mapPath = "tiled_project/maps/stars.tmj";
    std::vector<std::string> const args(argv, argv + argc);
    for (size_t i = 1; i + 1 < args.size(); ++i) {
        if (args[i] == "--map") {
            mapPath = args[i + 1];
        }
    }

    // auto platform = std::make_unique<moth::gfx::platform::sdl::Platform>();
    auto platform = std::make_unique<moth::gfx::platform::glfw::Platform>();
    platform->Startup();

    // startExampleApp(*platform);
    // exampleMain(*platform);
    start(*platform, mapPath);

    platform->Shutdown();
    return 0;
}

