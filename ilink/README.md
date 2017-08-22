# ilink util

**This is an Afternoon Project -quick small projects that make life a bit easier-**

This utility library consists of few functions for getting basic information about the network interfaces of the system. While this isn't a comprehensive library, it could be extended later should the need for that arises.

> The library was written for Linux, and making it cross-platform wasn't intended. You might need to tweak things here and there to get it to work with other systems.

## Examples

### ilink low-level functions
``` C
struct ifaddrs *addrs = get_ifaddrs();
struct ifaddrs *addrs_pos = addrs;
for (; addrs_pos; addrs_pos = addrs_pos->ifa_next)
{
    if (!if_is_network_ready(addrs_pos))
        continue

    printf("interface: %s (%s)\n", addrs_pos->ifa_name,
            if_is_wireless(addrs_pos)? "wireless" : "not wireless");
}
```

### ilink high-level struct
``` C
struct Interfaces interfaces = get_interfaces();
```

``` C
int interface_filter (struct ifaddrs _if)
{
    return !(_if.ifa_addr->sa_family != AF_PACKET
             || if_is_loopback(&_if)
             || !if_is_up(&_if)
             || !if_has_ip(&_if));
}

struct Interfaces interfaces = get_interfaces_filterd(interface_filter);
```
