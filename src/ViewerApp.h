#ifndef VIEWER_APP_H
#define VIEWER_APP_H

#include <chrono>
#include <cstdint>
#include <memory>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "CPUUsage.h"
#include "MemoryUsage.h"
#include "SampleData.h"
#include "ShaderProgram.h"
#include "VertexBuffer.h"

class AbstractSample;

class ViewerApp : public std::enable_shared_from_this<ViewerApp>
{
public:
    ViewerApp(std::vector<std::shared_ptr<AbstractSample> > samples);
    ~ViewerApp();

    bool setup(const char * title, int32_t width, int32_t height);

    void step();

    void renderUI(double frameTimeSecs);

    void teardown();

    void resetRenderState();

    void renderPerformanceMonitorUI(double frameTimeSecs);

    inline const bool isRunning() const {
        return m_isRunning;
    }

    inline int32_t displayWidth() const
    {
        return m_displayWidth;
    }

    inline int32_t displayHeight() const
    {
        return m_displayHeight;
    }

    inline double getTimeSecs() const {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - launchTime).count() / 1000000000.0;
    }

private:

    std::shared_ptr<ShaderProgram> debugProgram;
    std::shared_ptr<VertexBuffer> debugVbo;

    GLint u_MVP = -1;
    GLint u_color = -1;



    int32_t m_displayWidth = 0;
    int32_t m_displayHeight = 0;
    const glm::vec4 m_clearColor = glm::vec4(0.45f, 0.55f, 0.60f, 1.00f);
    const glm::vec4 m_vertexColor = glm::vec4(1.0f, 0.0f, 0.5f, 1.0f);
    const glm::vec4 m_lineColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    bool m_isRunning = false;

    SDL_Window* m_window = NULL;
    SDL_GLContext m_context = NULL;
    GLuint m_defaultVAO = 0;

    std::vector< std::shared_ptr<AbstractSample> > m_samples;
    size_t m_sampleCurrent = 0;
    size_t m_sampleSelected = 0;

    CPUUsage m_cpuUsage;
    MemoryUsage m_memUsage;

    const std::chrono::time_point<std::chrono::high_resolution_clock> launchTime = std::chrono::high_resolution_clock::now();

    SampleData m_frameStats;
    SampleData m_cpuStats;
    SampleData m_memStats;


    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    bool renderSample;
    bool renderTriangles;
    bool renderVertices;

    bool animateTransforms;
    bool animateVertices;



};

#endif // VIEWER_APP_H
