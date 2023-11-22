#ifndef SAMPLE_DATA_H
#define SAMPLE_DATA_H

#include <vector>

#include <glm/vec2.hpp>

struct SampleData {

    const size_t maxSize;
    std::vector<glm::vec2> data;

    size_t offset = 0;
    float min = 0.0f;
    float max = 0.0f;
    float avg = 0.0f;

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
            min = 0.0f;
            max = 0.0f;
            avg = 0.0f;
        }
    }

    inline void addPoint(float x, float y) {

        if (data.size() < maxSize) {
            data.push_back(glm::vec2(x, y));
        } else {
            data[offset] = glm::vec2(x, y);
            offset = (offset + 1) % maxSize;
        }

        min = FLT_MAX;
		max = -FLT_MAX;
		avg = 0.0f;

        for (size_t i = 0; i < data.size(); ++i)
		{
			const float val = data[i].y;
			min  = std::min(min, val);
			max  = std::max(max, val);
			avg += val;
		}

        avg /= data.size();
    }

};

#endif // SAMPLE_DATA_H