#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include <glad/glad.h>
#include <imgui.h>
#include <memory>

#include "AbstractGPUObject.h"
#include "ShaderProgram.h"
#include "VertexData.h"

template<class Vertex>
struct VertexBuffer : public AbstractGPUObject
{
    enum Hint
    {
        Stream  = GL_STREAM_DRAW,
        Static  = GL_STATIC_DRAW,
        Dynamic = GL_DYNAMIC_DRAW
    };

    GLuint handle = 0;
    std::vector<Vertex> vertices;
    Hint hint;

    inline VertexBuffer(const std::string &name) : AbstractGPUObject(name)
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
            glVertexAttribPointer(attributeLocations[i], attribute.count, attribute.type, attribute.action, program->vertexSize(),  reinterpret_cast<const GLvoid*>(offset));
        }

    }

    inline void unbind()
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    inline void upload(const std::vector<Vertex> &vertices, Hint hint)
    {
        this->vertices = vertices;
        this->hint = hint;
        glBindBuffer(GL_ARRAY_BUFFER, handle);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), hint);
    }

    inline virtual size_t getMemoryUsage() const override {
        return sizeof(Vertex) * vertices.size();
    }

    inline virtual void renderUI() {
        ImGui::SeparatorText(name.c_str());
    }
};


template<>
void VertexBuffer<TextureVertex>::renderUI();


#endif // VERTEXBUFFER_H
