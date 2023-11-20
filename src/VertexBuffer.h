#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include <glad/glad.h>

#include "ShaderProgram.h"

struct VertexBuffer
{
    enum Hint
    {
        Stream  = GL_STREAM_DRAW,
        Static  = GL_STATIC_DRAW,
        Dynamic = GL_DYNAMIC_DRAW
    };

    GLuint handle = 0;

    inline VertexBuffer()
    {
        glGenBuffers(1, &handle);
    }

    inline ~VertexBuffer()
    {
        glDeleteBuffers(1, &handle);
        handle = 0;
    }

    inline void bind(const std::shared_ptr<ShaderProgram> &program)
    {
        glBindBuffer(GL_ARRAY_BUFFER, handle);

        const std::vector<GLint> &attributeLocations = program->attributeLocations();
        for(size_t i = 0; i < attributeLocations.size(); ++i)
        {
            const AttributeInfo &attribute = program->attribute(i);
            size_t offset = program->attributeOffset(i);
            glVertexAttribPointer(attributeLocations[i], attribute.count, attribute.type, GL_FALSE, program->vertexSize(),  reinterpret_cast<const GLvoid*>(offset));
        }

    }

    inline void unbind()
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    template<typename Vertex>
    inline void upload(const std::vector<Vertex> &vertices, Hint hint)
    {
        glBindBuffer(GL_ARRAY_BUFFER, handle);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), hint);
    }

};

#endif // VERTEXBUFFER_H
