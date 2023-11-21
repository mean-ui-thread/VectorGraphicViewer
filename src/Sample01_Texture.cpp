#include "Sample01_Texture.h"

#include <random>

#include <imgui.h>

#include "ViewerApp.h"


template <typename T>
T randomFrom(const T min, const T max)
{
    static std::random_device rdev;
    static std::default_random_engine re(rdev());
    typedef typename std::conditional<
        std::is_floating_point<T>::value,
        std::uniform_real_distribution<T>,
        std::uniform_int_distribution<T>>::type dist_type;
    dist_type uni(min, max);
    return static_cast<T>(uni(re));
}

void Sample01_Texture::resetRenderState() {

    // Normally I would never do that. I would use an IBO or Triangle Strip,
    // but it is easier to get the vertex position to draw debug triangles that
    // way.
    vertices = {
        //{   X       Y      Z  }  { S     T  }
        { {-128.0f,-128.0f, 0.0f}, {0.0f, 0.0f} },
        { {-128.0f, 128.0f, 0.0f}, {0.0f, 1.0f} },
        { { 128.0f,-128.0f, 0.0f}, {1.0f, 0.0f} },

        { { 128.0f,-128.0f, 0.0f}, {1.0f, 0.0f} },
        { {-128.0f, 128.0f, 0.0f}, {0.0f, 1.0f} },
        { { 128.0f, 128.0f, 0.0f}, {1.0f, 1.0f} },
    };
    vbo->upload(vertices, VertexBuffer::Dynamic);

    filtering = Texture::NoFiltering;
    texture->setFiltering(filtering);
}

bool Sample01_Texture::setup()
{
#ifdef __EMSCRIPTEN__
    Shader texVert("assets/tex_100_es.vert");
#else
    Shader texVert("assets/tex_330_core.vert");
#endif

    if (texVert.compile() != 0)
    {
        return false;
    }

#ifdef __EMSCRIPTEN__
    Shader texFrag("assets/tex_100_es.frag");
#else
    Shader texFrag("assets/tex_330_core.frag");
#endif
    if (texFrag.compile() != 0)
    {
        return false;
    }

    program = std::make_shared<ShaderProgram>(std::vector<AttributeInfo>({
        {"a_position", AttributeInfo::Float, 3},
        {"a_texCoord0", AttributeInfo::Float, 2},
    }));
    program->attach(&texVert);
    program->attach(&texFrag);
    if (program->link() != 0)
    {
        return false;
    }

    u_MVP      = program->getUniformLocation("u_MVP");
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

    vbo = std::make_shared<VertexBuffer>();

    resetRenderState();

    return true;
}

void Sample01_Texture::teardown() {
    program.reset();
    texture.reset();
    vbo.reset();
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
    texture->bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei)vertices.size());
    texture->unbind();
    vbo->unbind();
    program->unbind();
}



void Sample01_Texture::renderUI() {

    const char* items[] = { "No Filtering", "Bi-linear Filtering", "Tri-linear Filtering"  };
    if (ImGui::Combo("Texture Filtering", (int*)&filtering, items, IM_ARRAYSIZE(items))) {
        texture->setFiltering(filtering);
    }


}

void Sample01_Texture::animateVertices(double t) {

    vertices[0].position.x = -128.0f + cos(t * 0.1f) * 40.0f;
    vertices[0].position.y = -128.0f + sin(t * 0.1f)  * 45.0f; 

    vertices[1].position.x = -128.0f + sin(t * 2.0f) * 20.0f;
    vertices[1].position.y = 128.0f + sin(t * 2.0f)* 15.0f;

    vertices[2].position.x = 128.0f + cos(t) * 30.0f;
    vertices[2].position.y = -128.0f + cos(t) * 33.3f;

    vertices[5].position.x = 128.0f + sin(t * 0.05f) * 80.0f;
    vertices[5].position.y = 128.0f + cos(t * 0.05f) * 12.0f;

    vertices[3].position = vertices[2].position;
    vertices[4].position = vertices[1].position;

    vbo->upload(vertices, VertexBuffer::Dynamic);

}

std::vector<glm::vec3> Sample01_Texture::getVertices() const
{
    std::vector<glm::vec3> result;
    result.reserve(vertices.size());
    for (size_t i = 0; i < vertices.size(); ++i) {
        result.push_back(vertices[i].position);
    }
    return result;
}