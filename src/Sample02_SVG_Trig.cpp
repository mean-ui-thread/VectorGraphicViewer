#include "Sample02_SVG_Trig.h"



void Sample02_SVG_Trig::resetRenderState() {
    
    vbo->upload(meshes[0].vertices, VertexBuffer<PositionVertex>::Static);
    ibo->upload(meshes[0].indices, IndexBuffer::Static);

}

bool Sample02_SVG_Trig::setup() {

#ifdef __EMSCRIPTEN__
    Shader texVert("assets/color_100_es.vert");
    Shader texFrag("assets/color_100_es.frag");
#else
    Shader texVert("assets/color_330_core.vert");
    Shader texFrag("assets/color_330_core.frag");
#endif

    program = std::make_shared<ShaderProgram>("Android Vector Graphic Program", std::vector<AttributeInfo>({
        {"a_position", AttributeInfo::Float, 3}
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

    vbo = std::make_shared<VertexBuffer<PositionVertex>>("Android Vector Graphic VBO");
    ibo = std::make_shared<IndexBuffer>("Android Vector Graphic IBO");


    Path2D ctx;

    // heart
    // ctx.moveTo(75, 40);
    // ctx.bezierCurveTo(75, 37, 70, 25, 50, 25);
    // ctx.bezierCurveTo(20, 25, 20, 62.5, 20, 62.5);
    // ctx.bezierCurveTo(20, 80, 40, 102, 75, 120);
    // ctx.bezierCurveTo(110, 102, 130, 80, 130, 62.5);
    // ctx.bezierCurveTo(130, 62.5, 130, 25, 100, 25);
    // ctx.bezierCurveTo(85, 25, 75, 37, 75, 40);
    //meshes = ctx.fill();

    //smiley
    ctx.arc(75, 75, 50, 0, M_PI * 2.0f, true); // Outer circle
    ctx.moveTo(110, 75);
    ctx.arc(75, 75, 35, 0, M_PI, false); // Mouth (clockwise)
    ctx.moveTo(65, 65);
    ctx.arc(60.0f, 65.0f, 5.0f, 0.0f, M_PI * 2.0f, true); // Left eye
    ctx.moveTo(95.0f, 65.0f);
    ctx.arc(90.0f, 65.0f, 5.0f, 0.0f, M_PI * 2.0f, true); // Right eye
    meshes = ctx.stroke();

    resetRenderState();


    return true;
}

void Sample02_SVG_Trig::teardown() {
    program.reset();
    vbo.reset();
    ibo.reset();
}

void Sample02_SVG_Trig::render(const glm::mat4 &mvp) {
    program->bind();
    program->setMVP(mvp);
    program->setColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    vbo->bind(program);
    ibo->bind();
    glDrawElements(GL_TRIANGLES, ibo->indices.size(), GL_UNSIGNED_SHORT, nullptr);
    ibo->unbind();
    vbo->unbind();
    program->unbind();

}

void Sample02_SVG_Trig::renderUI() {

}

// for debug purpose. Doesn't really need to be optimized.
std::vector<glm::vec3> Sample02_SVG_Trig::getVertices() const {
    std::vector<glm::vec3> result;
    result.reserve(vbo->vertices.size());
    for (size_t i = 0; i < vbo->vertices.size(); ++i) {
        result.push_back(vbo->vertices[i].position);
    }
    return result;
}

// for debug purpose. Doesn't really need to be optimized.
std::vector<Triangle> Sample02_SVG_Trig::getTriangles() const {
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