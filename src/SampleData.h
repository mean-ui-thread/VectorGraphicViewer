#ifndef SAMPLE_DATA_H
#define SAMPLE_DATA_H

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <climits>

struct SampleData
{
    static constexpr uint32_t SAMPLE_COUNT = 400;

    inline SampleData()
    {
        reset();
    }

    inline void reset()
    {
        m_offset = 0;
        memset(m_values, 0, sizeof(m_values) );

        m_min = 0.0f;
        m_max = 0.0f;
        m_avg = 0.0f;
    }

    inline void pushSample(float value)
    {
        m_values[m_offset] = value;
        m_offset = (m_offset+1) % SAMPLE_COUNT;

        float min = FLT_MAX;
        float max = 0.0f;
        float avg = 0.0f;

        for (uint32_t i = 0; i < SAMPLE_COUNT; ++i)
        {
            const float val = m_values[i];
            min  = std::min(min, val);
            max  = std::max(max, val);
            avg += val;
        }

        m_min = min;
        m_max = max;
        m_avg = avg / SAMPLE_COUNT;
    }

    int32_t m_offset;
    float m_values[SAMPLE_COUNT];

    float m_min;
    float m_max;
    float m_avg;
};

#endif // SAMPLE_DATA_H