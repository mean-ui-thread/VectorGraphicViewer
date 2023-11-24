#include "Shader.h"

#include <cassert>

Shader::Shader(const std::string &filePath) : filePath(filePath)
{
    ext = filePath.substr(filePath.find_last_of(".") + 1);

    std::ifstream f;
    f.open(filePath);
    if (!f.is_open())
    {
        SDL_LogCritical(0, "Could not open %s", filePath.c_str());
        return;
    }

    std::stringstream shaderStream;
    shaderStream << f.rdbuf();

    source = shaderStream.str();

    f.close();
}
