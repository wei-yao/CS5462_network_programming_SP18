#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <zconf.h>
#include "tictactoe.h"
#include <signal.h>
#define  PLAYER_ID 2
#define  OP_PLAYER_ID 1
//if ROBOT_ON equal 1, the computer will make random move
#define  ROBOT_ON 0
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

int main(int argc, char const *argv[]) {
//
    char board[ROWS][COLUMNS];
    if (argc < 3) {
        perror("too few arguments\n");
        printf("usage: ./client <remote-IP> <remote-port> \n");
        exit(EXIT_FAILURE);
    }
    const char *ip = argv[1];
    const char *port = argv[2];
    int sock = 0;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(port));
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    memset(serv_addr.sin_zero, '\0', sizeof serv_addr.sin_zero);
    signal(SIGPIPE, SIG_IGN);
    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    initSharedState(board);
    print_board(board);
    int state;
    int turn = OP_PLAYER_ID;
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
            if (send(sock, (void *) &target, 4, 0) < 0) {
                printf("Error in sending to player\n");
                exit(1);
            }
        } else {
            // if (recv(sock, (void *) &target, 4, 0) < 0) {
            //     printf("Error in receving from player\n");
            //     exit(1);
            // }
            int ret=recv(sock, (void *) &target, 4, 0);
            if (ret < 0) {
                printf("Error in receving from player\n");
                exit(1);
            }
            if (ret==0) {
                printf("peer disconnected\n");
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
