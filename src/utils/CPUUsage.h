#ifndef CPU_USAGE_H
#define CPU_USAGE_H

#include <cstdint>

class CPUUsage {
public:
    CPUUsage();

    float getValuePercent();

private:
#ifdef __linux__
    uint64_t lastTotalUser;
    uint64_t lastTotalUserLow;
    uint64_t lastTotalSys;
    uint64_t lastTotalIdle;
#endif

#ifdef __APPLE__
    float calculateCPULoad(uint64_t idleTicks, uint64_t totalTicks);
    uint64_t _previousTotalTicks = 0;
    uint64_t _previousIdleTicks = 0;
#endif
};


#endif // CPU_USAGE_H