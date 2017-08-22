#ifndef IOCTL_HELPERS_H
#define IOCTL_HELPERS_H

#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

int ioctl_request (struct ifreq *ifr, char *if_name, unsigned short flag);

char *ioctl_addr (char *if_name, unsigned short flag, unsigned len);

char *ioctl_brdaddr (char *if_name, unsigned short flag, unsigned len);

#endif
