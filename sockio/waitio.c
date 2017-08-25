#include "waitio.h"

WaitResult select_wait_result (int result)
{
    switch (result) {
        case -1: return WAIT_ERROR;
        case 0: return WAIT_TIMEOUT;
        default: return WAIT_ON_TIME;
    }
}

WaitResult wait_for_read (int fd, Timeout timeout)
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    struct timeval tv_timeout = sys_time(timeout);

    return select_wait_result(select(fd + 1, &fds, NULL, NULL, &tv_timeout));
}

WaitResult wait_for_write (int fd, Timeout timeout)
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    struct timeval tv_timeout = sys_time(timeout);

    return select_wait_result(select(fd + 1, NULL, &fds, NULL, &tv_timeout));
}
