#include "ilink.h"

int interface_filter (struct ifaddrs _if)
{
    return !(_if.ifa_addr->sa_family != AF_PACKET
             || !if_is_up(&_if));
}

int main(int argc, char const *argv[]) {
    struct Interfaces interfaces = get_interfaces_filterd(interface_filter);
    int i;
    for (i = 0; i < interfaces.count; i++) {
        printf("%s\n\t%s\n\t%s\n\t%s\n\t%s\n\t%d\n",
                interfaces.interfaces[i].name,
                interfaces.interfaces[i].has_ip? interfaces.interfaces[i].ip : NULL,
                interfaces.interfaces[i].has_ip? interfaces.interfaces[i].netmask : NULL,
                interfaces.interfaces[i].has_ip? interfaces.interfaces[i].broadcast : NULL,
                interfaces.interfaces[i].mac,
                (int)interfaces.interfaces[i].type);
    }
    free(interfaces.interfaces);
    return 0;
}
