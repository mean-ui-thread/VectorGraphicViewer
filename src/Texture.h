#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>

#include <glad/glad.h>
#include <SDL2/SDL_image.h>

struct Texture
{
    enum Filtering {
        NoFiltering = 0,
        BilinearFiltering,
        TrilinearFiltering,
        AnisotropicFiltering
    };

    GLuint handle = 0;
    std::string filePath;
    int width = 0;
    int height = 0;
    int filtering = NoFiltering;

    Texture(const std::string &filePath);
    ~Texture();

    int decode();

    void setFiltering(Filtering filtering);

    inline void bind(GLuint textureSlot = 0)
    {
        glActiveTexture(GL_TEXTURE0 + textureSlot);
        glBindTexture(GL_TEXTURE_2D, handle);
    }

    inline void unbind(GLuint textureSlot = 0)
    {
        glActiveTexture(GL_TEXTURE0 + textureSlot);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

};

#endif // TEXTURE_H
