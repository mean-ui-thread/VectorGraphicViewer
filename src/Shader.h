#ifndef SHADER_H
#define SHADER_H

#include <cassert>
#include <fstream>
#include <sstream>
#include <string>

#include <glad/glad.h>
#include <SDL2/SDL_log.h>

struct Shader
{
    GLuint handle = 0;
    std::string filePath;

    Shader(const std::string &filePath);

    ~Shader();

    int compile();
};

#endif // SHADER_H
