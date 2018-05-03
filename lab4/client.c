#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <zconf.h>
#include <errno.h>
#include "tictactoe.h"
#include <sys/time.h>
#define  PLAYER_ID 2
#define  OP_PLAYER_ID 1
//if ROBOT_ON equal 1, the computer will make random move
#define  ROBOT_ON 0
#define TIME_OUT 15
//define DEBUG enables debug with the local fake server
//#define  DEBUG

/**
 * read from sockid until recv all the bytes of length len , write into buffer
 * @param sockid
 * @param buffer
 * @param len
 * @return the actual bytes recved, -1 if network error
 */
int readData(int sockid, void *buffer, size_t len) {
    size_t left = len;
    ssize_t readSize;
    while ((readSize = read(sockid, buffer, left))) {
        if (readSize == -1) {
            perror("read file from network error\n");
            return -1;
        }
        buffer += readSize;
        left -= readSize;
    }
    return readSize;
}

int client(const char *ip, const char *port) {
//
    char board[ROWS][COLUMNS];
    int sock = 0;
    int serv_len;
//    int errno;
    struct sockaddr_in serv_addr;
    struct timeval timeout = {TIME_OUT,0};

    // open the socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
    // set the timeout
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0){
        printf("timeout, errno= %d\n",errno);
        exit(1);
    }


    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(port));
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_len = sizeof(serv_addr);
    memset(serv_addr.sin_zero, '\0', sizeof serv_addr.sin_zero);
//    signal(SIGPIPE, SIG_IGN);

    initSharedState(board);
    print_board(board);
    int state;
    int turn = PLAYER_ID;
    int target;
    do {
        if (turn == PLAYER_ID) {
            if (!ROBOT_ON) {
                while ((target = nextMove(board, turn)) == -1);
            } else {
                target = mockNextMove(board, turn);
            }
            move(board, turn, target);
            print_board(board);
            target = htonl(target);
            int ret;
            if ((ret=sendto(sock, (void *) &target, 4, 0, (struct sockaddr *) &serv_addr, serv_len)) < 0) {
                printf("Error in sending to player\n");
                exit(1);
            }
//            printf("%d",ret);
        } else {
            int ret=recvfrom(sock, (void *) &target, 4, 0, (struct sockaddr *) &serv_addr, (socklen_t *)&serv_len);
            if (ret <= 0) {
                printf("Time out\n");
                printf("ret is: %d\nError number: %d,%s\n", ret, errno, strerror(errno));
                exit(1);
            }
            target = ntohl(target);
            move(board, turn, target);
            print_board(board);
        }
        turn = 3 - turn;
    } while (!(state = checkwin(board)));
    print_board(board);
    print_result(state);
    close(sock);
    return 0;


}