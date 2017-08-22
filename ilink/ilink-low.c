#include "ilink-low.h"

int if_is_af_packet (struct ifaddrs _if)
{
    return _if.ifa_addr->sa_family == AF_PACKET;
}

unsigned get_num_interfaces (struct ifaddrs *addrs)
{
    return get_num_interfaces_filtered(addrs, if_is_packet);
}

unsigned get_num_interfaces_filtered (struct ifaddrs *addrs, int (*filter)(struct ifaddrs))
{
    struct ifaddrs *addrs_pos = addrs;
    int num_interfaces = 0;

    for (; addrs_pos; addrs_pos = addrs_pos->ifa_next)
    {
        if (filter(*addrs_pos))
            num_interfaces++;
    }

    return num_interfaces;
}

struct ifaddrs *get_ifaddrs ()
{
    struct ifaddrs *addrs;
    getifaddrs(&addrs);
    return addrs;
}

char *get_if_mac (struct ifaddrs *addr)
{
    int i;
    char *mac = (char *)malloc(MAC_ADDRESS_LEN * sizeof(char));
    struct sockaddr_ll *sock_addr = (struct sockaddr_ll *)addr->ifa_addr;

    bzero(mac, MAC_ADDRESS_LEN * sizeof(char));

    char *pos_mac = mac;
    for(i = 0; i < 6; i++)
        pos_mac += sprintf(pos_mac, "%02x%s", sock_addr->sll_addr[i], i < 5 ? ":" : "");
    return mac;
}

char *get_if_ip (struct ifaddrs *addr)
{
    return ioctl_addr(addr->ifa_name, SIOCGIFADDR, IP_ADDRESS_LEN);
}

char *get_if_netmask (struct ifaddrs *addr)
{
    return ioctl_addr(addr->ifa_name, SIOCGIFNETMASK, IP_ADDRESS_LEN);
}

char *get_if_broadcast (struct ifaddrs *addr)
{
    return ioctl_brdaddr(addr->ifa_name, SIOCGIFBRDADDR, IP_ADDRESS_LEN);
}

int if_has_ip (struct ifaddrs *addr)
{
    char *ip = get_if_ip(addr);
    if (ip != NULL)
    {
        free(ip);
        return 1;
    }
    else
        return 0;
}

int if_is_wireless (struct ifaddrs *addr)
{
    int fd;
    int res;
    struct ifreq ifr;

    ifr.ifr_ifru.ifru_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_ifrn.ifrn_name, addr->ifa_name, IFNAMSIZ-1);

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    res = ioctl(fd, SIOCGIWNAME, &ifr);
    close(fd);

    return res == 0;
}

int if_is_network_ready (struct ifaddrs *addrs)
{
    return !(addrs->ifa_addr->sa_family != AF_PACKET
             || if_is_loopback(addrs)
             || !if_is_up(addrs)
             || !if_has_ip(addrs));
}
