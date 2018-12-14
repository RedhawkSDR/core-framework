#ifndef TIMING_H
#define TIMING_H

#include <time.h>

inline double get_time()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec + now.tv_nsec*1e-9;
}

#endif
