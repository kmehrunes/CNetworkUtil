#include "ilink.h"

#include "string.h"
#include "stdarg.h"

enum link_type ifaddrs_type (struct ifaddrs *_if)
{
    if (if_is_loopback(_if))
        return LOOPBACK;
    else if (if_is_wireless(_if))
        return WIRELESS;
    else
        return WIRED;
}

struct Interface interface_from_ifaddrs (struct ifaddrs *_if)
{
    struct Interface interface;
    strcpy(interface.name, _if->ifa_name);

    char *ip = get_if_ip(_if);
    if (ip != NULL)
    {
        strcpy(interface.ip, ip);
        move_string(interface.netmask, get_if_netmask(_if));
        move_string(interface.broadcast, get_if_broadcast(_if));
    }

    move_string(interface.mac, get_if_mac(_if));
    interface.type = ifaddrs_type(_if);
    interface.has_ip = (ip != NULL);

    free(ip);
    return interface;
}

struct Interfaces get_interfaces ()
{
    struct ifaddrs *addrs = get_ifaddrs();
    struct ifaddrs *addrs_pos;
    struct Interface *interfaces;
    unsigned num_interfaces, i = 0;

    num_interfaces = get_num_interfaces(addrs);

    interfaces = (struct Interface *)malloc(sizeof(struct Interface) * num_interfaces);
    for (addrs_pos = addrs; addrs_pos; addrs_pos = addrs_pos->ifa_next)
    {
        if (addrs_pos->ifa_addr->sa_family != AF_PACKET)
            continue;

        interfaces[i++] = interface_from_ifaddrs(addrs_pos);
    }

    freeifaddrs(addrs);
    return (struct Interfaces) {num_interfaces, interfaces};
}

struct Interfaces get_interfaces_filterd (InterfaceFilterFunc filter)
{
    struct ifaddrs *addrs = get_ifaddrs();
    struct ifaddrs *addrs_pos;
    struct Interface *interfaces;
    unsigned num_interfaces, i = 0;

    num_interfaces = get_num_interfaces_filtered(addrs, filter);

    interfaces = (struct Interface *)malloc(sizeof(struct Interface) * num_interfaces);
    for (addrs_pos = addrs; addrs_pos; addrs_pos = addrs_pos->ifa_next)
    {
        if (filter(*addrs_pos))
            interfaces[i++] = interface_from_ifaddrs(addrs_pos);
    }

    freeifaddrs(addrs);
    return (struct Interfaces) {num_interfaces, interfaces};
}

void move_string (char *dst, char *val)
{
    if (val != NULL)
    {
        dst = strcpy(dst, val);
        free(val);
    }
}
