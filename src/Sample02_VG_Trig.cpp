#include "Sample02_VG_Trig.h"

#include <chrono>

void Sample02_VG_Trig::resetRenderState() {
    
    vbo->upload(mesh.vertices, VertexBuffer<ColorVertex>::Static);
    ibo->upload(mesh.indices, IndexBuffer::Static);

}

bool Sample02_VG_Trig::setup() {

#ifdef __EMSCRIPTEN__
    Shader texVert("assets/vcolor_100_es.vert");
    Shader texFrag("assets/vcolor_100_es.frag");
#else
    Shader texVert("assets/vcolor_330_core.vert");
    Shader texFrag("assets/vcolor_330_core.frag");
#endif

    program = std::make_shared<ShaderProgram>("Android Vector Graphic Program", std::vector<AttributeInfo>({
        {"a_position", AttributeInfo::Float, 3},
        {"a_color", AttributeInfo::UnsignedByte, 4, AttributeInfo::Normalize }
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

    vbo = std::make_shared<VertexBuffer<ColorVertex>>("Android Vector Graphic VBO");
    ibo = std::make_shared<IndexBuffer>("Android Vector Graphic IBO");

    draw();

    return true;
}

void Sample02_VG_Trig::teardown() {
    program.reset();
    vbo.reset();
    ibo.reset();
}

void Sample02_VG_Trig::render(const std::shared_ptr<ViewerApp> &app, const glm::mat4 &mvp) {
    program->bind();
    program->setMVP(mvp);
    vbo->bind(program);
    ibo->bind();
    glDrawElements(GL_TRIANGLES, ibo->indices.size(), GL_UNSIGNED_SHORT, nullptr);
    ibo->unbind();
    vbo->unbind();
    program->unbind();

}

void Sample02_VG_Trig::renderUI() {

    char progressBarText[32];

    sprintf(progressBarText, "%.1f", triangulationTimeMs);
    ImGui::ProgressBar(triangulationTimeMs / 10.0f, ImVec2(0, 0), progressBarText);
    ImGui::SameLine();
    ImGui::Text("Triangulation Time (ms)");

    const char* items[] = { 
        "Heart", 
        "Smiley", 
        "Pacman Game",
        "assets/android.svg",
        //"assets/Ghostscript_Tiger.svg"
    };
    if(ImGui::Combo("combo", &drawMode, items, IM_ARRAYSIZE(items))) {
        draw();
    }

    if (ImGui::SliderFloat("Tesselation", &tesselationFactor, 0.0f, 500.0f)) {
        draw();
    }
}

// for debug purpose. Doesn't really need to be optimized.
std::vector<glm::vec3> Sample02_VG_Trig::getVertices() const {
    std::vector<glm::vec3> result;
    result.reserve(vbo->vertices.size());
    for (size_t i = 0; i < vbo->vertices.size(); ++i) {
        result.push_back(vbo->vertices[i].position);
    }
    return result;
}

// for debug purpose. Doesn't really need to be optimized.
std::vector<Triangle> Sample02_VG_Trig::getTriangles() const {
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


void Sample02_VG_Trig::roundedRect(Path2D &ctx, float x, float y, float width, float height, float radius) {
  ctx.beginPath();
  ctx.moveTo(x, y + radius);
  ctx.lineTo(x, y + height - radius);
  ctx.arcTo(x, y + height, x + radius, y + height, radius);
  ctx.lineTo(x + width - radius, y + height);
  ctx.arcTo(x + width, y + height, x + width, y + height-radius, radius);
  ctx.lineTo(x + width, y + radius);
  ctx.arcTo(x + width, y, x + width - radius, y, radius);
  ctx.lineTo(x + radius, y);
  ctx.arcTo(x, y, x, y + radius, radius);
  ctx.fillStyle = Transparent;
  ctx.stroke(mesh);
}

void Sample02_VG_Trig::draw() {
    auto startTime = std::chrono::high_resolution_clock::now();
    switch(drawMode) {
        default:
        case Heart: drawHeart(); break;
        case Smiley: drawSmiley(); break;
        case PacmanGame: drawPacmanGame(); break;
        case AndroidSVG: drawAndroidSVG(); break;
        //case TigerSVG: drawTigerSVG(); break;
    }
    triangulationTimeMs = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - startTime).count() / 1000000.0;
}

void Sample02_VG_Trig::drawHeart() {
    mesh.indices.clear();
    mesh.vertices.clear();

    Path2D ctx(tesselationFactor);
    ctx.beginPath();
    ctx.moveTo(75, 40);
    ctx.bezierCurveTo(75, 37, 70, 25, 50, 25);
    ctx.bezierCurveTo(20, 25, 20, 62.5, 20, 62.5);
    ctx.bezierCurveTo(20, 80, 40, 102, 75, 120);
    ctx.bezierCurveTo(110, 102, 130, 80, 130, 62.5);
    ctx.bezierCurveTo(130, 62.5, 130, 25, 100, 25);
    ctx.bezierCurveTo(85, 25, 75, 37, 75, 40);
    ctx.fillStyle = Crimson;
    ctx.fill(mesh);

    resetRenderState();
}

void Sample02_VG_Trig::drawSmiley() {

    mesh.indices.clear();
    mesh.vertices.clear();

    Path2D ctx(tesselationFactor);
    ctx.beginPath();
    ctx.arc(75, 75, 50, 0, M_PI * 2.0f, true); // Outer circle
    ctx.moveTo(110, 75);
    ctx.arc(75, 75, 35, 0, M_PI, false); // Mouth (clockwise)
    ctx.moveTo(65, 65);
    ctx.arc(60.0f, 65.0f, 5.0f, 0.0f, M_PI * 2.0f, true); // Left eye
    ctx.moveTo(95.0f, 65.0f);
    ctx.arc(90.0f, 65.0f, 5.0f, 0.0f, M_PI * 2.0f, true); // Right eye
    ctx.fillStyle = Transparent;
    ctx.strokeStyle = DarkMagenta;
    ctx.stroke(mesh);


    resetRenderState();
}

void Sample02_VG_Trig::drawPacmanGame() {

    mesh.indices.clear();
    mesh.vertices.clear();

    Path2D ctx(tesselationFactor);

    roundedRect(ctx, 12, 12, 150, 150, 15);
    roundedRect(ctx, 19, 19, 150, 150, 9);
    roundedRect(ctx, 53, 53, 49, 33, 10);
    roundedRect(ctx, 53, 119, 49, 16, 6);
    roundedRect(ctx, 135, 53, 49, 33, 10);
    roundedRect(ctx, 135, 119, 25, 49, 10);

    ctx.fillStyle = Yellow;
    ctx.beginPath();
    ctx.arc(37, 37, 13, M_PI / 7.0f, -M_PI / 7.0f, false);
    ctx.lineTo(31, 37);
    ctx.fill(mesh);

    for (int i = 0; i < 8; i++) {
        ctx.fillStyle = Gold;
        ctx.fillRect(mesh, 51 + i * 16, 35, 4, 4);
    }

    for (int i = 0; i < 6; i++) {
        ctx.fillStyle = Gold;
        ctx.fillRect(mesh, 115, 51 + i * 16, 4, 4);
    }

    for (int i = 0; i < 8; i++) {
        ctx.fillStyle = Gold;
        ctx.fillRect(mesh, 51 + i * 16, 99, 4, 4);
    }

    ctx.fillStyle = FireBrick;
    ctx.beginPath();
    ctx.moveTo(83, 116);
    ctx.lineTo(83, 102);
    ctx.bezierCurveTo(83, 94, 89, 88, 97, 88);
    ctx.bezierCurveTo(105, 88, 111, 94, 111, 102);
    ctx.lineTo(111, 116);
    ctx.lineTo(106.333, 111.333);
    ctx.lineTo(101.666, 116);
    ctx.lineTo(97, 111.333);
    ctx.lineTo(92.333, 116);
    ctx.lineTo(87.666, 111.333);
    ctx.lineTo(83, 116);
    ctx.fill(mesh);

    ctx.fillStyle = White;
    ctx.beginPath();
    ctx.moveTo(91, 96);
    ctx.bezierCurveTo(88, 96, 87, 99, 87, 101);
    ctx.bezierCurveTo(87, 103, 88, 106, 91, 106);
    ctx.bezierCurveTo(94, 106, 95, 103, 95, 101);
    ctx.bezierCurveTo(95, 99, 94, 96, 91, 96);
    ctx.moveTo(103, 96);
    ctx.bezierCurveTo(100, 96, 99, 99, 99, 101);
    ctx.bezierCurveTo(99, 103, 100, 106, 103, 106);
    ctx.bezierCurveTo(106, 106, 107, 103, 107, 101);
    ctx.bezierCurveTo(107, 99, 106, 96, 103, 96);
    ctx.fill(mesh);

    ctx.fillStyle = Black;
    ctx.beginPath();
    ctx.arc(101, 102, 2, 0, M_PI * 2.0f, true);
    ctx.fill(mesh);

    ctx.beginPath();
    ctx.arc(89, 102, 2, 0, M_PI * 2.0f, true);
    ctx.fill(mesh);

    resetRenderState();
}

void Sample02_VG_Trig::drawAndroidSVG() {
    mesh.indices.clear();
    mesh.vertices.clear();

    std::vector<Path2D> paths = Path2D::fromSVGFile("assets/android.svg", Unit::px, 96, tesselationFactor);
    for(size_t i = 0; i < paths.size(); ++i) {
        auto &path = paths[i];
        if (path.strokeStyle != Transparent) {
            path.stroke(mesh);
        } else {
            path.fill(mesh);
        }
    }

    resetRenderState();
}

void Sample02_VG_Trig::drawTigerSVG() {
    mesh.indices.clear();
    mesh.vertices.clear();

    std::vector<Path2D> paths = Path2D::fromSVGFile("assets/Ghostscript_Tiger.svg", Unit::px, 96, tesselationFactor);
    for(size_t i = 0; i < paths.size(); ++i) {
        auto &path = paths[i];
        if (path.strokeStyle != Transparent) {
            path.stroke(mesh);
        } else {
            path.fill(mesh);
        }
    }

    resetRenderState();
}