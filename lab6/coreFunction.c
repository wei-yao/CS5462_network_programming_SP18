#include "coreFunction.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
int Socket(int family, int type, int protocol) {
    int sd;

    if ((sd = socket(family, type, protocol)) < 0) {
        printf("Error in opening stream socket");
        exit(1);
    }

    return sd;
}

void Bind(int sockfd, struct sockaddr_in* my_addr,int addrlen) {
    int ret;

    if ((ret = bind(sockfd, (struct sockaddr *) my_addr, addrlen)) < 0) {
        printf("Port already in use, binding socket error\n");
        exit(1);
    }
}

int Sendto(int sockfd, const void* msg, int len, unsigned int flags, const struct sockaddr_in* to, int tolen) {
    int ret;
    if((ret = sendto(sockfd, msg, len, flags, (struct sockaddr *)to, tolen)) < 0) {
        printf("%s\n", strerror(errno));
        exit(1);
    }

    return ret;
}

int Recvfrom (int sockfd, void* buf, int len, unsigned int flags, const struct sockaddr_in* from, int* fromlen) {
    int ret;
    ret = recvfrom(sockfd, buf, len, flags, (struct sockaddr *)from, (socklen_t *)fromlen);
    return ret;
}