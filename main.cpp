#include <MorglodsRender.hpp>
#include <mu/Thread.hpp>
#include <Types.hpp>
#include <Utils/FilesIO.hpp>
#include <Materials/Material.hpp>

#include <SimpleApp/SimpleGLFWApp.hpp>

/*
//Do not use hybryd graphics
extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}
*/

int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;
int SCREEN_CENTER_X = 400;
int SCREEN_CENTER_Y = 300;

#include "TestTriangle.hpp"
#include "Tests.hpp"
#include "texture_streamer_test.hpp"

int main() {
    mr::GLFWWindowHints hints;

    std::cout << "Loading config from \"settings.cfg\"... ";
    mu::Config config;
    if(config.ParseFile("settings.cfg")) {
        config.GetTo("window.width", WINDOW_WIDTH);
        config.GetTo("window.height", WINDOW_HEIGHT);
        hints.ConfigureFrom(&config);

        SCREEN_CENTER_X = WINDOW_WIDTH / 2;
        SCREEN_CENTER_Y = WINDOW_HEIGHT / 2;

        std::cout << "ok" << std::endl;
    } else {
        std::cout << "failed." << std::endl << "Creating default config... ";

        hints.SaveToConfig(&config);
        config.Set("window.width", std::to_string(WINDOW_WIDTH));
        config.Set("window.height", std::to_string(WINDOW_HEIGHT));
        config.Set("config.use", "1");

        if(config.SaveTo("settings.cfg")) std::cout << "ok" << std::endl;
        else std::cout << "failed saving." << std::endl;
    }

    texture_streamer_test_main(glm::vec2((float)WINDOW_WIDTH, (float)WINDOW_HEIGHT));

    //TestTriangle app;
    //app.Go("MorglodsRender test", WINDOW_WIDTH, WINDOW_HEIGHT, hints);

    return 0;
}

