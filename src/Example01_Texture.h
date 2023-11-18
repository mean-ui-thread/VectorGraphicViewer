#ifndef EXAMPLE01_TEXTURE_H
#define EXAMPLE01_TEXTURE_H

#include "Example.h"
#include "VertexData.h"

#include "ShaderProgram.h"
#include "Texture.h"
#include "VertexBuffer.h"

struct Example01_Texture : public Example {
    std::shared_ptr<ShaderProgram> defaultProgram;
    std::shared_ptr<Texture> backgroundTex;
    std::shared_ptr<VertexBuffer> backgroundVBO;

    glm::vec3 position = glm::vec3(640.0f, 480.0f, 0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    glm::vec3 anchorPoint = glm::vec3(400.0f, 300.0f, 0.0f);

    std::vector<DefaultTextureVertex> backgroundVertices = {
        //{   X       Y       Z  }  { S     T  }
        { {  0.0f,   0.0f,   0.0f}, {0.0f, 0.0f} },
        {   {0.0f, 600.0f,   0.0f}, {0.0f, 1.0f} },
        { {800.0f,   0.0f,   0.0f}, {1.0f, 0.0f} },
        { {800.0f, 600.0f,   0.0f}, {1.0f, 1.0f} },
    };

    std::vector<DefaultTextureVertex> mikeVertices = {
        //{   X       Y       Z  }  { S     T  }
        { {  0.0f,   0.0f,   0.0f}, {0.0f, 0.0f} },
        {   {0.0f, 512.0f,   0.0f}, {0.0f, 1.0f} },
        { {512.0f,   0.0f,   0.0f}, {1.0f, 0.0f} },
        { {512.0f, 512.0f,   0.0f}, {1.0f, 1.0f} },
    };

    glm::mat4 projectionMatrix;
    glm::mat4 modelMatrix;

    GLint u_MVP = 0;
    GLint u_texture0 = 0;

    inline Example01_Texture() : Example("Example01_Texture") {}
    virtual bool setup() override;
    virtual void render(const std::shared_ptr<ViewerApp> &app) override;
    virtual void renderUI(const std::shared_ptr<ViewerApp> &app) override;
};

#endif // EXAMPLE01_TEXTURE_H