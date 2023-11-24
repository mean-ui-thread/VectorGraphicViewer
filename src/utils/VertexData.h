#ifndef VERTEX_DATA_H
#define VERTEX_DATA_H

#include <glm/glm.hpp>

struct PositionVertex {
    glm::vec3 position;
};

struct TextureVertex {
    glm::vec3 position;
    glm::vec2 texCoord;
};

struct ColorVertex {
    glm::vec3 position;
    glm::u8vec4 color;
};

#endif // VERTEX_DATA_H