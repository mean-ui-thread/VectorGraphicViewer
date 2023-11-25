#include "Sample03_VG_Stencil.h"

#include <chrono>

#ifdef __EMSCRIPTEN__
#define NANOVG_GLES2_IMPLEMENTATION
#else
#define NANOVG_GL3_IMPLEMENTATION
#endif
#include <nanovg_gl.h>
#include <nanovg_gl_utils.h>

#define NANOSVG_IMPLEMENTATION
#include <nanosvg.h>

#include <ViewerApp.h>

void Sample03_VG_Stencil::resetRenderState() {

}

bool Sample03_VG_Stencil::setup() {

    NSVGimage* androidImage = nsvgParseFromFile("assets/android.svg", "px", 96);
    NSVGimage* tigerImage = nsvgParseFromFile("assets/Ghostscript_Tiger.svg", "px", 96);
    
    vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);

    return true;
}

void Sample03_VG_Stencil::teardown() {
    nvgReset(vg);
    nvgDeleteGL3(vg);
    nsvgDelete(androidImage);
    nsvgDelete(tigerImage);
}

void Sample03_VG_Stencil::render(const std::shared_ptr<ViewerApp> &app, const glm::mat4 &mvp) {

	nvgBeginFrame(vg, app->displayWidth(), app->displayHeight(), app->pxRatio());
    nvgTranslate(vg, app->position.x, app->position.y);
    nvgRotate(vg, app->rotation.z);
    nvgScale(vg, app->scale.x, app->scale.y);
    draw(app, mvp);
    nvgResetTransform(vg);
    nvgEndFrame(vg);
}

void Sample03_VG_Stencil::renderUI() {

    char progressBarText[32];

    sprintf(progressBarText, "%.1f", stencilTimeMs);
    ImGui::ProgressBar(stencilTimeMs / 10.0f, ImVec2(0, 0), progressBarText);
    ImGui::SameLine();
    ImGui::Text("Stencil Time (ms)");

    const char* items[] = { 
        "Heart", 
        "Smiley", 
        "Pacman Game",
        "assets/android.svg",
        "assets/Ghostscript_Tiger.svg"
    };
    ImGui::Combo("combo", &drawMode, items, IM_ARRAYSIZE(items));

}

// for debug purpose. Doesn't really need to be optimized.
std::vector<glm::vec3> Sample03_VG_Stencil::getVertices() const {
    std::vector<glm::vec3> result;
    // TODO
    return result;
}

// for debug purpose. Doesn't really need to be optimized.
std::vector<Triangle> Sample03_VG_Stencil::getTriangles() const {
    std::vector<Triangle> result;
    // TODO
    return result;
}


void Sample03_VG_Stencil::roundedRect(float x, float y, float width, float height, float radius) {
  nvgBeginPath(vg);
  nvgMoveTo(vg, x, y + radius);
  nvgLineTo(vg, x, y + height - radius);
  nvgArcTo(vg, x, y + height, x + radius, y + height, radius);
  nvgLineTo(vg, x + width - radius, y + height);
  nvgArcTo(vg, x + width, y + height, x + width, y + height-radius, radius);
  nvgLineTo(vg, x + width, y + radius);
  nvgArcTo(vg, x + width, y, x + width - radius, y, radius);
  nvgLineTo(vg, x + radius, y);
  nvgArcTo(vg, x, y, x, y + radius, radius);
  nvgFillColor(vg, nvgRGBA(0,0,0,0));
  nvgStroke(vg);
}

void Sample03_VG_Stencil::draw(const std::shared_ptr<ViewerApp> &app, const glm::mat4 &mvp) {
    auto startTime = std::chrono::high_resolution_clock::now();
    switch(drawMode) {
        default:
        case Heart: drawHeart(app, mvp); break;
        case Smiley: drawSmiley(app, mvp); break;
        case PacmanGame: drawPacmanGame(app, mvp); break;
        case AndroidSVG: drawAndroidSVG(app, mvp); break;
        case TigerSVG: drawTigerSVG(app, mvp); break;
    }
    stencilTimeMs = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - startTime).count() / 1000000.0;
}

void Sample03_VG_Stencil::drawHeart(const std::shared_ptr<ViewerApp> &app, const glm::mat4 &mvp) {

    nvgBeginPath(vg);
    nvgMoveTo(vg, 75, 40);
    nvgBezierTo(vg, 75, 37, 70, 25, 50, 25);
    nvgBezierTo(vg, 20, 25, 20, 62.5, 20, 62.5);
    nvgBezierTo(vg, 20, 80, 40, 102, 75, 120);
    nvgBezierTo(vg, 110, 102, 130, 80, 130, 62.5);
    nvgBezierTo(vg, 130, 62.5, 130, 25, 100, 25);
    nvgBezierTo(vg, 85, 25, 75, 37, 75, 40);
    nvgFillColor(vg, nvgRGB(Crimson.r, Crimson.g, Crimson.b));
    nvgFill(vg);

}

void Sample03_VG_Stencil::drawSmiley(const std::shared_ptr<ViewerApp> &app, const glm::mat4 &mvp) {

    nvgBeginPath(vg);
    nvgArc(vg, 75, 75, 50, 0, M_PI * 2.0f, NVG_CCW); // Outer circle
    nvgMoveTo(vg, 110, 75);
    nvgArc(vg, 75, 75, 35, 0, M_PI, NVG_CW); // Mouth (clockwise)
    nvgMoveTo(vg, 65, 65);
    nvgArc(vg, 60.0f, 65.0f, 5.0f, 0.0f, M_PI * 2.0f, NVG_CCW); // Left eye
    nvgMoveTo(vg, 95.0f, 65.0f);
    nvgArc(vg, 90.0f, 65.0f, 5.0f, 0.0f, M_PI * 2.0f, NVG_CCW); // Right eye
    nvgFillColor(vg, nvgRGBA(0,0,0,0));
    nvgStrokeColor(vg, nvgRGB(DarkMagenta.r,DarkMagenta.g,DarkMagenta.b));
    nvgStroke(vg);
}

