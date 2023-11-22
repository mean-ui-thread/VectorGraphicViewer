#include "Sample01_Texture.h"

#include <imgui.h>


void Sample01_Texture::resetRenderState() {

    vbo->upload({
        //{   X       Y      Z  }  { S     T  }
        { {-128.0f,-128.0f, 0.0f}, {0.0f, 0.0f} }, // top left
        { {-128.0f, 128.0f, 0.0f}, {0.0f, 1.0f} }, // bottom left
        { { 128.0f,-128.0f, 0.0f}, {1.0f, 0.0f} }, // top right
        { { 128.0f, 128.0f, 0.0f}, {1.0f, 1.0f} }, // bottom right
    }, VertexBuffer<TextureVertex>::Static);

    ibo->upload({
        0, 1, 2, // First Triangle: top left, bottom left, top right

        1, 3, 2, // 2nd Triangle: bottom left, bottom right, top right.
    }, IndexBuffer::Static);

    texture->setFiltering(Texture::NoFiltering);
}

bool Sample01_Texture::setup()
{
#ifdef __EMSCRIPTEN__
    Shader texVert("assets/tex_100_es.vert");
    Shader texFrag("assets/tex_100_es.frag");
#else
    Shader texVert("assets/tex_330_core.vert");
    Shader texFrag("assets/tex_330_core.frag");
#endif

    program = std::make_shared<ShaderProgram>("Android Texture Program", std::vector<AttributeInfo>({
        {"a_position", AttributeInfo::Float, 3},
        {"a_texCoord0", AttributeInfo::Float, 2},
    }));

    if (!program->attach(texVert)) {
        return false;
    }

    if (!program->attach(texFrag)) {
        return false;
    }

    if (!program->link())
    {
        return false;
    }

    u_MVP = program->getUniformLocation("u_MVP");
    if (u_MVP < 0) {
        return false;
    }

    u_texture0 = program->getUniformLocation("u_texture0");
    if (u_texture0 < 0) {
        return false;
    }

    texture = std::make_shared<Texture>("assets/android.png");
    if (texture->decode() != 0)
    {
        return false;
    }

    vbo = std::make_shared<VertexBuffer<TextureVertex>>("Android VBO");
    ibo = std::make_shared<IndexBuffer>("Android IBO");

    resetRenderState();

    return true;
}

void Sample01_Texture::teardown() {
    program.reset();
    texture.reset();
    vbo.reset();
    ibo.reset();
    u_MVP = -1;
    u_texture0 = -1;
}

void Sample01_Texture::render(const glm::mat4 &mvp)
{
    // Draw texture
    program->bind();
    program->setUniform(u_texture0, 0);
    program->setUniform(u_MVP, mvp);
    vbo->bind(program);
    ibo->bind();
    texture->bind();
    glDrawElements(GL_TRIANGLES, ibo->indices.size(), GL_UNSIGNED_SHORT, nullptr);
    texture->unbind();
    ibo->unbind();
    vbo->unbind();
    program->unbind();
}

void Sample01_Texture::renderUI() {
}

// for debug purpose. Doesn't really need to be optimized.
std::vector<glm::vec3> Sample01_Texture::getVertices() const {
    std::vector<glm::vec3> result;
    result.reserve(vbo->vertices.size());
    for (size_t i = 0; i < vbo->vertices.size(); ++i) {
        result.push_back(vbo->vertices[i].position);
    }
    return result;
}

// for debug purpose. Doesn't really need to be optimized.
std::vector<Triangle> Sample01_Texture::getTriangles() const {
    std::vector<Triangle> result;
    result.reserve(ibo->indices.size() / 3);
    for (size_t i = 0; i < ibo->indices.size(); i += 3)
    {
        result.push_back({ 
            {
                vbo->vertices[ibo->indices[i+0]].position,
                vbo->vertices[ibo->indices[i+1]].position,
                vbo->vertices[ibo->indices[i+2]].position
            }
        });
    }
    return result;
}