//
// Created by 魏尧 on 2/1/18.
//
#ifndef TICTACTOE_TICTACTOE_H
#define TICTACTOE_TICTACTOE_H
#define ROWS  3
#define COLUMNS  3
#define MOVE 0
#define NEWGAME 1
#define ENDGAME 2
#define  PLAYER1_MARK 'X'
#define  PLAYER2_MARK 'O'




/*
The data structure to send or receive
*/
struct dataExchange{
    int command;//1 means NEWGAME, 0 meand MOVE
    int move;//1-9
    int game_number;//0 is first game, 1 is second game
};

/**
 *
 * @param board
 * @return 1 if player 1 win, 2 if player 2 win, 3 if draw, 0 if not ended
 */
int checkwin(char board[ROWS][COLUMNS]);
void print_board(char board[ROWS][COLUMNS]);
/**
 * print the final result win/loss/draw
 * @param player : play id who wins ,1 for player 1, 2 for player 2, 3 for draw
 */
void print_result(int player);
/**
 * random generate next move
 * @param board
 * @param player
 * @return the index of next move (1 to 9), -1 if the all positions have been taken
 *
 */
int mockNextMove(char board[ROWS][COLUMNS]);
/**
 * prompt user to input next move
 * @param board
 * @param player id
 * @return  the index of next move (1 to 9), -1 if the input is valid or position has been taken
 * so you may have to loop this function until get a right ret value
 */

int nextMove(char board[ROWS][COLUMNS],int player);
/**
 * set the board state at index for player
 * @param board
 * @param player id
 * @param index
 * @return 0 if success, -1 if the index is invalid or position has been taken.
 */
int move(char board[ROWS][COLUMNS],int player,int index);

//init the board to '1' to '9'
int initSharedState(char board[ROWS][COLUMNS]);

/*
int Socket(int family, int type, int protocol);

int Bind(int sockfd, struct sockaddr *my_addr,int addrlen);

int Sendto(int sockfd, const void *msg, int len, unsigned int flags, const struct sockaddr *to, int tolen);

int Recvfrom (int sockfd, void *buf, int len, unsigned int flags struct sockaddr *from, int *fromlen);
*/

/*
transfer the data from host order to net order
*/
void host_to_net(struct dataExchange* data, int command, int move, int game_number);

/*
transfer the data from net order to host order
*/
void net_to_host(struct dataExchange* data, int command, int move, int game_number);
int client(const char *ip, const char *port);
int server(const char* portStr);
#endif