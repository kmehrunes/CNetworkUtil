#ifndef TIMEIO_H
#define TIMEIO_H

#include <sys/time.h>
#include <math.h>
#include <stdint.h>

#define MILLIS 1000
#define MICROS 1000000

typedef struct {
    uint32_t seconds;
    uint64_t microseconds;
} Timeout;

static Timeout microseconds (uint64_t us)
{
    uint32_t secs = us/MICROS;
    return (Timeout) {
        secs,
        us - (secs * MICROS)
    };
}

static Timeout milliseconds (uint64_t ms)
{
    return microseconds(ms*MICROS/MILLIS);
}

static Timeout seconds (uint32_t s)
{
    return (Timeout) {
        s,
        0
    };
}

static struct timeval sys_time (Timeout timeout)
{
    return (struct timeval) {
        timeout.seconds,
        timeout.microseconds
    };
}

#endif
