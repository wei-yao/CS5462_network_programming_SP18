#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <zconf.h>
#include <errno.h>
#include "tictactoe.h"
#include <sys/time.h>

#define  PLAYER_ID 1
#define  OP_PLAYER_ID 2
#define ROBOT_ON 0
#define TIME_OUT 30

//if ROBOT_ON equal 1, the computer will make random move
int server(const char* portStr) {
    char board[ROWS][COLUMNS];
    int port;
    int sd;
//    int errno;
    struct sockaddr_in server_address;
    struct sockaddr_in from_address;
    struct timeval timeout = {TIME_OUT,0};//8s for timeout
    int from_length;

    port = atoi(portStr);

    //open socket
    if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("Error in opening stream socket");
        exit(1);
    }
    //set timeout
    if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0){
        printf("timeout, errno=%d\n",errno);
        exit(1);
    }



    /*bind the server address to socket*/
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;
    from_length = sizeof(from_address);

    if (bind(sd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        printf("Port already in use, binding socket error\n");
        exit(1);
    }
//    printf("Waiting for player %d\n",OP_PLAYER_ID);
//    printf("Player %d online\n",OP_PLAYER_ID);

    initSharedState(board);
    print_board(board);
    int state;
    int target;
    int turn = OP_PLAYER_ID;
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
            if (sendto(sd, (void *) &target, 4, 0, (struct sockaddr *) &from_address, from_length) < 0) {
                printf("Error in sending to player\n");
                exit(1);
            }
        } else {
            int ret=recvfrom(sd, (void *) &target, 4, 0, (struct sockaddr *) &from_address, (socklen_t *)&from_length);
            if (ret <= 0) {
                printf("Time out\n");
//                printf("Error in receving from player\n");
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
    close(sd);
    return 0;
}