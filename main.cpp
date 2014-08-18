#include <MorglodsRender.hpp>
#include <Utils/Threads.hpp>
#include <Types.hpp>
#include <Utils/FilesIO.hpp>
#include <Materials/Material.hpp>

#include <SimpleApp.hpp>

//Do not use hybryd graphics
extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;
int SCREEN_CENTER_X = 400;
int SCREEN_CENTER_Y = 300;

//#include "TestSponza.hpp"
//#include "TestOneBufferGeometry.hpp"
#include "TestTriangle.hpp"

int main() {
    //Render settings
    MR::WindowHints hints;

    //Config
    std::cout << "Loading config from \"settings.cfg\"... ";
    MR::Config config;
    if(config.ParseFile("settings.cfg")) {
        config.GetTo("window.width", WINDOW_WIDTH);
        config.GetTo("window.height", WINDOW_HEIGHT);
        hints.Configure(&config);

        SCREEN_CENTER_X = WINDOW_WIDTH / 2;
        SCREEN_CENTER_Y = WINDOW_HEIGHT / 2;

        std::cout << "ok" << std::endl;
    } else {
        std::cout << "failed." << std::endl << "Creating default config... ";

        hints.SaveConfig(&config);
        config.Set("window.width", std::to_string(WINDOW_WIDTH));
        config.Set("window.height", std::to_string(WINDOW_HEIGHT));

        if(config.SaveTo("settings.cfg")) std::cout << "ok" << std::endl;
        else std::cout << "failed saving." << std::endl;
    }

    //App
    //TestSponza app;
    TestTriangle app;
    app.Go("MorglodsRender test", WINDOW_WIDTH, WINDOW_HEIGHT, hints);

    return 0;
}
