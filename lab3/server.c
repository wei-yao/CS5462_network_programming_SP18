#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <zconf.h>
#include "tictactoe.h"
#include <signal.h>

#define  PLAYER_ID 1
#define  OP_PLAYER_ID 2
#define ROBOT_ON 0

//if ROBOT_ON equal 1, the computer will make random move
int main(int argc, char const *argv[]) {
    char board[ROWS][COLUMNS];
    int port;
    int sd;
    int connected_sd;
    struct sockaddr_in server_address;
    struct sockaddr_in from_address;
    socklen_t from_length;

    if (argc < 2) {
        printf("Invalid Command. Used ./server <port>");
        exit(1);
    }
    port = atoi(argv[1]);

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Error in opening stream socket");
        exit(1);
    }
    signal(SIGPIPE, SIG_IGN);
    // if (setsockopt(sd, SOL_SOCKET, SO_REUSEPORT,
    //                &opt, sizeof(opt))) {
    //     perror("setsockopt error");
    //     exit(EXIT_FAILURE);
    // }

    /*bind the server address to socket*/
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(sd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        printf("Port already in use, binding socket error\n");
        exit(1);
    }
    printf("Waiting for player %d\n",OP_PLAYER_ID);
    listen(sd, 5);
    connected_sd = accept(sd, (struct sockaddr *) &from_address, &from_length);
    printf("Player %d online\n",OP_PLAYER_ID);

    initSharedState(board);
    print_board(board);
    int state;
    int target;
    int turn = PLAYER_ID;
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
            if (send(connected_sd, (void *) &target, 4, 0) < 0) {
                printf("Error in sending to player\n");
                exit(1);
            }
        } else {
            int ret=recv(connected_sd, (void *) &target, 4, 0);
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
    close(sd);
    close(connected_sd);
    return 0;
}
