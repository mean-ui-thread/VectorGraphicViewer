
#include "ViewerApp.h"

#include <iostream>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include "Example.h"
#include "ShaderProgram.h"



ViewerApp::ViewerApp(std::vector<std::shared_ptr<Example>> examples)
: examples(examples) {
    assert(examples.size() > 0);
}

ViewerApp::~ViewerApp()
{
    if (context)
    {
        SDL_GL_DeleteContext(context);
        context = NULL;
    }

    if (window)
    {
        SDL_DestroyWindow(window);
        window = NULL;
    }

    IMG_Quit();
    SDL_Quit();
}

int ViewerApp::setup(const char * title, int32_t width, int32_t height)
{
    //Initialize SDL2
    if(SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_LogCritical(0, "SDL could not initialize: %s", SDL_GetError());
        return -1;
    }

    //Initialize SDL Image
    if (!IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG))
    {
        SDL_LogCritical(0, "SDL Image could not initialize: %s", IMG_GetError());
        return -1;
    }

#ifndef __EMSCRIPTEN__
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // MSAA (Multi-sample Anti-Aliasing x4).
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    //Use OpenGL 3.2 core (Minimum for Renderdoc)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif

#ifdef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
#endif

    // Create SDL Window
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if(!window)
    {
        SDL_LogCritical(0, "Window creation error: %s", SDL_GetError());
        return -1;
    }

    // Create GL Context
    context = SDL_GL_CreateContext(window);
    if (!context)
    {
        SDL_LogCritical(0, "GL context creation error: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        window = NULL;
        return -1;
    }

    //Initialize GLAD
#ifdef __EMSCRIPTEN__
    if(!gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress))
#else
    if(!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
#endif
    {
        SDL_LogCritical(0,"Error initializing GLAD!");

        SDL_GL_DeleteContext(context);
        context = NULL;

        SDL_DestroyWindow(window);
        window = NULL;

        return -1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init(NULL);

#ifndef __EMSCRIPTEN__
    //Default Vertex Array Object
    glGenVertexArrays(1, &defaultVAO);
    glBindVertexArray(defaultVAO);
#endif

#if 1
    // Try adaptive v-sync
    if (SDL_GL_SetSwapInterval(-1) != 0)
    {
        SDL_LogWarn(0, "Enabling Adaptive V-Sync failed: %s", SDL_GetError());
        // fallback to v-sync
        if (SDL_GL_SetSwapInterval(1) != 0){
            SDL_LogWarn(0, "Enabling V-sync failed: %s", SDL_GetError());
        }
    }
#else
    if (SDL_GL_SetSwapInterval(0) != 0)
    {
        SDL_LogWarn(0, "Disabling V-sync failed: %s", SDL_GetError());
    }
#endif

    // populate displayWidth and displayHeight before the user's init()
    // so the user can use these variables if needed.
    SDL_GL_GetDrawableSize(window, &displayWidth, &displayHeight);

    // User Init
    for(size_t i = 0; i < examples.size(); ++i) {
        if (!examples[i]->setup())
        {
            SDL_GL_DeleteContext(context);
            context = NULL;

            SDL_DestroyWindow(window);
            window = NULL;

            return -1;
        }
    }


    // Our state
    clearColor = glm::vec4(0.45f, 0.55f, 0.60f, 1.00f);
    isRunning = true;
    return 0;
}

void ViewerApp::step()
{
    auto frameStartTime = std::chrono::high_resolution_clock::now();

    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        // User requests quit
        if (e.type == SDL_QUIT) {
            isRunning = false;
        }

        ImGui_ImplSDL2_ProcessEvent(&e);
    }

    SDL_GL_GetDrawableSize(window, &displayWidth, &displayHeight);
    glViewport(0, 0, displayWidth, displayHeight);
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    examples[exampleIndex]->render(shared_from_this());

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(480.0f, 640.0f), ImGuiCond_Once);
    ImGui::Begin(examples[exampleIndex]->name.c_str());

    char frameTextOverlay[256];
    snprintf(frameTextOverlay, sizeof(frameTextOverlay), "Avg: %.1fms, %.1f FPS", frameStats.m_avg, 1000.0f/frameStats.m_avg);

    ImGui::PlotHistogram(
        "Frame", frameStats.m_values, SampleData::SAMPLE_COUNT,
        frameStats.m_offset, frameTextOverlay, 0.0f, 100.0f,
        ImVec2((float)SampleData::SAMPLE_COUNT, 45.0f));

    examples[exampleIndex]->renderUI(shared_from_this());

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(window);

    auto frameEndTime = std::chrono::high_resolution_clock::now();
    frameStats.pushSample(float(std::chrono::duration_cast<std::chrono::microseconds>(frameEndTime-frameStartTime).count() / 1000.0f) );
}

void ViewerApp::teardown()
{
    examples.clear();

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &defaultVAO);
    defaultVAO = 0;

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}
