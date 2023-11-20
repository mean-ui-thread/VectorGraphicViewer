#ifndef SAMPLE_DATA_H
#define SAMPLE_DATA_H

#include <vector>

#include <glm/vec2.hpp>

struct SampleData {

    size_t maxSize;
    size_t offset;
    std::vector<glm::vec2> data;

public:
    SampleData(const size_t maxSize) :
        maxSize(maxSize)
    {
        offset = 0;
        data.reserve(maxSize);
    }

    inline void clear() {
        if (data.size() > 0) {
            data.clear();
            offset = 0;
        }
    }

    inline void addPoint(float x, float y) {

        if (data.size() < maxSize) {
            data.push_back(glm::vec2(x, y));
        } else {
            data[offset] = glm::vec2(x, y);
            offset = (offset + 1) % maxSize;
        }
    }

};

#endif // SAMPLE_DATA_H