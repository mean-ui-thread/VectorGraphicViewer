#include "CPUUsage.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sys/times.h>


CPUUsage::CPUUsage() {

    tms timeSample;
    m_lastCPU = times(&timeSample);
    m_lastSysCPU = timeSample.tms_stime;
    m_lastUserCPU = timeSample.tms_utime;

    FILE* file = fopen("/proc/cpuinfo", "r");

    char line[128];
    while(fgets(line, 128, file) != NULL) {
        if (strncmp(line, "processor", 9) == 0) {
            ++m_numProcessors;
        }
    }
    fclose(file);
}

float CPUUsage::getValuePercent() {
    float percent;

    struct tms timeSample;
    clock_t now = times(&timeSample);
    if (now <= m_lastCPU || timeSample.tms_stime < m_lastSysCPU || timeSample.tms_utime < m_lastUserCPU) {
        percent = 0.0; // Overflow detected.
    }
    else {
        percent = (timeSample.tms_stime - m_lastSysCPU) + (timeSample.tms_utime - m_lastUserCPU);
        percent /= (now - m_lastCPU);
        percent /= m_numProcessors;
        percent *= 100;
    }
    m_lastCPU = now;
    m_lastSysCPU = timeSample.tms_stime;
    m_lastUserCPU = timeSample.tms_utime;

    return percent;
}