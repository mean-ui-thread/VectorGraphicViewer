#ifndef CPU_USAGE_H
#define CPU_USAGE_H

#include <cstdint>
#include <ctime>

class CPUUsage {
public:
    CPUUsage();

    float getValuePercent();

private:
    clock_t m_lastCPU;
    clock_t m_lastSysCPU;
    clock_t m_lastUserCPU;
    int32_t m_numProcessors = 0;
};


#endif // CPU_USAGE_H