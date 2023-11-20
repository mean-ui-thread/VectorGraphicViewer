#ifndef EXAMPLE00_WELCOME_H
#define EXAMPLE00_WELCOME_H

#include "AbstractSample.h"
#include "VertexData.h"

#include "ShaderProgram.h"
#include "Texture.h"
#include "VertexBuffer.h"

class Sample00_Welcome : public AbstractSample {

public:

    inline Sample00_Welcome(const std::string &name) : AbstractSample(name) {}
    virtual bool setup() override;
    virtual void teardown() override;
    virtual void render(const glm::mat4 &mvp) override;
    virtual void renderVertices(const glm::mat4 &mvp) override;
    virtual void renderTriangles(const glm::mat4 &mvp) override;
    virtual void renderUI() override;

private:


};

#endif // EXAMPLE00_WELCOME_H