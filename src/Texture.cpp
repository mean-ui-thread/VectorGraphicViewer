#include "Texture.h"

Texture::Texture(const std::string &filePath) : filePath(filePath)
{
    glGenTextures(1, &handle);
}

Texture::~Texture()
{
    glDeleteTextures(1, &handle);
    handle = 0;
}

int Texture::decode()
{
    SDL_Surface* surface = IMG_Load(filePath.c_str());
    if(surface == NULL)
    {
        SDL_LogCritical(0, "Unable to load image %s: %s", filePath.c_str(), IMG_GetError());
        return -1;
    }

    SDL_Surface *surfaceRGBA = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
    if (surfaceRGBA == NULL)
    {
        SDL_LogCritical(0, "Unable to load convert %s to RGBA: %s", filePath.c_str(), SDL_GetError());
        SDL_FreeSurface(surface);
        return -1;
    }

    width = surfaceRGBA->w;
    height = surfaceRGBA->h;

    bind(0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surfaceRGBA->w, surfaceRGBA->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surfaceRGBA->pixels);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    SDL_FreeSurface(surfaceRGBA);
    SDL_FreeSurface(surface);

    return 0;
}

void Texture::setFiltering(Filtering filtering) {
    bind(0);
    if (this->filtering != filtering) {

        switch(filtering) {
            case NoFiltering:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                break;
            case BilinearFiltering:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                break;
            case TrilinearFiltering:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                break;
            default:
                break;
        }
        this->filtering = filtering;
    }
}