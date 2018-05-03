//
// Created by 魏尧 on 2/16/18.
//
#include <stdio.h>
#include <stdlib.h>

int client(const char *ip, const char *port);
int server(const char* portStr);
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("usage: ./tictactoe <port> <play number: 1 or 2> <ip address>\n");
        return -1;
    }
    char *port = argv[1];
    long playerId = strtol(argv[2], 0, 10);
    if (playerId != 1 && playerId != 2) {
        perror("playId must be 1 or 2\n");
        return -1;
    }
    if (argc == 3) {
        if (playerId == 1) {
            server(port);
        }
        else {
            printf("usage: ./tictactoe <port> <player> <ip-address>\n");
            return -1;
        }
    }
    else if (argc == 4) {
        if (playerId == 2) {
            char* ip = argv[3];
            client(ip, port);
        }
        else {
            printf("usage: ./tictactoe <port> <palyer>\n");
            return -1;
        }
    }
    return 0;
}
