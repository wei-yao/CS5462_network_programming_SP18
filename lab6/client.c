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
#define  ROBOT_ON 1
#define TIME_OUT 15
//define DEBUG enables debug with the local fake server
//#define  DEBUG
#define INIT_TERMINATE 0
#define  TIMEOUT_LIMIT 5

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
//    int last_recv = -1;
    struct dataExchange data;
    struct sockaddr_in serv_addr;
    struct timeval timeout = {TIME_OUT,0};
    struct dataExchange last_send;
    struct dataExchange last_rcv;
    int game_no=-1;
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
    last_send=data;
    Sendto(sock, (void *)&data, sizeof(data), 0, &serv_addr, serv_len);//send game request
//    if(Recvfrom(sock, (void *)&data, sizeof(data), 0, &serv_addr, &serv_len)<=0)
//    {
//        printf("connection time out\n");
//        return -1;
//    };//get response from server to create a new game
//    //detect whether create success. If success client moves first.
//    if (ntohl(data.command) == NEWGAME&&ntohl(data.game_number) != -1) {
//        printf("begin new game no %d\n",ntohl(data.game_number));
////        while((target = nextMove(board, PLAYER_ID)) == -1);
//        if(ROBOT_ON){
//            target=mockNextMove(board);
//        }else{
//            while ((target = nextMove(board, PLAYER_ID)) == -1);
//        }
//        move(board, PLAYER_ID, target);
//        print_board(board);
//        data.move = htonl(target);
//        data.command = htonl(MOVE);
//        Sendto(sock, (void *)&data, sizeof(data), 0, &serv_addr, serv_len);
//    }
//    else if (ntohl(data.game_number) == -1) {
//        printf("There are no more game slots available\n");
//        exit(1);
//    }
//    else {
//        printf("Create failed\n");
//        exit(1);
//    }
    int timeout_cnt=0;
    do {
        int recvRet;
        if((recvRet=Recvfrom(sock, (void *)&data, sizeof(data), 0, &serv_addr, &serv_len))<=0||!memcmp(&data,&last_rcv,sizeof(data))){
            //timeout, or rec dumplicate package
//            printf("dup %d recvRet %d",duplicate,recvRet);
            if(!memcmp(&data,&last_rcv,sizeof(data))){
                printf("recv duplicate package: %d %d resend \n",ntohl(data.command),ntohl(data.move));
            }else{
                printf("time out, resend last move  %d %d\n",ntohl(last_send.command),ntohl(last_send.move));
            }

            Sendto(sock, (void *)&last_send, sizeof(data), 0, &serv_addr, serv_len);
            if(timeout_cnt++>TIMEOUT_LIMIT)
            {
                perror("time out exceed max limits");
                return -1;
            }
            continue;
        }
//        printf("equal %d")
//        printf("last move, %d %d %d",ntohl(last_rcv.command),ntohl(last_rcv.move),ntohl(last_rcv.game_number));
//        printf("last move, %d %d %d",ntohl(data.command),ntohl(data.move),ntohl(data.game_number));
        last_rcv=data;
        timeout_cnt = 0;
        if (ntohl(data.command) == NEWGAME) {
            if(ntohl(data.game_number)!=-1) {
                game_no=ntohl(data.game_number);
                printf("begin new game no %d\n", ntohl(data.game_number));
                if (ROBOT_ON) {
                    target = mockNextMove(board);
                } else {
                    while ((target = nextMove(board, PLAYER_ID)) == -1);
                }
                move(board, PLAYER_ID, target);
                print_board(board);
                data.move = htonl(target);
                data.command = htonl(MOVE);
                Sendto(sock, (void *) &data, sizeof(data), 0, &serv_addr, serv_len);
                last_send=data;
            }else{
                printf("There are no more game slots available\n");
                exit(1);
            }
        }else if(ntohl(data.command) == MOVE) {
            //should check if time out here
            data.move = ntohl(data.move);
            printf("Recieve %d\n", data.move);
//            last_recv = data.move;
//            int gameNo = ntohl(data.game_number);
            move(board, OP_PLAYER_ID, data.move);
            print_board(board);

            //game is over, clinets lose
            if ((state = checkwin(board)) != 0) {
                // loser responds with ENDGAME command and game number
                data.command = htonl(ENDGAME);
                data.move = htonl(0);
                Sendto(sock, (void *) &data, sizeof(data), 0, &serv_addr, serv_len);
                last_send=data;
            } else {
                if (ROBOT_ON) {
                    target = mockNextMove(board);
                } else {
                    while ((target = nextMove(board, PLAYER_ID)) == -1);
                }
                move(board, PLAYER_ID, target);
                data.move = htonl(target);
//            data.game_number = htonl(data.game_number);
                Sendto(sock, (void *) &data, sizeof(data), 0, &serv_addr, serv_len);
                last_send=data;
                print_board(board);

            }
        }else if (ntohl(data.command) == ENDGAME) {
            //client lose, already sent end game request
            //should tell between ack and initial request in the package
            state = checkwin(board);
            printf("game  %d finished", game_no);
            if(ntohl(last_send.command)==ENDGAME){

                print_board(board);
                print_result(state);
//                break;
            }else{
                Sendto(sock, (void *) &data, sizeof(data), 0, &serv_addr, serv_len);
                last_send=data;
                print_result(state);
//                break;
            }
            break;
        }

    } while (1);
    close(sock);
    return 0;
}