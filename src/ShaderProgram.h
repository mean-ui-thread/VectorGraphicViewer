#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <iostream>
#include <vector>

#include "AttributeInfo.h"
#include "Shader.h"

struct ShaderProgram
{
    GLuint handle = 0;
    std::vector<AttributeInfo> attributes;
    std::vector<GLint> attributeLocations;
    std::vector<size_t> attributeOffsets;
    GLint vertexSize = 0;

    ShaderProgram(const std::vector<AttributeInfo> &attributes);

    ~ShaderProgram();

    void attach(Shader *shader);

    int link();

    GLint getUniformLocation(const char *uniformName);

    inline void bind()
    {
        glUseProgram(handle);
        for(size_t i = 0; i < attributeLocations.size(); ++i)
        {
            glEnableVertexAttribArray(attributeLocations[i]);
        }
    }

    inline void unbind()
    {
        for(size_t i = 0; i < attributeLocations.size(); ++i)
        {
            glDisableVertexAttribArray(attributeLocations[i]);
        }
        glUseProgram(0);
    }


    inline void setUniform(int32_t uniformLocation, float value)
    {
        glUniform1f(uniformLocation, value);
    }

    inline void setUniform(int32_t uniformLocation, const glm::vec2 &value)
    {
        glUniform2f(uniformLocation, value.x, value.y);
    }

    inline void setUniform(int32_t uniformLocation, const glm::vec3 &value)
    {
        glUniform3f(uniformLocation, value.x, value.y, value.z);
    }

    inline void setUniform(int32_t uniformLocation, const glm::vec4 &value)
    {
        glUniform4f(uniformLocation, value.x, value.y, value.z, value.w);
    }

    inline void setUniform(int32_t uniformLocation, int32_t value)
    {
        glUniform1i(uniformLocation, value);
    }

   inline  void setUniform(int32_t uniformLocation, const glm::ivec2 &value)
    {
        glUniform2i(uniformLocation, value.x, value.y);
    }

    inline void setUniform(int32_t uniformLocation, const glm::ivec3 &value)
    {
        glUniform3i(uniformLocation, value.x, value.y, value.z);
    }

    inline void setUniform(int32_t uniformLocation, const glm::ivec4 &value)
    {
        glUniform4i(uniformLocation, value.x, value.y, value.z, value.w);
    }

    inline void setUniform(int32_t uniformLocation, const glm::mat2 &value)
    {
        glUniformMatrix2fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(value));
    }

    inline void setUniform(int32_t uniformLocation, const glm::mat3 &value)
    {
        glUniformMatrix3fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(value));
    }

    inline void setUniform(int32_t uniformLocation, const glm::mat4 &value)
    {
        glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(value));
    }

   inline void setUniform(int32_t uniformLocation, const std::vector<float> &values)
    {
        glUniform1fv(uniformLocation, (GLsizei)values.size(), values.data());
    }

    inline void setUniform(int32_t uniformLocation, const std::vector<glm::vec2> &values)
    {
        glUniform2fv(uniformLocation, (GLsizei)values.size(), glm::value_ptr(values.front()));
    }

    inline void setUniform(int32_t uniformLocation, const std::vector<glm::vec3> &values)
    {
        glUniform3fv(uniformLocation, (GLsizei)values.size(), glm::value_ptr(values.front()));
    }

    inline void setUniform(int32_t uniformLocation, const std::vector<glm::vec4> &values)
    {
        glUniform4fv(uniformLocation, (GLsizei)values.size(), glm::value_ptr(values.front()));
    }

    inline void setUniform(int32_t uniformLocation, const std::vector<int32_t> &values)
    {
        glUniform1iv(uniformLocation, (GLsizei)values.size(), values.data());
    }

    inline void setUniform(int32_t uniformLocation, const std::vector<glm::ivec2> &values)
    {
        glUniform2iv(uniformLocation, (GLsizei)values.size(), glm::value_ptr(values.front()));
    }

    inline void setUniform(int32_t uniformLocation, const std::vector<glm::ivec3> &values)
    {
        glUniform3iv(uniformLocation, (GLsizei)values.size(), glm::value_ptr(values.front()));
    }

    inline void setUniform(int32_t uniformLocation, const std::vector<glm::ivec4> &values)
    {
        glUniform4iv(uniformLocation, (GLsizei)values.size(), glm::value_ptr(values.front()));
    }

    inline void setUniform(int32_t uniformLocation, const std::vector<glm::mat2> &values)
    {
        glUniformMatrix2fv(uniformLocation, (GLsizei)values.size(), GL_FALSE, glm::value_ptr(values.front()));
    }

    inline void setUniform(int32_t uniformLocation, const std::vector<glm::mat3> &values)
    {
        glUniformMatrix3fv(uniformLocation, (GLsizei)values.size(), GL_FALSE, glm::value_ptr(values.front()));
    }

    inline void setUniform(int32_t uniformLocation, const std::vector<glm::mat4> &values)
    {
        glUniformMatrix4fv(uniformLocation, (GLsizei)values.size(), GL_FALSE, glm::value_ptr(values.front()));
    }
};

#endif // SHADERPROGRAM_H
