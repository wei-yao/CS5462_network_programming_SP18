#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#define BUFF_SIZE 1024
#define HEAD_LEN 24
#define DOMAIN AF_INET
unsigned int fsize(FILE* fp) {
    fseek(fp, 0L, SEEK_END);
    size_t sz = ftell(fp);
    rewind(fp);
    return (unsigned int)sz;
}
int main(int argc, char const *argv[])
{
    if(argc<4){
        perror("too few arguments\n");
        printf("usage: ftpc <remote-IP> <remote-port> <local-file-to-transfer>\n");
        exit(EXIT_FAILURE);
    }
    const char* ip=argv[1];
    const char* port=argv[2];
    const char* fileName=argv[3];
    char head[HEAD_LEN];
    memset(head,' ', sizeof(head));
    memcpy(head, fileName, strlen(fileName));
    int sock = 0;
    struct sockaddr_in serv_addr;
    char* buffer=malloc(BUFF_SIZE);
    if(!buffer){
        fprintf(stderr,"malloc %d bytes memory fails\n",BUFF_SIZE);
        return -1;
    }
    if ((sock = socket(DOMAIN, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    FILE* infile=fopen(fileName,"r");
    if(!infile){
        fprintf(stderr, "can not open file %s\n", fileName);
        return -1;
    }
    unsigned int fSize=fsize(infile);
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = DOMAIN;
    serv_addr.sin_port = htons(atoi(port));
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    memset(serv_addr.sin_zero, '\0', sizeof serv_addr.sin_zero);
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    char pad=0;
    memset(head,pad,sizeof(head));
    unsigned int tmpF=htonl(fSize);
    memcpy(head,&tmpF,sizeof(fSize));
    memcpy(head+sizeof(fSize),fileName,strlen(fileName));
    write(sock , head , sizeof(head));
    size_t readSize=0;
    while((readSize=fread(buffer,1,BUFF_SIZE,infile))){
        write(sock , buffer , readSize );
    }
    unsigned int receiveFileSize;
    read(sock , &receiveFileSize, 4);

    receiveFileSize=ntohl(receiveFileSize);
    if(receiveFileSize!=fSize){
        fprintf(stderr, "server only received %u bytes of %u bytes\n", receiveFileSize,fSize);
    }
    else{
        printf("server received all the %u bytes files\n",receiveFileSize);
    }
    if(buffer)
        free(buffer);
    fclose(infile);
    close(sock);
    return 0;
}
