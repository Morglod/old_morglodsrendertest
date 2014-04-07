#include <MorglodsRender.hpp>
#include <SimpleApp.hpp>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define SCREEN_CENTER_X (WINDOW_WIDTH  / 2)
#define SCREEN_CENTER_Y (WINDOW_HEIGHT / 2)

#include "TestLods.hpp"
#include "TestTriangle.hpp"

int main(){
    TestLods app;
    app.Go("MorglodsRender test", WINDOW_WIDTH, WINDOW_HEIGHT);

    return 0;
}
