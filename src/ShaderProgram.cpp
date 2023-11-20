
#include "ShaderProgram.h"

ShaderProgram::ShaderProgram(const std::vector<AttributeInfo> &attributes) :
        m_attributes(attributes)
{
    m_attributeLocations.resize(attributes.size(), -1);
    m_attributeOffsets.resize(attributes.size(), 0);
    m_handle = glCreateProgram();
}

ShaderProgram::~ShaderProgram()
{
    if (m_handle)
    {
        glDeleteProgram(m_handle);
        m_handle = 0;
    }
}

void ShaderProgram::attach(Shader *shader)
{
    glAttachShader(m_handle, shader->handle());
}

int ShaderProgram::link()
{
    glLinkProgram(m_handle);

    GLint linkStatus;
    glGetProgramiv(m_handle, GL_LINK_STATUS, &linkStatus);
    if (!linkStatus)
    {
        GLchar infoLog[1024];
        glGetProgramInfoLog(m_handle, sizeof(infoLog), NULL, infoLog);
        SDL_LogCritical(0, "Could not link shader program:\n%s", infoLog);
        return -1;
    }

    m_vertexSize = 0;
    for(size_t i = 0; i < m_attributes.size(); ++i)
    {
        m_attributeLocations[i] = glGetAttribLocation(m_handle, m_attributes[i].name.c_str());
        if (m_attributeLocations[i] == -1)
        {
            SDL_LogCritical(0, "Could not find attribute named \"%s\" in shader program.", m_attributes[i].name.c_str());
            return -1;
        }

        m_attributeOffsets[i] = m_vertexSize;
        m_vertexSize += m_attributes[i].size();
    }


    return 0;
}

GLint ShaderProgram::getUniformLocation(const char *uniformName)
{
    GLint location = glGetUniformLocation(m_handle, uniformName);
    if (location == -1)
    {
        SDL_LogCritical(0, "Could not find uniform named \"%s\" in shader program.", uniformName);
    }
    return location;
}