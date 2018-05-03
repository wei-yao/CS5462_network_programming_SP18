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
#define TIME_OUT 10
#define  MC_PORT 1818
#define MC_GROUP "239.0.0.1"
#define  BACK_UP_FILE "back_up_ip.txt"
#define BLACK_LIST_LEN 1
//define DEBUG enables debug with the local fake server
//#define  DEBUG

int client(const char *ip, const char *port);
int main(int argc, char *argv[]){
    if (argc < 3) {
        printf("usage: ./tictactoeClient <port> <ip> \n");
        return -1;
    }
//    printf(" %d\n",  sizeof(struct dataExchange));
    return client(argv[2],argv[1]);
}

int client(const char *ip, const char *port) {
//
    char board[ROWS][COLUMNS];
    int sd = 0;
    int multicast_sd;
//    int serv_len;
//    int last_recv = -1;
    struct dataExchange send_pkg,recv_pgk;
    struct sockaddr_in serv_addr,multicast_addr;
//    struct ip_mreq mreq;
    struct timeval timeout = {TIME_OUT,0};
//    struct dataExchange last_send;
//    struct dataExchange last_rcv;
//    int game_no=-1;
    // open the socket
    if((sd = Socket(AF_INET, SOCK_STREAM, 0))<0){
        perror("open socket error");
        exit(1);
    }
    if((multicast_sd= socket(AF_INET, SOCK_DGRAM, 0))<0){
        perror("open socket error");
        exit(1);
    }
//    // set the timeout
//    if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0){
//        printf("timeout, errno= %d\n",errno);
//        exit(1);
//    }
    if (setsockopt(multicast_sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0){
        printf("timeout, errno= %d\n",errno);
        exit(1);
    }
    struct dataExchange end_package;
    int disconnected=1;
    int is_initial_connection=1;
    end_package.command=htonl(ENDGAME);
    end_package.move=htonl(-1);

    memset(&multicast_addr,0, sizeof(multicast_addr));
    multicast_addr.sin_family=AF_INET;
    multicast_addr.sin_port=htons(MC_PORT);
    multicast_addr.sin_addr.s_addr=inet_addr(MC_GROUP);
//    in_addr_t origin=inet_addr(MC_GROUP);
    int addr_len= sizeof(multicast_addr);
//    serv_len = sizeof(serv_addr);
    memset(serv_addr.sin_zero, 0, sizeof serv_addr.sin_zero);
    initSharedState(board);
    print_board(board);
    int target;
    int state;
    send_pkg.command = htonl(NEWGAME);
    send_pkg.move = htonl(-1);
    send_pkg.game_number = htonl(-1);
    initSharedState(send_pkg.board);
    int init_end_shake=0;
    int ret=0;
    int command,recv_move,game_number;
    char server_req='1';
    char new_ip[INET_ADDRSTRLEN];
    char black_list[BLACK_LIST_LEN][20]={""};
 while(1){
     if(disconnected){

         //not first connection, needs to multicast to find the server ip and port;
         printf("disconneted, try to reconnect\n");


         if(!is_initial_connection){
             //have to close the socket and create new socket every time, otherwise the reconnection will fail
             close(sd);
             if((sd = Socket(AF_INET, SOCK_STREAM, 0))<0){
                 perror("open socket error");
                 exit(1);
             }
             sendto(multicast_sd,&server_req,1,0,(
             struct sockaddr*)&multicast_addr, sizeof(multicast_addr));
            int server_port;
             in_addr_t recv_server_addr;

             if(Recvfrom(multicast_sd,&server_port, 4,0,&multicast_addr, &addr_len)==4){

                 server_port=ntohl(server_port);
                 recv_server_addr=multicast_addr.sin_addr.s_addr;

                 memset(new_ip,0, sizeof(new_ip));
                 inet_ntop(AF_INET,&(multicast_addr.sin_addr),new_ip, sizeof(new_ip));
                 int skip=0;
                 for(int i=0;i<BLACK_LIST_LEN;i++){
                     if(!strcmp(black_list[i],new_ip)){
                         skip=1;
                         break;
                     }
                 }
                 if(skip)
                 {
                     sleep(5);
                     continue;
                 }

                 printf("receive new ip %s，port %d\n",new_ip, server_port);


             }else{
                 //time out for receiving the port，todo： read addr from file
                 FILE* file=fopen(BACK_UP_FILE,"r");
                 if(file){
                     char ip_buf[20];
                     memset(ip_buf,0,20);
                     fscanf(file,"%s %d",ip_buf,&server_port);
                     recv_server_addr=inet_addr(ip_buf);
                     fclose(file);
                 }else{
                     //read file error
                     continue;
                 }


             }
             memset(&serv_addr, 0, sizeof(serv_addr));
             serv_addr.sin_family = AF_INET;
             serv_addr.sin_port = htons(server_port);
             //todo multicast_addr will be changed after recv, do I need to reset it every time I send
             serv_addr.sin_addr.s_addr = recv_server_addr;
             memset(serv_addr.sin_zero, 0, sizeof serv_addr.sin_zero);

         }else{

             is_initial_connection=0;
             memset(&serv_addr, 0, sizeof(serv_addr));
             serv_addr.sin_family = AF_INET;
             serv_addr.sin_port = htons(atoi(port));
             serv_addr.sin_addr.s_addr = inet_addr(ip);



         }

         memset(new_ip,0, sizeof(new_ip));
         inet_ntop(AF_INET,&(serv_addr.sin_addr),new_ip, sizeof(new_ip));
        printf("connect to new ip %s %d",new_ip, ntohs(serv_addr.sin_port));
         if(connect(sd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))==-1){
             sleep(2);
             printf("server unavailable \n");
             continue;
         }
         disconnected=0;
         if(ntohl(send_pkg.command)!=NEWGAME){
             send_pkg.command=htonl(CONTINUE);
             send_pkg.game_number=htonl(-1);
             memcpy(send_pkg.board,board,ROWS*COLUMNS);
         }
         if(Send(sd, (void *) &send_pkg, sizeof(send_pkg), 0)<=0){
             disconnected=1;
             continue;
         }
     }else{
         if(Recv(sd, (void *) &recv_pgk, sizeof(recv_pgk), 0)<=0){
             printf("connection closed\n");
             disconnected=1;
             continue;
         }
         command=ntohl(recv_pgk.command);
         recv_move=ntohl(recv_pgk.move);
         game_number=ntohl(recv_pgk.game_number);
         //this include new game return -1 or continue return -1;
         if(game_number==-1){
             printf("There are no more game slots available\n");
             disconnected=1;
             continue;
         }
         if (command == NEWGAME) {
             printf("gameno %d\n",game_number);
//                game_no=send_pkg.game_number;

//                printf("begin new game no %d\n", ntohl(send_pkg.game_number));
                 if (ROBOT_ON) {
                     target = mockNextMove(board);
                 } else {
                     while ((target = nextMove(board, PLAYER_ID)) == -1);
                 }
                 move(board, PLAYER_ID, target);
                 print_board(board);
                 printf("game number %d\n",game_number);
                 send_pkg.move = htonl(target);
                 send_pkg.command = htonl(MOVE);
                 send_pkg.game_number = htonl(game_number);
                 //handle exception here
                 if(Send(sd, (void *) &send_pkg, sizeof(send_pkg), 0)<=0){
                     disconnected=1;
                     continue;
                 }
//                last_send=send_pkg;
         }else if(command == MOVE||command==CONTINUE) {
             //todo reject invalid move
             move(board, OP_PLAYER_ID, recv_move);
             print_board(board);

             //game is over, clinets lose
             if ((checkwin(board)) != 0) {
                 end_package.game_number=htonl(game_number);
                 if(Send(sd,(void*)&end_package,sizeof(end_package),0)<=0){
                     disconnected=1;
                     continue;
                 }
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
                 send_pkg.game_number=htonl(game_number);
                 send_pkg.command=htonl(MOVE);

                 send_pkg.move = htonl(target);
                 if(Send(sd, (void *) &send_pkg, sizeof(send_pkg), 0)<=0){
                     disconnected=1;
                     continue;
                 }
                 print_board(board);
             }
         }else if(command==ENDGAME){
             //check if I initiate the end handshake
             //check game No field?
             if((state = checkwin(board))){
                 print_board(board);
                 print_result(state);
                 if(!init_end_shake){
                     end_package.game_number=htonl(game_number);
                     if(Send(sd,(void*)&end_package,sizeof(end_package),0)<=0){
                         disconnected=1;
                         continue;
                     }
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



     }

 }


    



//    last_send=send_pkg;
    



//    print_board(board);
//    print_result(state);
    close(sd);
    return ret;
}