
#include "ViewerApp.h"

#include <cassert>
#include <chrono>
#include <iostream>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>

#include "AbstractGPUObject.h"
#include "AbstractSample.h"
#include "ShaderProgram.h"

static constexpr size_t SAMPLE_COUNT = 300;

ViewerApp::ViewerApp(std::vector<std::shared_ptr<AbstractSample> > samples): 
    m_samples(samples),
    m_frameStats(SAMPLE_COUNT),
    m_cpuStats(SAMPLE_COUNT),
    m_memStats(SAMPLE_COUNT),
    m_gpuMemStats(SAMPLE_COUNT),
    m_trigStats(SAMPLE_COUNT)
{
    assert(m_samples.size() > 0);
}

ViewerApp::~ViewerApp()
{
    if (m_context)
    {
        SDL_GL_DeleteContext(m_context);
        m_context = NULL;
    }

    if (m_window)
    {
        SDL_DestroyWindow(m_window);
        m_window = NULL;
    }

    IMG_Quit();
    SDL_Quit();
}

bool ViewerApp::setup(const char * title, int32_t width, int32_t height)
{
    //Initialize SDL2
    if(SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_LogCritical(0, "SDL could not initialize: %s", SDL_GetError());
        return false;
    }

    //Initialize SDL Image
    if (!IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG))
    {
        SDL_LogCritical(0, "SDL Image could not initialize: %s", IMG_GetError());
        return false;
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
    m_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if(!m_window)
    {
        SDL_LogCritical(0, "Window creation error: %s", SDL_GetError());
        return false;
    }

    // Create GL Context
    m_context = SDL_GL_CreateContext(m_window);
    if (!m_context)
    {
        SDL_LogCritical(0, "GL context creation error: %s", SDL_GetError());
        SDL_DestroyWindow(m_window);
        m_window = NULL;
        return false;
    }

    //Initialize GLAD
#ifdef __EMSCRIPTEN__
    if(!gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress))
