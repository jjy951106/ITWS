/* Multi_Thread */

#include <stdio.h>
#include <stdlib.h> // exit(0) 'normal' exit(1) 'error'
#include <string.h>
#include <stdint.h> // int32_t int64_t

#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <pthread.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BACKLOG 5 // total client number

#define PORT 5005


int main(int argc, char *argv[]){

    int sock, new, client_len;

    struct sockaddr_in server_addr, client_addr;

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("socket() failed\n");
        exit(1);
    }

    memset(&server_addr, '\0', sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if(argc == 2) server_addr.sin_port = htons(atoi(argv[1]));

    if(bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        printf("bind() failed\n");
        exit(1);
    }

    if(listen(sock, BACKLOG) == -1){
        printf("listen() failed\n");
        exit(1);
    }

    while((new = accept(sock, (struct sockaddr*)&client_addr, &client_len)) != -1){
        printf("Client IP : %s Port : %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }

    close(sock);

    return 0;
}