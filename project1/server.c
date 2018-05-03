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
#include <pthread.h>
#include "tictactoe.h"
#include "coreFunction.h"

// max number of game supported
#define  GAME_NUM 15
#define  PLAYER_ID 2
#define  OP_PLAYER_ID 1
//#define ROBOT_ON 0
#define TIME_OUT 30
#define INIT_TERMINATE 0
#define MC_PORT 1818
#define MC_GROUP "239.0.0.1"
//todo add timeout for each game session
int server(const char *portStr);
void multicast(int port);//thread to response the port to client
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
    else if (data->command == CONTINUE) {
        //todo reject invalid old board state here
        memcpy(&games[gameNo].board, &data->board, sizeof(data->board));//assign the board to the game
        if ((state = checkwin(games[gameNo].board)) != 0) {
            print_board(games[gameNo].board);
            print_result(state);
            data->command = htonl(ENDGAME);
            data->move = htonl(-1);
            data->game_number = htonl(gameNo);
            Send(connected_sd, (void *) data, sizeof(*data), 0);
            return 1;
        }
        target = mockNextMove(games[gameNo].board);
        move(games[gameNo].board, PLAYER_ID, target);
        printf("make move %d",target);
        data->command = htonl(CONTINUE);
        data->move = htonl(target);
        data->game_number = htonl(gameNo);
        Send(connected_sd, (void *) data, sizeof(*data), 0);
    }else if (data->command == NEWGAME) {
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
    pthread_t t;
    struct GAME games[GAME_NUM];
//    int game_cnt = 0;//count the number of games that is playing
    for (int i = 0; i < GAME_NUM; i++) {
        initGame(&games[i]);
    }
    int port;
    int sd;
    int connected_sd;
    port = atoi(portStr);
    pthread_create(&t, NULL, (void*)&multicast,port);//create multicast thread
    //pthread_join(t, NULL);
//    time_t now;
    int clientSDList[GAME_NUM] = {0};//client socket list
    fd_set socketFDS;//set of file descriptor
    int maxSD;
    int rc;
    int state;

    //the data struct to recv and send
    struct dataExchange data;
    struct sockaddr_in server_address, from_address;
    struct timeval timeout = {TIME_OUT, 0};//timeout
    int from_length;

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
                if(rc<=0||(state = respond(clientSDList[i], &data, i, games)) != 0 ) {
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

void multicast(int port){
    struct ip_mreq {
        struct in_addr imr_multiaddr; /* IP multicast address of group */
        struct in_addr imr_interface; /* local IP address of interface */
    };
    printf("multicast thread start\n");
    int multicast_sd;
    int s_port;
    struct ip_mreq mreq;
    struct sockaddr_in multicast_addr;
    if((multicast_sd= socket(AF_INET, SOCK_DGRAM, 0))<0){
        perror("open socket error");
        exit(1);
    }
//    printf("Hello, World!\n");

    memset(&multicast_addr,0, sizeof(multicast_addr));
    multicast_addr.sin_family=AF_INET;
    multicast_addr.sin_port=htons(MC_PORT);
    multicast_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    bind(multicast_sd,(struct sockaddr*)&multicast_addr, sizeof(multicast_addr));
    mreq.imr_multiaddr.s_addr = inet_addr(MC_GROUP);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if(setsockopt(multicast_sd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
               &mreq, sizeof(mreq))<0){
        perror("error in set opt");
        exit(1);
    }
    char msg;
    int addrlen= sizeof(multicast_addr);
    s_port = htonl(port);

    while(1) {
        printf("Multicast receive\n");
        recvfrom(multicast_sd, &msg, sizeof(msg), 0,
                               (struct sockaddr *) &multicast_addr, (socklen_t*)&addrlen);
        printf("Recv multicast request, send port %d\n", port);
        sendto(multicast_sd, (void *)&s_port, 4, 0, (struct sockaddr *) &multicast_addr, sizeof(multicast_addr));
    }
//    putchar(msg);
}