#include "Shader.h"

#include <cassert>

Shader::Shader(const std::string &filePath) : m_filePath(filePath)
{
    std::string ext = filePath.substr(filePath.find_last_of(".") + 1);

    if (ext == "vert" || ext == "vsh")
    {
        m_handle = glCreateShader(GL_VERTEX_SHADER);
    }
    else if (ext == "frag" || ext == "fsh")
    {
        m_handle = glCreateShader(GL_FRAGMENT_SHADER);
    }
}

Shader::~Shader() {
    if (m_handle)
    {
        glDeleteShader(m_handle);
        m_handle = 0;
    }
}

int Shader::compile()
{
    assert(m_handle);

    std::ifstream f;
    f.open(m_filePath);
    if (!f.is_open())
    {
        SDL_LogCritical(0, "Could not open %s", m_filePath.c_str());
        return -1;
    }

    std::stringstream shaderStream;
    shaderStream << f.rdbuf();

    std::string sourceCode = shaderStream.str();

    f.close();

    const char * rawSourceCode = sourceCode.c_str();

    glShaderSource(m_handle, 1, &rawSourceCode, NULL);
    glCompileShader(m_handle);

    GLint compileStatus;
    glGetShaderiv(m_handle, GL_COMPILE_STATUS, &compileStatus);

    if (!compileStatus)
    {
        GLchar infoLog[1024];
        glGetShaderInfoLog(m_handle, sizeof(infoLog), NULL, infoLog);
        SDL_LogCritical(0, "Could not compile %s : %s", m_filePath.c_str(), infoLog);
        return -1;
    }

    return 0;
}