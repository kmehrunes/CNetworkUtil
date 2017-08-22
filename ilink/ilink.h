#ifndef ILINK_H
#define ILINK_H

#include "ilink-low.h"

enum link_type { LOOPBACK = 0x2, WIRED = 0x4, WIRELESS = 0x8, ANY = 0x16 };

struct Interface
{
    char name[IF_NAME_SIZE];
    enum link_type type;
    char ip[IP_ADDRESS_LEN];
    char netmask[IP_ADDRESS_LEN];
    char broadcast[IP_ADDRESS_LEN];
    char mac[MAC_ADDRESS_LEN];
    unsigned char has_ip;
};

struct Interfaces
{
    unsigned count;
    struct Interface *interfaces;
};

typedef int (*InterfaceFilterFunc)(struct ifaddrs);

struct Interfaces get_interfaces ();
struct Interfaces get_interfaces_filterd (InterfaceFilterFunc filter);

void move_string (char *dst, char *value);

#endif
