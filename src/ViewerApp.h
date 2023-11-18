#ifndef VIEWER_APP_H
#define VIEWER_APP_H

#include <cstdint>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "SampleData.h"

struct Example;

struct ViewerApp : public std::enable_shared_from_this<ViewerApp>
{
    int displayWidth, displayHeight;
    glm::vec4 clearColor;
    bool isRunning;

    SDL_Window* window = NULL;
    SDL_GLContext context = NULL;
    GLuint defaultVAO = 0;

    std::vector<std::shared_ptr<Example>> examples;
    size_t exampleIndex = 0;

    SampleData frameStats;

    ViewerApp(std::vector<std::shared_ptr<Example>> examples);
    ~ViewerApp();

    int32_t setup(const char * title, int32_t width, int32_t height);

    void step();

    void teardown();
};

#endif // VIEWER_APP_H
