#ifndef WAITIO_H
#define WAITIO_H

#include <unistd.h>
#include "timeoutio.h"

typedef enum { WAIT_ON_TIME, WAIT_TIMEOUT, WAIT_ERROR } WaitResult;

WaitResult wait_for_read (int fd, Timeout timeout);
WaitResult wait_for_write (int fd, Timeout timeout);

WaitResult select_wait_result (int result);

#endif
