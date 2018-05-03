//
// Created by 魏尧 on 2/1/18.
//

#include "tictactoe.h"


#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

int move(char board[ROWS][COLUMNS],int player,int choice){
    char mark = (player == 1) ? PLAYER1_MARK : PLAYER2_MARK;
    if(choice<1||choice>ROWS*COLUMNS)
        return -1;
    int row = (int)((choice-1) / ROWS);
    int column = (choice-1) % COLUMNS;
    if (board[row][column] == (choice+'0'))
    {
        board[row][column] = mark;
        printf("player %d moves to %d\n",player,choice);
        return 0;
    }else{
        return -1;
    }


}
int mockNextMove(char board[ROWS][COLUMNS]){
    int pos[9];
    int count=0;
    for(int i=0;i<ROWS;i++){
        for(int j=0;j<COLUMNS;j++){
            if(board[i][j]>='1'&&board[i][j]<='9'){
                pos[count++]=i*COLUMNS+j;
            }

        }
    }
    if(!count)
        return -1;

    return pos[rand()%count]+1;

}
int nextMove(char board[ROWS][COLUMNS],int player){
    char buff[21];
    memset(buff,0, sizeof(buff));
    printf("Player %d, enter a number:  ", player); // print out player so you can pass game
    scanf("%20s",buff);
    char* endptr;
    long int choice=strtol(buff,&endptr,10);
    if((*endptr!=0)||choice<1||choice>9){
        printf("Invalid move \n");
        //remove all line from buff
        while(getchar()!='\n');
        return -1;
    }
    //only accept the answer if it is a single chracter
    int row = (int)((choice-1) / ROWS);
    int column = (choice-1) % COLUMNS;
    if (board[row][column] == (choice+'0'))
        return choice;
    else
    {
        printf("Invalid move \n");
        getchar();
        return -1;
    }

}


void print_result(int player){
    switch (player) {
        case  1:
        case 2:
            printf("==>\aPlayer %d wins\n ", player);
        default:
            break;
        case 3:
            printf("==>\aGame draw\n"); // ran out of squares, it is a draw
    }

}


int checkwin(char board[ROWS][COLUMNS])
{
    /************************************************************************/
    /* brute force check to see if someone won, or if there is a draw       */
    /* return a 0 if the game is 'over' and return -1 if game should go on  */
    /************************************************************************/
    for(int i=0;i<3;i++){
        if(board[i][0]==board[i][1]&&board[i][1]==board[i][2])
            return board[i][0]==PLAYER1_MARK?1:2;
        if(board[0][i]==board[1][i]&&board[1][i]==board[2][i])
            return board[0][i]==PLAYER1_MARK?1:2;
    }

     if (board[0][0] == board[1][1] && board[1][1] == board[2][2] ) // diagonal
        return board[0][0]==PLAYER1_MARK?1:2;

    else if (board[2][0] == board[1][1] && board[1][1] == board[0][2] ) // diagonal
        return board[2][0]==PLAYER1_MARK?1:2;
    for(int i=0;i<ROWS;i++){
        for(int j=0;j<COLUMNS;j++){
            if(board[i][j]<='9'&&board[i][j]>='0')
                return 0;//not ended
        }
    }
    return 3;//all occupied,draw
//    else if (board[0][0] != '1' && board[0][1] != '2' && board[0][2] != '3' &&
//             board[1][0] != '4' && board[1][1] != '5' && board[1][2] != '6' &&
//             board[2][0] != '7' && board[2][1] != '8' && board[2][2] != '9')
//
//        return 3; // Return of 3 means draw
//    else
//        return  0; // return of 0 means keep playing
}


void print_board(char board[ROWS][COLUMNS])
{
    /*****************************************************************/
    /* brute force print out the board and all the squares/values    */
    /*****************************************************************/

    printf("\n\n\n\tCurrent TicTacToe Game\n\n");

    printf("Player 1 (X)  -  Player 2 (O)\n\n\n");


    printf("     |     |     \n");
    printf("  %c  |  %c  |  %c \n", board[0][0], board[0][1], board[0][2]);

    printf("_____|_____|_____\n");
    printf("     |     |     \n");

    printf("  %c  |  %c  |  %c \n", board[1][0], board[1][1], board[1][2]);

    printf("_____|_____|_____\n");
    printf("     |     |     \n");

    printf("  %c  |  %c  |  %c \n", board[2][0], board[2][1], board[2][2]);

    printf("     |     |     \n\n");
}



int initSharedState(char board[ROWS][COLUMNS]){
    srand(time(NULL));
    /* this just initializing the shared state aka the board */
    int i, j, count = 1;
//    printf ("in sharedstate area\n");
    for (i=0;i<3;i++)
        for (j=0;j<3;j++){
            board[i][j] = count + '0';
            count++;
//            board[i][j]=EMPTY;
        }


    return 0;

}


