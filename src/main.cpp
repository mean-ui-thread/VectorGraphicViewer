
#include <memory>

#if __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "ViewerApp.h"

#include "Example01_Texture.h"

auto app = std::make_shared<ViewerApp>(std::vector<std::shared_ptr<Example>>({
    std::make_shared<Example01_Texture>()
}));

void step() {
    app->step();
}

int main(int argc, char *argv[])
{
    if (app->setup("VectorGraphicViewer", 1280, 960) == 0)
    {
#ifdef __EMSCRIPTEN__
        emscripten_set_main_loop(step, 0, 1);
#else
        while(app->isRunning)
        {
            step();
        }
#endif
        app->teardown();
    }

    app.reset();

    return 0;
}