#else
    if(!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
#endif
    {
        SDL_LogCritical(0,"Error initializing GLAD!");

        SDL_GL_DeleteContext(m_context);
        m_context = NULL;

        SDL_DestroyWindow(m_window);
        m_window = NULL;

        return false;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGuiContext * imguiCtx = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(m_window, m_context);
    ImGui_ImplOpenGL3_Init(NULL);

    ImPlot::CreateContext();

#ifndef __EMSCRIPTEN__
    //Default Vertex Array Object
    glGenVertexArrays(1, &m_defaultVAO);
    glBindVertexArray(m_defaultVAO);
#endif


    // populate displayWidth and displayHeight before the user's init()
    // so the user can use these variables if needed.
    SDL_GL_GetDrawableSize(m_window, &m_displayWidth, &m_displayHeight);

    // User Init
    if (!m_samples[m_sampleCurrent]->setup())
    {
        SDL_GL_DeleteContext(m_context);
        m_context = NULL;

        SDL_DestroyWindow(m_window);
        m_window = NULL;

        return false;
    }

#ifdef __EMSCRIPTEN__
    Shader texVert("assets/color_100_es.vert");
    Shader texFrag("assets/color_100_es.frag");
#else
    Shader texVert("assets/color_330_core.vert");
    Shader texFrag("assets/color_330_core.frag");
#endif

    m_debugProgram = std::make_shared<ShaderProgram>("#debugProgram", std::vector<AttributeInfo>({
        {"a_position", AttributeInfo::Float, 3}
    }));

    if (!m_debugProgram->attach(texVert)) {
        return false;
    }

    if (!m_debugProgram->attach(texFrag)) {
        return false;
    }

    if (!m_debugProgram->link())
    {
        return false;
    }

    m_u_color = m_debugProgram->getUniformLocation("u_color");
    if (m_u_color < 0) {
        return false;
    }

    m_u_MVP = m_debugProgram->getUniformLocation("u_MVP");
    if (m_u_MVP < 0) {
        return false;
    }

    m_debugVbo = std::make_shared<VertexBuffer<glm::vec3>>("#debugVBO");

    glPointSize(8);
    glLineWidth(4);

    glEnable(GL_BLEND);

    // don't multiply alpha because we are pre-multiplying our textures in Texture.cpp
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    resetRenderState();

    m_isRunning = true;

    return m_isRunning;
}

void ViewerApp::teardown()
{
    m_samples.clear();

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &m_defaultVAO);
    m_defaultVAO = 0;

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void ViewerApp::step()
{
    double frameTimeSecs = getTimeSecs();

    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        // User requests quit
        if (e.type == SDL_QUIT) {
            m_isRunning = false;
        }

        ImGui_ImplSDL2_ProcessEvent(&e);
    }

    SDL_GL_GetDrawableSize(m_window, &m_displayWidth, &m_displayHeight);
    glViewport(0, 0, m_displayWidth, m_displayHeight);
    glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    float fieldOfViewRad = glm::radians(45.0f);
    float cameraDistance = ((float)m_displayHeight/2.0f) / tan(fieldOfViewRad/2.0f);
    float aspectRatio = (float)m_displayWidth / (float)m_displayHeight;

    glm::mat4 projectionMatrix = glm::perspective(fieldOfViewRad, aspectRatio, 0.0001f, cameraDistance*2.0f);
    projectionMatrix = glm::rotate(projectionMatrix, glm::radians(180.0f), glm::vec3(1, 0, 0));
    projectionMatrix = glm::translate(projectionMatrix, glm::vec3(-m_displayWidth*0.5f, -m_displayHeight*0.5f, cameraDistance));

    if (animateTransforms) {
        position.x = m_displayWidth / 2.0f + cos(frameTimeSecs) * m_displayWidth / 16.0f;
        position.y = m_displayHeight / 2.0f + sin(frameTimeSecs) * m_displayHeight / 16.0f;
        position.z = sin(frameTimeSecs * 0.25f) * m_displayHeight;
        rotation.z = sin(frameTimeSecs * 1.333f) * 60.0f;
    }

    glm::mat4 modelMatrix  = glm::translate(glm::identity<glm::mat4>(), position);
    modelMatrix *= glm::yawPitchRoll(glm::radians(rotation.y), glm::radians(rotation.x), glm::radians(rotation.z));
    modelMatrix  = glm::scale(modelMatrix, scale);

    const glm::mat4 mvp = projectionMatrix * modelMatrix;

    // Enable/Disable v-sync if requested
    if (m_verticalSyncCurrent != m_verticalSyncRequested) {
        if(m_verticalSyncRequested) {
            // Try adaptive v-sync
            if (SDL_GL_SetSwapInterval(-1) != 0)
            {
                SDL_LogWarn(0, "Enabling Adaptive V-Sync failed: %s", SDL_GetError());
                // fallback to v-sync
                if (SDL_GL_SetSwapInterval(1) != 0){
                    SDL_LogWarn(0, "Enabling V-sync failed: %s", SDL_GetError());
                }
            }
        } else {
            if (SDL_GL_SetSwapInterval(0) != 0)
            {
                SDL_LogWarn(0, "Disabling V-sync failed: %s", SDL_GetError());
            }
        }
        m_verticalSyncCurrent = m_verticalSyncRequested;
    }

    // Switch sample if requested
    if (m_sampleCurrent != m_sampleRequested) {
        m_samples[m_sampleCurrent]->teardown();
        m_sampleCurrent = m_sampleRequested;
        m_samples[m_sampleCurrent]->setup();
        resetRenderState();
    }

    if (renderSample) {
        m_samples[m_sampleCurrent]->render(mvp);
    }

    if (renderTriangles) {
        std::vector<Triangle> debugTriangles = m_samples[m_sampleCurrent]->getTriangles();
        m_debugProgram->bind();
        m_debugProgram->setUniform(m_u_MVP, mvp);
        m_debugProgram->setUniform(m_u_color, m_vertexColor);
        m_debugVbo->bind(m_debugProgram);
        m_debugProgram->setUniform(m_u_color, m_lineColor);
        for (size_t i = 0; i < debugTriangles.size(); ++i) {
            m_debugVbo->upload(debugTriangles[i].points, VertexBuffer<glm::vec3>::Stream);
            glDrawArrays(GL_LINE_LOOP, 0, debugTriangles[i].points.size());
        }
        m_debugVbo->unbind();
        m_debugProgram->unbind();
    }

    if (renderVertices) {
        std::vector<glm::vec3> debugVertices = m_samples[m_sampleCurrent]->getVertices();
        if (debugVertices.size() > 0) {
            m_debugVbo->upload(debugVertices, VertexBuffer<glm::vec3>::Stream);
            m_debugProgram->bind();
            m_debugProgram->setUniform(m_u_MVP, mvp);
            m_debugProgram->setUniform(m_u_color, m_vertexColor);
            m_debugVbo->bind(m_debugProgram);
            m_debugProgram->setUniform(m_u_color, m_vertexColor);
            glDrawArrays(GL_POINTS, 0, (GLsizei)debugVertices.size());
            m_debugVbo->unbind();
            m_debugProgram->unbind();

        }
    }

    renderUI(frameTimeSecs);

    // simple protection to ensure that we only collect stats at around 60Hz in
    // case our refresh rate is higher or in case v-sync is turned off.
    // Otherwise we would need a larger cyclic sample buffer which will consume
    // more memory and would take much longer to calculate min/max/avg, etc
    static double statsTimeCounter = 1.0;
    if (statsTimeCounter > 0.016) {
        // Should be gotten before calling SDL_GL_SwapWindow since this function
        // puts the CPU to sleep when V-SYNC is turned on. Capturing the CPU usage
        // before that function call makes more sense.
        m_cpuStats.addPoint(frameTimeSecs, m_cpuUsage.getValuePercent());

        SDL_GL_SwapWindow(m_window);

        // calculate how long the frame took to render.
        m_lastFrameDurationSecs = getTimeSecs() - frameTimeSecs;

        m_frameStats.addPoint(frameTimeSecs, m_lastFrameDurationSecs * 1000.0f); // from seconds to milliseconds
        m_memStats.addPoint(frameTimeSecs, m_memUsage.getValueKB() / 1024.0f); // from KB to MB.

        size_t totalGPUMemUsage = 0;

        auto it = AbstractGPUObject::registry.cbegin();
        auto end = AbstractGPUObject::registry.cend();
        while (it != end) {
            totalGPUMemUsage += it->second->getMemoryUsage();
            it++;
        }
        m_gpuMemStats.addPoint(frameTimeSecs, totalGPUMemUsage / 1024.0f); // from B to KB.

        m_trigStats.addPoint(frameTimeSecs, m_samples[m_sampleCurrent]->getTriangles().size()); // from B to KB.

        statsTimeCounter = 0.0;
    } else {

        SDL_GL_SwapWindow(m_window);

        // calculate how long the frame took to render.
        m_lastFrameDurationSecs = getTimeSecs() - frameTimeSecs;
        statsTimeCounter += m_lastFrameDurationSecs;
    }

}

