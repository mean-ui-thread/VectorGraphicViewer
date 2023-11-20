#include "Sample01_Texture.h"

#include "imgui.h"

#include "ViewerApp.h"

bool Sample01_Texture::setup()
{
#ifdef __EMSCRIPTEN__
    Shader defaultVert("assets/default_100_es.vert");
#else
    Shader defaultVert("assets/default_330_core.vert");
#endif

    if (defaultVert.compile() != 0)
    {
        return false;
    }

#ifdef __EMSCRIPTEN__
    Shader defaultFrag("assets/default_100_es.frag");
#else
    Shader defaultFrag("assets/default_330_core.frag");
#endif
    if (defaultFrag.compile() != 0)
    {
        return false;
    }

    program = std::make_shared<ShaderProgram>(std::vector<AttributeInfo>({
        {"a_position", AttributeInfo::Float, 3},
        {"a_texCoord0", AttributeInfo::Float, 2},
    }));
    program->attach(&defaultVert);
    program->attach(&defaultFrag);
    if (program->link() != 0)
    {
        return false;
    }

    u_MVP      = program->getUniformLocation("u_MVP");
    u_texture0 = program->getUniformLocation("u_texture0");

    texture = std::make_shared<Texture>("assets/background.jpg");
    if (texture->decode() != 0)
    {
        return false;
    }

    vbo = std::make_shared<VertexBuffer>();
    vbo->upload(vertices, VertexBuffer::Static);

    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    return true;
}

void Sample01_Texture::teardown() {

}

void Sample01_Texture::render(const glm::mat4 &mvp)
{
    // Draw texture
    program->bind();
    program->setUniform(u_texture0, 0);
    program->setUniform(u_MVP, mvp);
    vbo->bind(program);
    texture->bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei)vertices.size());
    texture->unbind();
    vbo->unbind();
    program->unbind();
}

void Sample01_Texture::renderVertices(const glm::mat4 &mvp)
{
    // Draw texture
    program->bind();
    program->setUniform(u_texture0, 0);
    program->setUniform(u_MVP, mvp);
    vbo->bind(program);
    texture->bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei)vertices.size());
    texture->unbind();
    vbo->unbind();
    program->unbind();
}

void Sample01_Texture::renderTriangles(const glm::mat4 &mvp)
{
    // Draw texture
    program->bind();
    program->setUniform(u_texture0, 0);
    program->setUniform(u_MVP, mvp);
    vbo->bind(program);
    texture->bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei)vertices.size());
    texture->unbind();
    vbo->unbind();
    program->unbind();
}

void Sample01_Texture::renderUI() {

}