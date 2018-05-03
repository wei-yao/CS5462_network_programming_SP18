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
#define  PLAYER_ID 1
#define  OP_PLAYER_ID 2
//if ROBOT_ON equal 1, the computer will make random move
#define  ROBOT_ON 0
#define TIME_OUT 30
//define DEBUG enables debug with the local fake server
//#define  DEBUG

int client(const char *ip, const char *port);
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
    int sd = 0;
//    int serv_len;
//    int last_recv = -1;
    struct dataExchange data;
    struct sockaddr_in serv_addr;
    struct timeval timeout = {TIME_OUT,0};
//    struct dataExchange last_send;
//    struct dataExchange last_rcv;
    int game_no=-1;
    // open the socket
    sd = Socket(AF_INET, SOCK_STREAM, 0);
    // set the timeout
    if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0){
        printf("timeout, errno= %d\n",errno);
        exit(1);
    }
    struct dataExchange end_package;

    end_package.command=htonl(ENDGAME);
    end_package.move=htonl(-1);
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(port));
    serv_addr.sin_addr.s_addr = inet_addr(ip);
//    serv_len = sizeof(serv_addr);
    memset(serv_addr.sin_zero, '\0', sizeof serv_addr.sin_zero);

    data.command = htonl(NEWGAME);
    data.move = htonl(-1);
    data.game_number = htonl(-1);

    if(connect(sd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))==-1){
        printf("server unavailable %s %s\n",ip,port);
        exit(-1);
    }
    
    initSharedState(board);
    print_board(board);
    int target;
    int state;
//    last_send=data;
    
    Send(sd, (void *) &data, sizeof(data), 0);
    int init_end_shake=0;
//    int timeout_cnt=0;
    int ret=0;
    do {
        int rc;
        rc = Recv(sd, (void *) &data, sizeof(data), 0);
        if (rc == 0) {
            printf("Time out\n");
            break;
        }
        data.command = ntohl(data.command);
        data.move = ntohl(data.move);
        data.game_number = ntohl(data.game_number);

        if (data.command == NEWGAME) {
            printf("gameno %d\n",data.game_number);
            if(data.game_number!=-1) {
                game_no=data.game_number;
                end_package.game_number=htonl(game_no);
//                printf("begin new game no %d\n", ntohl(data.game_number));
                if (ROBOT_ON) {
                    target = mockNextMove(board);
                } else {
                    while ((target = nextMove(board, PLAYER_ID)) == -1);
                }
                move(board, PLAYER_ID, target);
                print_board(board);
                printf("game number %d\n",data.game_number);
                data.move = htonl(target);
                data.command = htonl(MOVE);
                data.game_number = htonl(data.game_number);

                Send(sd, (void *) &data, sizeof(data), 0);
//                last_send=data;
            }else{
                printf("There are no more game slots available\n");
                exit(1);
            }
        }else if(data.command == MOVE) {
            //todo reject invalid move
            move(board, OP_PLAYER_ID, data.move);
            print_board(board);

            //game is over, clinets lose
            if ((state = checkwin(board)) != 0) {
                Send(sd,(void*)&end_package,sizeof(end_package),0);
                init_end_shake=1;
//                break;
            } else {
                if (ROBOT_ON) {
                    sleep(2);
                    target = mockNextMove(board);
                } else {
                    while ((target = nextMove(board, PLAYER_ID)) == -1);
                }
                move(board, PLAYER_ID, target);
//                printf("move to %d",target);
                data.game_number=htonl(data.game_number);
                data.command=ntohl(data.command);

                data.move = htonl(target);
                Send(sd, (void *) &data, sizeof(data), 0);
                print_board(board);
            }
        }else if(data.command==ENDGAME){
            //check if I initiate the end handshake
            //check game No field?
            if((state = checkwin(board))){
                print_board(board);
                print_result(state);
                if(!init_end_shake){
                    Send(sd,(void*)&end_package,sizeof(end_package),0);
                }
                ret=1;
                break;
            }else{
                //game is not finished, but receive end signal
                printf("receive invalid eng game move \n");
                ret=-1;
                break;
            }

        }
    } while (1);
//    print_board(board);
//    print_result(state);
    close(sd);
    return ret;
}