void ViewerApp::renderUI(double frameTimeSecs)
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(m_window);
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(480.0f, m_displayHeight -40.0f), ImGuiCond_Once);

    if (ImGui::Begin("Control Panel"))
    {
        ImGui::Checkbox("V-Sync", &m_verticalSyncRequested);

        if (ImGui::BeginTabBar("##SampleSelection", ImGuiTabBarFlags_FittingPolicyScroll))
        {
            for (size_t i = 0; i < m_samples.size(); ++i)
            {
                if (ImGui::BeginTabItem(m_samples[i]->name().c_str()))
                {
                    m_sampleRequested = i;
                    ImGui::EndTabItem();
                }
            }
        }
        ImGui::EndTabBar();

        if (ImGui::CollapsingHeader("Performance Monitor", ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_DefaultOpen)) {
            renderPerformanceMonitorUI(frameTimeSecs);
        }

        if (ImGui::Button("Reset")) {
            resetRenderState();
        }

        ImGui::Checkbox("Render Sample", &renderSample);
        ImGui::SameLine();
        ImGui::Checkbox("Render Triangles", &renderTriangles);
        ImGui::SameLine();
        ImGui::Checkbox("Render Vertices", &renderVertices);

        ImGui::Checkbox("Animate Transforms", &animateTransforms);
        if (ImGui::CollapsingHeader("Transforms", ImGuiTreeNodeFlags_CollapsingHeader)) {
            ImGui::SliderFloat("Translate X", &position.x, -m_displayWidth, m_displayWidth);
            ImGui::SliderFloat("Translate Y", &position.y, -m_displayHeight, m_displayHeight);
            ImGui::SliderFloat("Translate Z", &position.z, -8000.0f, 8000.0f);
            ImGui::SliderFloat("Rotate X", &rotation.x, -720, 720);
            ImGui::SliderFloat("Rotate Y", &rotation.y, -720, 720);
            ImGui::SliderFloat("Rotate Z", &rotation.z, -720, 720);
            ImGui::SliderFloat("Scale X", &scale.x, -4, 4);
            ImGui::SliderFloat("Scale Y", &scale.y, -4, 4);
        }

        if (ImGui::CollapsingHeader("GPU Objects", ImGuiTreeNodeFlags_CollapsingHeader)) {
            auto it = AbstractGPUObject::registry.cbegin();
            auto end = AbstractGPUObject::registry.cend();
            while (it != end) {
                ImGui::SeparatorText(it->first.c_str());
                it->second->renderUI();   
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f),"Estimated memory usage: %s", it->second->getPrintableMemoryUsage().c_str());
                it++;
            }
        }

        m_samples[m_sampleCurrent]->renderUI();
    }

    ImGui::End();

    //bool show = true;
    //ImGui::ShowDemoWindow(&show);
    // ImPlot::ShowDemoWindow(&show);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ViewerApp::resetRenderState()
{
    position = glm::vec3(m_displayWidth / 2.0f, m_displayHeight / 2.0f, 0.0f);
    rotation = glm::vec3(0.0f);
    scale = glm::vec3(1.0f);
    renderSample = true;
    renderTriangles = false;
    renderVertices = false;

    animateTransforms = false;

    m_samples[m_sampleCurrent]->resetRenderState();
}

