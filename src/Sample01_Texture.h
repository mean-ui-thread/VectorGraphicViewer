#ifndef EXAMPLE01_TEXTURE_H
#define EXAMPLE01_TEXTURE_H

#include "AbstractSample.h"
#include "IndexBuffer.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "VertexBuffer.h"
#include "VertexData.h"

class Sample01_Texture : public AbstractSample {

public:

    inline Sample01_Texture(const std::string &name) : AbstractSample(name) {}
    virtual void resetRenderState() override;
    virtual bool setup() override;
    virtual void teardown() override;
    virtual void render(const glm::mat4 &mvp) override;
    virtual void renderUI() override;
    virtual std::vector<glm::vec3> getVertices() const override;
    virtual std::vector<Triangle> getTriangles() const override;

private:
    std::shared_ptr<ShaderProgram> program;
    std::shared_ptr<Texture> texture;
    std::shared_ptr<VertexBuffer<TextureVertex>> vbo;
    std::shared_ptr<IndexBuffer> ibo;

    GLint u_MVP = -1;
    GLint u_texture0 = -1;
};

#endif // EXAMPLE01_TEXTURE_H