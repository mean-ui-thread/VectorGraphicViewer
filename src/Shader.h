#ifndef SHADER_H
#define SHADER_H

#include <cassert>
#include <fstream>
#include <sstream>
#include <string>

#include <glad/glad.h>
#include <SDL2/SDL_log.h>

struct Shader {

    Shader(const std::string &filePath);

    std::string ext;
    std::string filePath;
    std::string source;
};

#endif // SHADER_H
