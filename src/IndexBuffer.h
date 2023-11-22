#ifndef INDEX_BUFFER_H
#define INDEX_BUFFER_H

#include <cstdint>
#include <vector>

#include <glad/glad.h>

#include "AbstractGPUObject.h"

struct IndexBuffer : public AbstractGPUObject
{
    enum Hint
    {
        Stream  = GL_STREAM_DRAW,
        Static  = GL_STATIC_DRAW,
        Dynamic = GL_DYNAMIC_DRAW
    };

    GLuint handle = 0;

    std::vector<uint16_t> indices;
    Hint hint;

    inline IndexBuffer(const std::string &name) : AbstractGPUObject(name)
    {
        glGenBuffers(1, &handle);
    }

    inline ~IndexBuffer()
    {
        glDeleteBuffers(1, &handle);
        handle = 0;
    }

    inline void bind()
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
    }

    inline void unbind()
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    inline void upload(const std::vector<uint16_t> &indices, Hint hint)
    {
        this->indices = indices;
        this->hint = hint;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * indices.size(), indices.data(), hint);
    }

    inline virtual size_t getMemoryUsage() const override {
        return sizeof(uint16_t) * indices.size();
    }

    virtual void renderUI() override;

};

#endif // INDEX_BUFFER_H
