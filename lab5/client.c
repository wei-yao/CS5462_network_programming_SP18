#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <zconf.h>
#include <errno.h>
#include <sys/time.h>
#include "tictactoe.h"
#include "coreFunction.h"
#define  PLAYER_ID 2
#define  OP_PLAYER_ID 1
//if ROBOT_ON equal 1, the computer will make random move
#define  ROBOT_ON 0
#define TIME_OUT 15
//define DEBUG enables debug with the local fake server
//#define  DEBUG
int main(int argc, char *argv[]){
    if (argc < 3) {
        printf("usage: ./tictactoeClient <port> <ip> \n");
        return -1;
    }
    return client(argv[2],argv[1]);
}
int client(const char *ip, const char *port) {
//
    char board[ROWS][COLUMNS];
    int sock = 0;
    int serv_len;

    struct dataExchange data;
    struct sockaddr_in serv_addr;
    struct timeval timeout = {TIME_OUT,0};

    // open the socket
    sock = Socket(AF_INET, SOCK_DGRAM, 0);
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

    data.command = htonl(NEWGAME);
    data.move = htonl(-1);//garbage
    data.game_number = htonl(-1);//garbage

    initSharedState(board);
    int target;
    int state;

    Sendto(sock, (void *)&data, sizeof(data), 0, &serv_addr, serv_len);

    do {
        Recvfrom(sock, (void *)&data, sizeof(data), 0, &serv_addr, &serv_len);
        data.move = ntohl(data.move);
//        data.game_number = ntohl(data.game_number);
        int gameNo=ntohl(data.game_number);
        if (gameNo == -1) {
            printf("There are no more game slots available\n");
            exit(1);
        }
        move(board, OP_PLAYER_ID, data.move);
        print_board(board);
        if ((state = checkwin(board)) != 0) {//game is over
            printf("game id %d",gameNo);
            print_board(board);
            print_result(state);
            break;
        }
        else {//game is not over, client's turn
            if(ROBOT_ON){
                target=mockNextMove(board);
            }else{
                while ((target = nextMove(board, PLAYER_ID)) == -1);
            }
            move(board, PLAYER_ID, target);
            data.move = htonl(target);
//            data.game_number = htonl(data.game_number);
            Sendto(sock, (void *)&data, sizeof(data), 0, &serv_addr, serv_len);

            if ((state = checkwin(board)) != 0) {
                printf("game id %d",gameNo);
                print_board(board);
                print_result(state);
//                Sendto(sock, (void *)&data, 12, 0, &serv_addr, serv_len);
                break;
            }
            else print_board(board);
//            Sendto(sock, (void *)&data, 12, 0, &serv_addr, serv_len);

        }
    } while (1);
    close(sock);
    return 0;
}