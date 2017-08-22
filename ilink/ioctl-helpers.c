#include "ioctl-helpers.h"

int ioctl_request (struct ifreq *ifr, char *if_name, unsigned short flag)
{
    ifr->ifr_ifru.ifru_addr.sa_family = AF_INET;
    strncpy(ifr->ifr_ifrn.ifrn_name, if_name, IFNAMSIZ-1);

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    int res = ioctl(fd, flag, ifr);
    close(fd);

    return res;
}

char *ioctl_addr (char *if_name, unsigned short flag, unsigned len)
{
    struct ifreq ifr;
    if (ioctl_request(&ifr, if_name, flag) == -1)
        return NULL;

    char *addr = (char *)malloc(len * sizeof(char));
    bzero(addr, len * sizeof(char));
    sprintf(addr, "%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

    return addr;
}

char *ioctl_brdaddr (char *if_name, unsigned short flag, unsigned len)
{
    struct ifreq ifr;
    if (ioctl_request(&ifr, if_name, flag) == -1)
        return NULL;

    char *broadaddr = (char *)malloc(len * sizeof(char));
    bzero(broadaddr, len * sizeof(char));
    sprintf(broadaddr, "%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_broadaddr)->sin_addr));

    return broadaddr;
}
