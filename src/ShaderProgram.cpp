
#include "ShaderProgram.h"

ShaderProgram::ShaderProgram(const std::vector<AttributeInfo> &attributes) :
        attributes(attributes)
{
    attributeLocations.resize(attributes.size(), -1);
    attributeOffsets.resize(attributes.size(), 0);
    handle = glCreateProgram();
}

ShaderProgram::~ShaderProgram()
{
    if (handle)
    {
        glDeleteProgram(handle);
        handle = 0;
    }
}

void ShaderProgram::attach(Shader *shader)
{
    glAttachShader(handle, shader->handle);
}

int ShaderProgram::link()
{
    glLinkProgram(handle);

    GLint linkStatus;
    glGetProgramiv(handle, GL_LINK_STATUS, &linkStatus);
    if (!linkStatus)
    {
        GLchar infoLog[1024];
        glGetProgramInfoLog(handle, sizeof(infoLog), NULL, infoLog);
        SDL_LogCritical(0, "Could not link shader program:\n%s", infoLog);
        return -1;
    }

    vertexSize = 0;
    for(size_t i = 0; i < attributes.size(); ++i)
    {
        attributeLocations[i] = glGetAttribLocation(handle, attributes[i].name.c_str());
        if (attributeLocations[i] == -1)
        {
            SDL_LogCritical(0, "Could not find attribute named \"%s\" in shader program.", attributes[i].name.c_str());
            return -1;
        }

        attributeOffsets[i] = vertexSize;
        vertexSize += attributes[i].count * attributes[i].sizeOfType();
    }


    return 0;
}

GLint ShaderProgram::getUniformLocation(const char *uniformName)
{
    GLint location = glGetUniformLocation(handle, uniformName);
    if (location == -1)
    {
        SDL_LogCritical(0, "Could not find uniform named \"%s\" in shader program.", uniformName);
    }
    return location;
}