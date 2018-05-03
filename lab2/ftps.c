//
//  Ftps.c
//  Ftp
//
//  Created by yao wei on 1/16/18.
//  Copyright © 2018 maynard. All rights reserved.
//


#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#define BUFF_SIZE 1024
#define FULL_PATH_SIZE 50
#define HEAD_LEN 24
#define DOMAIN AF_INET

int main(int argc, char const *argv[]) {
    int server_fd, new_socket;
    char pad = ' ';
    ssize_t readSize;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char *buffer = malloc(BUFF_SIZE);
    const char *dir = "recvd/";
    char fullPath[FULL_PATH_SIZE];
    char head[HEAD_LEN];
    if (!buffer) {
        fprintf(stderr, "malloc %d bytes memory fails\n", BUFF_SIZE);
        return -1;
    }
    if (argc < 2) {
        perror("too few arguments\n");
        printf("usage: ftps <local-port>\n");
        exit(EXIT_FAILURE);
    }
    //incase recvd dir not exist
    mkdir(dir, 0777);
    const char *port = argv[1];

    // Creating socket file descriptor
    if ((server_fd = socket(DOMAIN, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT,
                   &opt, sizeof(opt))) {
        perror("setsockopt error");
        exit(EXIT_FAILURE);
    }
    address.sin_family = DOMAIN;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    // address.sin_addr.s_addr = INADDR_ANY;
    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    address.sin_port = htons((strtol(port,0,10)));

    if (bind(server_fd, (struct sockaddr *) &address,
             sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 20) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    while (1) {


        if ((new_socket = accept(server_fd, (struct sockaddr *) &address,
                                 (socklen_t *) &addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
            //todo: continue;
        }
        // unsigned int head;
        char *headP = head;
        //read the 4 bytes length and 20 bytes file name in head buffer
        while ((readSize = read(new_socket, headP, head + sizeof(head) - headP))) {
            if (readSize == -1) {
                perror("read file from network error\n");
                exit(EXIT_FAILURE);
            }
            headP += readSize;
        }
        unsigned int fileSize = *((unsigned int *) head);
        fileSize  = ntohl(fileSize);
        //remove the padding space
        for (char *cp = head + sizeof(head) - 1; cp >= head && *cp == pad; cp--) {
            *cp = 0;
        }

        memset(fullPath, 0, sizeof(fullPath));
        strcat(fullPath, dir);
        strcat(fullPath, head + 4);
        FILE *fout = fopen(fullPath, "wb");
        if (!fout) {
            perror("can not write file");
            exit(EXIT_FAILURE);

        }
        
        //use leftSize to record the remaining byte to receive
        unsigned int leftSize = fileSize;
        while ((readSize = read(new_socket, buffer, leftSize > BUFF_SIZE ? BUFF_SIZE : leftSize))) {
            if (readSize == -1) {
                perror("read file from network error\n");
                exit(EXIT_FAILURE);
            }
            leftSize -= readSize;
            //to do: check is there is enough space
            //test if need to calculate the byte
            fwrite(buffer, 1, readSize, fout);
        }
        unsigned int recvSize = fileSize - leftSize;
        recvSize=htonl(recvSize);
        write(new_socket, &recvSize, 4);

        fclose(fout);
        printf("finished recv file %s\n", fullPath + 6);
    }
    if (buffer)
        free(buffer);
    close(new_socket);
    return 0;
}

