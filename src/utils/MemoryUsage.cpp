#include "MemoryUsage.h"

#ifdef __linux__
#include <cstdlib>
#include <cstdio>
#include <cstring>

float MemoryUsage::getValueKB() const {
    float result = -1.0f;

    char line[128] = {};
    FILE* file = fopen("/proc/self/status", "r");
    while (fgets(line, 128, file) != NULL) {
        if (strncmp(line, "RssAnon:", 8) == 0) {
            int i = strlen(line);
            const char* p = line;
            while (*p <'0' || *p > '9') p++;
            line[i-3] = '\0';
            i = atoi(p);
            result = atof(p);
            break;
        }
    }
    fclose(file);
    return result;
}
#endif

#ifdef __APPLE__

#include <mach/mach.h>
#include <mach/message.h>  // for mach_msg_type_number_t
#include <mach/kern_return.h>  // for kern_return_t
#include <mach/task_info.h>

float MemoryUsage::getValueKB() const {
    
    mach_task_basic_info_data_t taskinfo = {};
    mach_msg_type_number_t outCount = MACH_TASK_BASIC_INFO_COUNT;

    if (KERN_SUCCESS != task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&taskinfo, &outCount)) {
        return -1;
    }

    return taskinfo.resident_size/1024.0f/1024.0f;
}
#endif