void ViewerApp::renderPerformanceMonitorUI(double frameTimeSecs)
{
    if (ImPlot::BeginPlot("##CPU Performance Scope", ImVec2(-1, 128)))
    {
        static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels;
        ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoSideSwitch);
        ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_AuxDefault | ImPlotAxisFlags_NoSideSwitch);
        ImPlot::SetupAxisLimits(ImAxis_X1, frameTimeSecs - 5.0, frameTimeSecs, ImPlotCond_Always);
        ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 100);
        ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
        ImPlot::PlotLine("CPU Usage (%)", &m_cpuStats.data[0].x, &m_cpuStats.data[0].y, m_cpuStats.data.size(), ImPlotLineFlags_None, m_cpuStats.offset, sizeof(glm::vec2));
        ImPlot::PlotLine("Mem Usage (MB)", &m_memStats.data[0].x, &m_memStats.data[0].y, m_memStats.data.size(), ImPlotLineFlags_None, m_memStats.offset, sizeof(glm::vec2));
        ImPlot::PlotLine("Frame time (ms)", &m_frameStats.data[0].x, &m_frameStats.data[0].y, m_frameStats.data.size(), ImPlotLineFlags_None, m_frameStats.offset, sizeof(glm::vec2));
        ImPlot::EndPlot();
    }

    char progressBarText[32];
    sprintf(progressBarText, "%d", (int)(std::floor(1000.0f / m_frameStats.avg + 0.5f)));
    ImGui::ProgressBar(1.0f / m_frameStats.avg, ImVec2(0, 0), progressBarText);
    ImGui::SameLine();
    ImGui::Text("FPS");

    if (ImPlot::BeginPlot("##GPU Performance Scope", ImVec2(-1, 128)))
    {
        static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels;
        ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoSideSwitch);
        ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_AuxDefault | ImPlotAxisFlags_NoSideSwitch);
        ImPlot::SetupAxisLimits(ImAxis_X1, frameTimeSecs - 5.0, frameTimeSecs, ImPlotCond_Always);
        ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 1000);
        ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
        ImPlot::PlotLine("GPU Mem Usage (KB)", &m_gpuMemStats.data[0].x, &m_gpuMemStats.data[0].y, m_gpuMemStats.data.size(), ImPlotLineFlags_None, m_gpuMemStats.offset, sizeof(glm::vec2));
        ImPlot::PlotLine("Triangle Count", &m_trigStats.data[0].x, &m_trigStats.data[0].y, m_trigStats.data.size(), ImPlotLineFlags_None, m_trigStats.offset, sizeof(glm::vec2));
        ImPlot::EndPlot();
    }
}