void Sample03_VG_Stencil::drawPacmanGame(const std::shared_ptr<ViewerApp> &app, const glm::mat4 &mvp) {


    roundedRect(12, 12, 150, 150, 15);
    roundedRect(19, 19, 150, 150, 9);
    roundedRect(53, 53, 49, 33, 10);
    roundedRect(53, 119, 49, 16, 6);
    roundedRect(135, 53, 49, 33, 10);
    roundedRect(135, 119, 25, 49, 10);

    nvgBeginPath(vg);
    nvgFillColor(vg, nvgRGB(Yellow.r, Yellow.g, Yellow.b));
    nvgBeginPath(vg);
    nvgArc(vg, 37, 37, 13, M_PI / 7.0f, M_PI / -7.0f, NVG_CW);
    nvgLineTo(vg, 31, 37);
    nvgFill(vg);

    for (int i = 0; i < 8; i++) {
        nvgBeginPath(vg);
        nvgFillColor(vg, nvgRGB(Gold.r, Gold.g, Gold.b));
        nvgRect(vg, 51 + i * 16, 35, 4, 4);
        nvgFill(vg);
    }

    for (int i = 0; i < 6; i++) {
        nvgBeginPath(vg);
        nvgFillColor(vg, nvgRGB(Gold.r, Gold.g, Gold.b));
        nvgRect(vg, 115, 51 + i * 16, 4, 4);
        nvgFill(vg);
    }

    for (int i = 0; i < 8; i++) {
        nvgBeginPath(vg);
        nvgFillColor(vg, nvgRGB(Gold.r, Gold.g, Gold.b));
        nvgRect(vg, 51 + i * 16, 99, 4, 4);
        nvgFill(vg);
    }

    nvgBeginPath(vg);
    nvgFillColor(vg, nvgRGB(FireBrick.r, FireBrick.g, FireBrick.b));
    nvgBeginPath(vg);
    nvgMoveTo(vg, 83, 116);
    nvgLineTo(vg, 83, 102);
    nvgBezierTo(vg, 83, 94, 89, 88, 97, 88);
    nvgBezierTo(vg, 105, 88, 111, 94, 111, 102);
    nvgLineTo(vg, 111, 116);
    nvgLineTo(vg, 106.333, 111.333);
    nvgLineTo(vg, 101.666, 116);
    nvgLineTo(vg, 97, 111.333);
    nvgLineTo(vg, 92.333, 116);
    nvgLineTo(vg, 87.666, 111.333);
    nvgLineTo(vg, 83, 116);
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgFillColor(vg, nvgRGB(White.r, White.g, White.b));
    nvgBeginPath(vg);
    nvgMoveTo(vg, 91, 96);
    nvgBezierTo(vg, 88, 96, 87, 99, 87, 101);
    nvgBezierTo(vg, 87, 103, 88, 106, 91, 106);
    nvgBezierTo(vg, 94, 106, 95, 103, 95, 101);
    nvgBezierTo(vg, 95, 99, 94, 96, 91, 96);
    nvgMoveTo(vg, 103, 96);
    nvgBezierTo(vg, 100, 96, 99, 99, 99, 101);
    nvgBezierTo(vg, 99, 103, 100, 106, 103, 106);
    nvgBezierTo(vg, 106, 106, 107, 103, 107, 101);
    nvgBezierTo(vg, 107, 99, 106, 96, 103, 96);
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgFillColor(vg, nvgRGB(Black.r, Black.g, Black.b));
    nvgArc(vg, 101, 102, 2, 0, M_PI * 2.0f, NVG_CCW);
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgArc(vg, 89, 102, 2, 0, M_PI * 2.0f, NVG_CCW);
    nvgFill(vg);

    resetRenderState();
}

void Sample03_VG_Stencil::drawAndroidSVG(const std::shared_ptr<ViewerApp> &app, const glm::mat4 &mvp) {
    for (NSVGshape *shape = androidImage->shapes; shape != nullptr; shape = shape->next) {
        nvgStrokeWidth(vg, shape->strokeWidth);
        for (NSVGpath *path = shape->paths; path != nullptr; path = path->next) {
            nvgBeginPath(vg);
            nvgMoveTo(vg, path->pts[0], path->pts[1]);
            for (int i = 0; i < path->npts-1; i += 3) {
                float* p = &path->pts[i*2];
                nvgBeginPath(vg);
                nvgBezierTo(vg, p[2], p[3], p[4], p[5], p[6], p[7]);
                nvgStroke(vg);
            }
        }
    }
}

void Sample03_VG_Stencil::drawTigerSVG(const std::shared_ptr<ViewerApp> &app, const glm::mat4 &mvp) {
    for (NSVGshape *shape = tigerImage->shapes; shape != nullptr; shape = shape->next) {
        nvgStrokeWidth(vg, shape->strokeWidth);
        for (NSVGpath *path = shape->paths; path != nullptr; path = path->next) {
            nvgBeginPath(vg);
            nvgMoveTo(vg, path->pts[0], path->pts[1]);
            for (int i = 0; i < path->npts-1; i += 3) {
                float* p = &path->pts[i*2];
                nvgBeginPath(vg);
                nvgBezierTo(vg, p[2], p[3], p[4], p[5], p[6], p[7]);
            }
            if (path->closed)
                nvgLineTo(vg, path->pts[0], path->pts[1]);
            nvgStroke(vg);
        }
    }
}