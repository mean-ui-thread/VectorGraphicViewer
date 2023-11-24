
#include <memory>

#if __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "ViewerApp.h"

#include "Sample00_Welcome.h"
#include "Sample01_PNG.h"
#include "Sample02_SVG_Trig.h"

#define SAMPLE(NAME) std::make_shared<NAME>(#NAME)

auto app = std::make_shared<ViewerApp>(std::vector<std::shared_ptr<AbstractSample>>({
    // SAMPLE(Sample00_Welcome),
    // SAMPLE(Sample01_PNG),
    SAMPLE(Sample02_SVG_Trig)
}));

void step() {
    app->step();
}

int main(int argc, char *argv[])
{
    if (app->setup("VectorGraphicViewer", 1600, 900))
    {
#ifdef __EMSCRIPTEN__
        emscripten_set_main_loop(step, 0, 1);
#else
        while(app->isRunning())
        {
            step();
        }
#endif
        app->teardown();
    }

    app.reset();

    return 0;
}
