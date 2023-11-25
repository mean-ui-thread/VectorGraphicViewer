#ifndef SAMPLE02_VG_TRIG_H
#define SAMPLE02_VG_TRIG_H

#include "AbstractSample.h"
#include "IndexBuffer.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "VertexBuffer.h"
#include "VertexData.h"
#include "VectorGraphic.h"



class Sample02_VG_Trig : public AbstractSample {

public:

    enum DrawMode {
        Heart = 0,
        Smiley,
        PacmanGame,
        AndroidSVG,
        // TigerSVG
    };

    inline Sample02_VG_Trig(const std::string &name) : AbstractSample(name) {}
    virtual void resetRenderState() override;
    virtual bool setup() override;
    virtual void teardown() override;
    virtual void render(const std::shared_ptr<ViewerApp> &app, const glm::mat4 &mvp) override;
    virtual void renderUI() override;
    virtual std::vector<glm::vec3> getVertices() const override;
    virtual std::vector<Triangle> getTriangles() const override;

    void roundedRect(Path2D &ctx, float x, float y, float width, float height, float radius) ;

    void draw();
    void drawHeart();
    void drawSmiley();
    void drawPacmanGame();
    void drawAndroidSVG();
    void drawTigerSVG();

private:
    Mesh mesh;
    std::shared_ptr<ShaderProgram> program;
    std::shared_ptr<VertexBuffer<ColorVertex>> vbo;
    std::shared_ptr<IndexBuffer> ibo;
    int drawMode = Heart;

    float tesselationFactor = 100.0f;
    float triangulationTimeMs = 0.0f; 
};

#endif // SAMPLE02_VG_TRIG_H