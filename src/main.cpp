#include <moth_graphics/platform/glfw/glfw_platform.h>
#include <moth_graphics/platform/application.h>
#include "game_layer.h"

void start(moth_graphics::platform::IPlatform& platform) {
    auto application = std::make_unique<moth_graphics::platform::Application>(platform, "Example", 640, 480);
    application->Init();
    auto* window = application->GetWindow();
    auto& mothContext = window->GetMothContext();
    auto& graphics = window->GetGraphics();
    window->GetLayerStack().SetEventListener(application.get());
    //
    // // load the fonts before layouts
    // mothContext.GetFontFactory().LoadProject("assets/fonts.json");
    //
    // // add an example layer
    window->GetLayerStack().PushLayer(std::make_unique<GameLayer>(*window, mothContext, graphics));

    // start the application
    application->Run();
}

int main(int argc, char* argv[]) {
    // auto platform = std::make_unique<moth_graphics::platform::sdl::Platform>();
    auto platform = std::make_unique<moth_graphics::platform::glfw::Platform>();
    platform->Startup();

    // startExampleApp(*platform);
    // exampleMain(*platform);
    start(*platform);

    platform->Shutdown();
    return 0;
}

