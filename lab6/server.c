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
#define  PLAYER_ID 2
#define  OP_PLAYER_ID 1
//#define ROBOT_ON 0
#define TIME_OUT 10
#define INIT_TERMINATE 0
//todo add timeout for each game session
struct GAME {
    char board[ROWS][COLUMNS];
    //whether the board is idle, 0 is idle, 1 is taken
    char idle;//idle == 0 means idle, 1 meands playing
    int server_last_move;
    int client_last_move;
    time_t recent;//store the latest time when recieving from clients
    int timeout_cnt;//number of timeouts
    struct sockaddr_in client_addr;
    int dup_cnt;
};

void initGame(struct GAME *game) {
    game->idle = 0;
    game->timeout_cnt = 0;
    game->server_last_move = -1;
    game->client_last_move = -1;
//    game->client_addr ;
    game->dup_cnt = 0;
    time(&game->recent);
    initSharedState(game->board);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: ./tictactoeServer <port> \n");
        return -1;
    }
    return server(argv[1]);
}

int server(const char *portStr) {
    struct GAME games[GAME_NUM];
    int game_cnt = 0;//count the number of games that is playing
    for (int i = 0; i < GAME_NUM; i++) {
        initGame(&games[i]);
    }
    int port;
    int sd;
    time_t now;

    //the data struct to recv and send
    struct dataExchange data;
    struct sockaddr_in server_address;
    struct sockaddr_in from_address;
    struct timeval timeout = {TIME_OUT, 0};//timeout
    int from_length;

    port = atoi(portStr);

    //open sdet
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    //set timeout
    if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout)) < 0) {
        printf("timeout, errno=%d\n", errno);
        exit(1);
    }
    int on = 1;
    setsockopt(sd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));

    /*bind the server address to sdet*/
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;
    from_address.sin_family = AF_INET;
    from_length = sizeof(from_address);

    Bind(sd, &server_address, sizeof(server_address));

    int state;
    int target;
    int current_game;//the game number that server is playing, current_game = data.game_number

    do {
        printf("Waiting for clients' move.\n");

        if (Recvfrom(sd, (void *) &data, sizeof(data), 0, &from_address, &from_length) <= 0) {
            time(&now);
            //other game may starves
            for (int i = 0; i < GAME_NUM; ++i) {
                if (games[i].idle == 1) {
                    if ((now - games[i].recent) >= TIME_OUT) {
                        printf("Game %d Time out\n", i);
                        printf("now %ld\n", now);
                        printf("%ld\n", games[i].recent);
                        games[i].timeout_cnt++;
                        if (games[i].server_last_move == -1) {
                            //NEWGAME package lost;
                            data.command = htonl(NEWGAME);
                            data.move = htonl(-1);
                            data.game_number = htonl(i);
                            Sendto(sd, (void *) &data, sizeof(data), 0, &(games[i].client_addr), from_length);
                            continue;
                        }
                        data.command = htonl(MOVE);
                        data.move = htonl(games[i].server_last_move);
                        data.game_number = htonl(i);
                        Sendto(sd, (void *) &data, sizeof(data), 0, &(games[i].client_addr), from_length);
                        printf("Send %d\n", games[i].server_last_move);
                        if (games[i].timeout_cnt == 5) {
                            game_cnt--;
                            initGame(&games[i]);
                        }
                    }
                }
            }
            continue;
        }
//        Sendto(sd, (void *) &data, sizeof(data), 0, &from_address, from_length);

        current_game = ntohl(data.game_number);
        data.move = ntohl(data.move);
        data.command = ntohl(data.command);
        if (data.command!=NEWGAME&&(current_game < 0 || current_game > GAME_NUM))
            continue;
        //we can also check if the client addr stored in the games match with the sent package
//        games[current_game].client_addr = &from_address;
//        time(&games[current_game].recent);
        //MOVE 
        if (data.command == MOVE) {
            if (games[current_game].idle == 0) {
                continue;
            }
            games[current_game].timeout_cnt=0;
            printf("game id %d\n", current_game);
//            char** currentBoard=games[data.game_number].board;
            //dumplicate package
            if (games[current_game].client_last_move == data.move) {
                if (games[current_game].dup_cnt++ > 5) {
                    game_cnt--;
                    //todo
                    //bug may exist if the other client does not realize the
                    //connection is closed and try to send package after the same slot is
                    //assigned to another client, then two client will send move to the same slot
                    initGame(&games[current_game]);
                    continue;
                }
                data.move = htonl(games[current_game].server_last_move);
                Sendto(sd, (void *) &data, sizeof(data), 0, &from_address, from_length);
            }
            //reject invalid input
            if (move(games[current_game].board, OP_PLAYER_ID, data.move) == -1) {
                continue;
            }
            games[current_game].dup_cnt = 0;
            games[current_game].client_last_move = data.move;

            //game is over and server lose
            if ((state = checkwin(games[current_game].board)) != 0) {

                data.command = htonl(ENDGAME);
                data.move = htonl(-1);
                games[current_game].server_last_move=INIT_TERMINATE;
                // loser responds with ENDGAME command and game number
                Sendto(sd, (void *) &data, sizeof(data), 0, &from_address, from_length);
                //Recieving winner three way handshake to end the game
                //possibly receive other user's move
//                Recvfrom(sd, (void *) &data, sizeof(data), 0, &from_address, &from_length);

                print_board(games[current_game].board);
                print_result(state);
//                initGame(&games[current_game]);
//                game_cnt--;
            } else {
                target = mockNextMove(games[current_game].board);
                games[current_game].server_last_move = target;
                move(games[current_game].board, PLAYER_ID, target);

                data.command = htonl(data.command);
                data.move = htonl(target);

                Sendto(sd, (void *) &data, sizeof(data), 0, &from_address, from_length);

                //server wins after server's move
                //winner do nothing, wait for the loser to send endgame
                if ((state = checkwin(games[current_game].board)) != 0) {
                    printf("game finished %d",current_game);
                    print_result(state);

                }
//
//                    //winners recieve ENDGAME commadns from loser
//                    Recvfrom(sd, (void *) &data, sizeof(data), 0, &from_address, &from_length);
//
//                    //winners send 2 in command to complete the handshake
//                    Sendto(sd, (void *) &data, sizeof(data), 0, &from_address, from_length);
//
//                    print_board(games[current_game].board);
//                    print_result(state);
//                    initGame(&games[current_game]);
//                    game_cnt--;
//                }
//                else print_board(games[current_game].board);
            }
        }
        if (data.command == NEWGAME) {
            if (game_cnt < GAME_NUM) {
                printf("Create New Game\n");
                int gameNo = 0;
                //search for idle game
                for (int i = 0; i < GAME_NUM; i++) {
                    if (games[i].idle == 0) {
                        gameNo = i;
                        break;
                    }
                }
                games[gameNo].idle = 1;
                //board initialization is done when a game is finished
                game_cnt++;
                data.command = htonl(NEWGAME);
                data.move = htonl(-1);
                data.game_number = htonl(gameNo);
                games[gameNo].client_addr = from_address;
                time(&games[gameNo].recent);
                Sendto(sd, (void *) &data, sizeof(data), 0, &from_address, from_length);
                printf("begin a new game id %d\n", gameNo);
            } else {
                //no game board available
                data.command = htonl(NEWGAME);
                data.move = htonl(-1);
                data.game_number = htonl(-1);
                Sendto(sd, (void *) &data, sizeof(data), 0, &from_address, from_length);
            }
        } else if (data.command == ENDGAME) {
            if (games[current_game].idle == 1) {
                data.command = htonl(ENDGAME);
                data.move = htonl(-1);
                //todo handle shake data lost
                //server does not init terminate, ie not loser

                if(games[current_game].server_last_move!=INIT_TERMINATE)
                     Sendto(sd, (void *) &data, sizeof(data), 0, &from_address, from_length);
                initGame(&games[current_game]);
                game_cnt--;
            }

        }
    } while (1);
    close(sd);
    return 0;
}