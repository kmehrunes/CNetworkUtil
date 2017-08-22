#ifndef ILINK_LOW_H
#define ILINK_LOW_H

#include <string.h>
#include <ifaddrs.h>
#include <malloc.h>

#include <linux/if_packet.h>
#include <net/if.h>
#include <arpa/inet.h>

#include "ioctl-helpers.h"

#define IF_NAME_SIZE 16 // the same as IFNAMSIZ in Linux
#define MAC_ADDRESS_LEN 18 // 6*2 + 5 + 1 true only for MAC-48
#define IP_ADDRESS_LEN 40 // 8*4 + 7 + 1 the maximum for IPv6

#define IFADDRS_SIZE sizeof(struct ifaddrs)

#define SIOCGIWNAME 0x8B01

/**
 * Gets all interfaces from the system without any
 * filter, and returns the head of a linked list
 * to them.
 * @return The head of a linked list of the interfaces.
 */
struct ifaddrs *get_ifaddrs ();

/**
  * A wrapper around `freeifaddrs()` which is used
  * to free a linked list of `struct ifaddrs`.
  */
#define free_addrs(addrs) freeifaddrs((struct ifaddrs *)addrs)

/**
  * A macro which checks if an interface has
  * the specified flag or not.
  */
#define if_has_flag(addr, flag) (((struct ifaddrs *)addr)->ifa_flags) & flag

/**
  * A shortcut macro which checks if an interface
  * is a loopback interface or not.
  */
#define if_is_loopback(addr) if_has_flag(addr, IFF_LOOPBACK)

/**
  * A shortcut macro which checks if an interface
  * is up or not.
  */
#define if_is_up(addr) if_has_flag(addr, IFF_UP)

/**
 * Checks if an interface is of type AF_PACKET.
 */
int if_is_packet (struct ifaddrs _if);

/**
 * Returns the number of interfaces which are
 * of type AF_PACKET.
 */
unsigned get_num_interfaces (struct ifaddrs *addrs);

/**
 * Returns the number of interfaces which match
 * the given filter.
 */
unsigned get_num_interfaces_filtered (struct ifaddrs *addrs, int (*filter)(struct ifaddrs));

/**
 * Returns the MAC address of an interface in
 * the standard format of MAC-48 string.
 */
char *get_if_mac (struct ifaddrs *addr);

/**
 * Returns the IP address of an interface as
 * a string.
 * Warning: Wasn't tested with IPv6.
 */
char *get_if_ip (struct ifaddrs *addr);

/**
 * Returns the subnet mask of an interface as
 * a string.
 * Warning: Wasn't tested with IPv6.
 */
char *get_if_netmask (struct ifaddrs *addr);

/**
 * Returns the broadcast IP of an interface as
 * a string.
 * Warning: Wasn't tested with IPv6.
 */
char *get_if_broadcast (struct ifaddrs *addr);

/**
 * Checks if an interface is assigned an IP or
 * not.
 */
int if_has_ip (struct ifaddrs *addr);

/**
 * Checks if an interface is network-ready or not. For
 * an interface to be ready it must: be of AF_PACKET
 * family, not be a loopback, be up, and has IP.
 */
int if_is_network_ready (struct ifaddrs *addrs);

/**
 * @brief Checks if an interface is wireless or not.
 * @param addr A pointer to `struct addr` which represents
 * the interface.
 * @return 1 if the interface is wireless, 0 otherwise.
 */
int if_is_wireless(struct ifaddrs *addr);

#endif
