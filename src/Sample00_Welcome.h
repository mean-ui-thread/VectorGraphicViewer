#ifndef SAMPLE00_WELCOME_H
#define SAMPLE00_WELCOME_H

#include "AbstractSample.h"
#include "VertexData.h"

#include "ShaderProgram.h"
#include "Texture.h"
#include "VertexBuffer.h"

class Sample00_Welcome : public AbstractSample {

public:

    inline Sample00_Welcome(const std::string &name) : AbstractSample(name) {}
    virtual void resetRenderState() override;
    virtual bool setup() override;
    virtual void teardown() override;
    virtual void render(const glm::mat4 &mvp) override;
    virtual void renderUI() override;
    virtual std::vector<glm::vec3> getVertices() const override;
    virtual std::vector<Triangle> getTriangles() const override;

};

#endif // SAMPLE00_WELCOME_H