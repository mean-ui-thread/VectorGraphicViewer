#ifndef EXAMPLE01_TEXTURE_H
#define EXAMPLE01_TEXTURE_H

#include "AbstractSample.h"
#include "VertexData.h"

#include "ShaderProgram.h"
#include "Texture.h"
#include "VertexBuffer.h"

class Sample01_Texture : public AbstractSample {

public:

    inline Sample01_Texture(const std::string &name) : AbstractSample(name) {}
    virtual void resetRenderState() override;
    virtual bool setup() override;
    virtual void teardown() override;
    virtual void render(const glm::mat4 &mvp) override;
    virtual void renderUI() override;
    virtual void animateVertices(double t) override;
    virtual std::vector<glm::vec3> getVertices() const override;

private:
    std::shared_ptr<ShaderProgram> program;
    std::shared_ptr<Texture> texture;
    std::shared_ptr<VertexBuffer> vbo;

    GLint u_MVP = -1;
    GLint u_texture0 = -1;

    std::vector<TextureVertex> vertices;
    Texture::Filtering filtering;
};

#endif // EXAMPLE01_TEXTURE_H