#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <vector>

#include <glm/vec3.hpp>

struct Triangle
{
    std::vector<glm::vec3> points;
};

#endif // TRIANGLE_H