#ifndef SAMPLE03_VG_Stencil_H
#define SAMPLE03_VG_Stencil_H

#include "AbstractSample.h"
#include "IndexBuffer.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "VertexBuffer.h"
#include "VertexData.h"
#include "VectorGraphic.h"

struct NVGcontext;
struct NSVGimage;

class Sample03_VG_Stencil : public AbstractSample {

public:

    enum DrawMode {
        Heart = 0,
        Smiley,
        PacmanGame,
        AndroidSVG,
        TigerSVG
    };

    inline Sample03_VG_Stencil(const std::string &name) : AbstractSample(name) {}
    virtual void resetRenderState() override;
    virtual bool setup() override;
    virtual void teardown() override;
    virtual void render(const std::shared_ptr<ViewerApp> &app, const glm::mat4 &mvp) override;
    virtual void renderUI() override;
    virtual std::vector<glm::vec3> getVertices() const override;
    virtual std::vector<Triangle> getTriangles() const override;

    void roundedRect(float x, float y, float width, float height, float radius) ;

    void draw(const std::shared_ptr<ViewerApp> &app, const glm::mat4 &mvp);
    void drawHeart(const std::shared_ptr<ViewerApp> &app, const glm::mat4 &mvp);
    void drawSmiley(const std::shared_ptr<ViewerApp> &app, const glm::mat4 &mvp);
    void drawPacmanGame(const std::shared_ptr<ViewerApp> &app, const glm::mat4 &mvp);
    void drawAndroidSVG(const std::shared_ptr<ViewerApp> &app, const glm::mat4 &mvp);
    void drawTigerSVG(const std::shared_ptr<ViewerApp> &app, const glm::mat4 &mvp);

private:
    NVGcontext *vg = nullptr;
    NSVGimage* androidImage = nullptr;
    NSVGimage* tigerImage = nullptr;

    int drawMode = Heart;

    float stencilTimeMs = 0.0f; 
};

#endif // Stencil