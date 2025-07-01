#include <stdio.h>
#include <canyon/platform/glfw/glfw_platform.h>
#include <canyon/platform/application.h>
#include "game_layer.h"

void start(canyon::platform::IPlatform& platform) {
    auto application = std::make_unique<canyon::platform::Application>(platform, "Example", 640, 480);
    application->Init();
    auto window = application->GetWindow();
    auto& mothContext = window->GetMothContext();
    auto& graphics = window->GetGraphics();
    window->GetLayerStack().SetEventListener(application.get());
    //
    // // load the fonts before layouts
    // mothContext.GetFontFactory().LoadProject("assets/fonts.json");
    //
    // // add an example layer
    window->GetLayerStack().PushLayer(std::make_unique<GameLayer>(mothContext, graphics));

    // start the application
    application->Run();
}

int main(int argc, char* argv[]) {
    // auto platform = std::make_unique<canyon::platform::sdl::Platform>();
    auto platform = std::make_unique<canyon::platform::glfw::Platform>();
    platform->Startup();

    // startExampleApp(*platform);
    // exampleMain(*platform);
    start(*platform);

    platform->Shutdown();
    return 0;
}

