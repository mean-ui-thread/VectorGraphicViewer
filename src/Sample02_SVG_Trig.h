#ifndef SAMPLE02_SVG_TRIG_H
#define SAMPLE02_SVG_TRIG_H

#include "AbstractSample.h"
#include "IndexBuffer.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "VertexBuffer.h"
#include "VertexData.h"
#include "VectorGraphic.h"

class Sample02_SVG_Trig : public AbstractSample {

public:

    inline Sample02_SVG_Trig(const std::string &name) : AbstractSample(name) {}
    virtual void resetRenderState() override;
    virtual bool setup() override;
    virtual void teardown() override;
    virtual void render(const glm::mat4 &mvp) override;
    virtual void renderUI() override;
    virtual std::vector<glm::vec3> getVertices() const override;
    virtual std::vector<Triangle> getTriangles() const override;

private:
    Mesh mesh;
    std::shared_ptr<ShaderProgram> program;
    std::shared_ptr<VertexBuffer<ColorVertex>> vbo;
    std::shared_ptr<IndexBuffer> ibo;
};

#endif // SAMPLE02_SVG_TRIG_H