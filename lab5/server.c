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

// max number of game supported
#define  GAME_NUM 2
#define  PLAYER_ID 1
#define  OP_PLAYER_ID 2
//#define ROBOT_ON 0
#define TIME_OUT 600

//todo add timeout for each game session
struct GAME{
    char board[ROWS][COLUMNS];
    //whether the board is idle, 0 is idle, 1 is taken
    char idle;
};
void initGame(struct GAME* game){
    game->idle=0;
    initSharedState(game->board);
}
int main(int argc, char *argv[]){
    if (argc < 2) {
        printf("usage: ./tictactoeServer <port> \n");
        return -1;
    }
    return server(argv[1]);
}
int server(const char* portStr) {
    struct GAME  games[GAME_NUM];
    int game_cnt = 0;//count the number of games that is playing
    for(int i=0;i<GAME_NUM;i++){
        initGame(&games[i]);
    }
    int port;
    int sd;

    //the data struct to recv and send
    struct dataExchange data;
    struct sockaddr_in server_address;
    struct sockaddr_in from_address;
//    struct timeval timeout = {TIME_OUT,0};//timeout
    int from_length;

    port = atoi(portStr);

    //open socket
    sd = Socket(AF_INET, SOCK_DGRAM, 0);
    //set timeout
//    if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0){
//        printf("timeout, errno=%d\n",errno);
//        exit(1);
//    }

    /*bind the server address to socket*/
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;
    from_length = sizeof(from_address);

    Bind(sd, &server_address, sizeof(server_address));

    int state;
    int target;
    int current_game;//the game number that server is playing, current_game = data.game_number

    do {
        printf("Waiting for clients' move.\n");
        Recvfrom(sd, (void *)&data, sizeof(data), 0, &from_address, &from_length);
            //time out
        data.command = ntohl(data.command);
        data.move = ntohl(data.move);
        //original order is network order, just keep it
//        data.game_number = ntohl(data.game_number);
        //todo validate data range
        //MOVE 
        if (data.command == MOVE) {
            current_game = ntohl(data.game_number);
            //invalid game number, exceed the range or the correspond game not started
            if(current_game<0||current_game>=GAME_NUM||games[current_game].idle==0)
                continue;
            printf("game id %d\n",current_game);
//            char** currentBoard=games[data.game_number].board;
            //the move is invalid
            if(move(games[current_game].board, OP_PLAYER_ID, data.move)==-1)
                continue;
            // no print board for another user move
            if ((state = checkwin(games[current_game].board)) != 0) {
                print_board(games[current_game].board);
                print_result(state);
                initGame(&games[current_game]);
                game_cnt--;
            }
            else {
                target = mockNextMove(games[current_game].board);
                move(games[current_game].board, PLAYER_ID, target);
                data.command = htonl(data.command);
                data.move = htonl(target);
//                data.game_number = htonl(data.game_number);
                Sendto(sd, (void *)&data, sizeof(data), 0, &from_address, from_length);
//                print_board(games[current_game].board);
                if ((state = checkwin(games[current_game].board)) != 0) {
//                    printf("game id %d",current_game);
                    print_board(games[current_game].board);
                    print_result(state);
                    initGame(&games[current_game]);
                    game_cnt--;
                }
//                else print_board(games[current_game].board);
            }
        }
        //NEWGAME
        if (data.command == NEWGAME) {
            if (game_cnt < GAME_NUM) {
                printf("Create New Game\n");
                int gameNo=0;
                //search for idle game
                for(int i=0;i<GAME_NUM;i++){
                    if(games[i].idle==0){
                        gameNo=i;
                        break;
                    }
                }
                games[gameNo].idle=1;
                //board initialization is done when a game is finished
                game_cnt++;
                target = mockNextMove(games[gameNo].board);
                move(games[gameNo].board, PLAYER_ID, target);
                print_board(games[gameNo].board);
                data.command = htonl(MOVE);
                data.move = htonl(target);
                data.game_number = htonl(gameNo);
                Sendto(sd, (void *)&data, sizeof(data), 0, &from_address, from_length);
                printf("begin a new game id %d\n",gameNo);

            }
            else {
                //no game board available
                data.command = htonl(NEWGAME);
                data.move = htonl(-1);
                data.game_number = htonl(-1);
                Sendto(sd, (void *)&data, sizeof(data), 0, &from_address, from_length);
            }
        }
    } while (1);
    close(sd);
    return 0;
}