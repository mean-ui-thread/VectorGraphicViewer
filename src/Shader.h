#ifndef SHADER_H
#define SHADER_H

#include <cassert>
#include <fstream>
#include <sstream>
#include <string>

#include <glad/glad.h>
#include <SDL2/SDL_log.h>

class Shader {

public:
    Shader(const std::string &filePath);

    ~Shader();

    int compile();

    GLuint handle() const {
        return m_handle;
    }

private:
    GLuint m_handle = 0;
    std::string m_filePath;

};

#endif // SHADER_H
