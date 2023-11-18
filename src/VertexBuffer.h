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

        for(size_t i = 0; i < program->attributeLocations.size(); ++i)
        {
            glVertexAttribPointer(program->attributeLocations[i], program->attributes[i].count, program->attributes[i].type, GL_FALSE, program->vertexSize,  reinterpret_cast<const GLvoid*>(program->attributeOffsets[i]));
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
