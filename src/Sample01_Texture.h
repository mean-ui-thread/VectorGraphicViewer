#ifndef EXAMPLE01_TEXTURE_H
#define EXAMPLE01_TEXTURE_H

#include "AbstractSample.h"
#include "VertexData.h"

#include "ShaderProgram.h"
#include "Texture.h"
#include "VertexBuffer.h"

class Sample01_Texture : public AbstractSample {

public:

    inline Sample01_Texture(const std::string &name) : AbstractSample(name) {}
    virtual bool setup() override;
    virtual void teardown() override;
    virtual void render(const glm::mat4 &mvp) override;
    virtual void renderVertices(const glm::mat4 &mvp) override;
    virtual void renderTriangles(const glm::mat4 &mvp) override;
    virtual void renderUI() override;

private:
    std::shared_ptr<ShaderProgram> program;
    std::shared_ptr<Texture> texture;
    std::shared_ptr<VertexBuffer> vbo;

    std::vector<DefaultTextureVertex> vertices = {
        //{   X       Y       Z  }  { S     T  }
        { {  0.0f,   0.0f,   0.0f}, {0.0f, 0.0f} },
        {   {0.0f, 600.0f,   0.0f}, {0.0f, 1.0f} },
        { {800.0f,   0.0f,   0.0f}, {1.0f, 0.0f} },
        { {800.0f, 600.0f,   0.0f}, {1.0f, 1.0f} },
    };

    GLint u_MVP = 0;
    GLint u_texture0 = 0;
};

#endif // EXAMPLE01_TEXTURE_H