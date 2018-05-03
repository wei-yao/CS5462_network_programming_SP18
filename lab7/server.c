#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <zconf.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#include "tictactoe.h"
#include "coreFunction.h"

// max number of game supported
#define  GAME_NUM 2
#define  PLAYER_ID 2
#define  OP_PLAYER_ID 1
//#define ROBOT_ON 0
#define TIME_OUT 10
#define INIT_TERMINATE 0
//todo add timeout for each game session
int server(const char *portStr);
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: ./tictactoeServer <port> \n");
        return -1;
    }
    return server(argv[1]);
}
/*
server respond to clients
 return value: 0: not finished, 1: finished , -1: receive invalid move
*/
int respond(int connected_sd, struct dataExchange *data, int gameNo, struct GAME games[]) {
    int state = 0;
    int target;

    data->command = ntohl(data->command);
    data->move = ntohl(data->move);
    data->game_number = ntohl(data->game_number);

    if (data->command == MOVE) {
        if(data->game_number!=gameNo)
        {
            printf("invalid gameNumber \n");
            return -1;
        }
//        printf("Game id %d\n", gameNo);
        if(move(games[gameNo].board, OP_PLAYER_ID, data->move)){
            printf("invalid move \n");
            return -1;
        }
        if ((state = checkwin(games[gameNo].board)) != 0) {
//            print_result(state);
            //recv the handshake, then close the connection
            games[gameNo].initEndGame=1;
            data->command = htonl(ENDGAME);
            data->move = htonl(-1);
            data->game_number = htonl(data->game_number);
            Send(connected_sd, (void *) data, sizeof(*data), 0);

        }
        else {
            target = mockNextMove(games[gameNo].board);
            move(games[gameNo].board, PLAYER_ID, target);
            printf("make move %d",target);
            data->command = htonl(MOVE);
            data->move = htonl(target);
            data->game_number = htonl(data->game_number);

            Send(connected_sd, (void *) data, sizeof(*data), 0);

        }
    }
    else if (data->command == NEWGAME) {
        data->command = htonl(NEWGAME);
        data->move = htonl(-1);
        data->game_number = htonl(gameNo);
        Send(connected_sd, (void *) data, sizeof(*data), 0);
    }else if(data->command==ENDGAME){
        //check if I initiate the end handshake
        //check game No field?
        if((state = checkwin(games[gameNo].board))){
            print_board(games[gameNo].board);
            print_result(state);
            if(!games[gameNo].initEndGame){
                data->command = htonl(ENDGAME);
                data->move = htonl(-1);
                data->game_number = htonl(data->game_number);
                Send(connected_sd, (void *) data, sizeof(*data), 0);
            }
            return 1;
        }else{
            //game is not finished, but receive end signal
            printf("receive invalid eng game move \n");
            return -1;
        }

    }
    return 0;
}

int server(const char *portStr) {
    struct GAME games[GAME_NUM];
//    int game_cnt = 0;//count the number of games that is playing
    for (int i = 0; i < GAME_NUM; i++) {
        initGame(&games[i]);
    }
    int port;
    int sd;
    int connected_sd;
//    time_t now;
    int clientSDList[GAME_NUM] = {0};//client socket list
    fd_set socketFDS;//set of file descriptor
    int maxSD;
    int rc;
    int state;

    //the data struct to recv and send
    struct dataExchange data;
    struct sockaddr_in server_address;
    struct sockaddr_in from_address;
    struct timeval timeout = {TIME_OUT, 0};//timeout
    int from_length;

    port = atoi(portStr);
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0){
        printf("timeout, errno= %d\n",errno);
        exit(1);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;
    from_length = sizeof(from_address);

    Bind(sd, &server_address, sizeof(server_address));

    listen(sd, 5);
    maxSD = sd;

 
    do {
        printf("Waiting for clients' move.\n");
        FD_ZERO(&socketFDS);
        FD_SET(sd, &socketFDS);
        for (int i = 0; i < GAME_NUM; i++) {
            if (clientSDList[i] > 0) {
                FD_SET(clientSDList[i], &socketFDS);
                maxSD = (clientSDList[i] > maxSD) ? clientSDList[i] : maxSD;
            }
        }
        //what if select time out
        rc = select(maxSD + 1, &socketFDS, NULL, NULL, NULL);
        printf("rc %d",rc);
        if (FD_ISSET(sd, &socketFDS)) {
            printf("sd is set.\n");
            connected_sd = accept(sd, (struct sockaddr *) &from_address, (socklen_t*)&from_length);
            int available=0;
            for (int i=0; i < GAME_NUM; ++i) {
                if (clientSDList[i] == 0){
//                    FD_SET(connected_sd,&socketFDS);
                    clientSDList[i] = connected_sd;
                    available=1;
                    break;
                }
            }
            //no game slot available
            if(!available){
                struct dataExchange pkg;
                pkg.command = htonl(NEWGAME);
                pkg.move = htonl(-1);
                pkg.game_number = htonl(-1);
                Send(connected_sd, (void *) &pkg, sizeof(pkg), 0);
                close(connected_sd);
            }
        }

        for (int i=0; i<GAME_NUM;i++) {
            if (FD_ISSET(clientSDList[i], &socketFDS)) {
                printf("client %d is set\n", i);
                rc = Recv(clientSDList[i], (void *) &data, sizeof(data), 0);
                if(rc==0||(state = respond(clientSDList[i], &data, i, games)) != 0 ) {
                    initGame(&games[i]);
//                    FD_CLR(clientSDList[i],&socketFDS);
                    printf("client %d is remove\n", i);
                    close(clientSDList[i]);
                    clientSDList[i] = 0;
                }
            }
        }
    } while (1);
    close(sd);
    return 0;
}