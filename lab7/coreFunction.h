/*
The core functions that will be use in the socket programming.
Encapsulate the system funcions in these functions which the first
character is upper case.
*/
#ifndef _COREFUNCTION_H_
#define _COREFUNCTION_H_
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int Socket(int family, int type, int protocol);

void Bind(int sockfd, struct sockaddr_in* my_addr,int addrlen);

int Sendto(int sockfd, const void* msg, int len, unsigned int flags, const struct sockaddr_in* to, int tolen);

int Recvfrom (int sockfd, void* buf, int len, unsigned int flags, const struct sockaddr_in* from, int* fromlen);

int Send(int sockfd, void* buf, int len, unsigned int flags);

int Recv(int sockfd, void* buf, int len, unsigned int flags);
#endif