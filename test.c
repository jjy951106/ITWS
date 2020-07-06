#include <stdio.h>
#include <stdlib.h> // exit(0) 'normal' exit(1) 'error'
#include <string.h>
#include <stdint.h> // int32_t int64_t

#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <pthread.h>
#include <unistd.h>  // sleep usleep
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER "192.168.0.158"
#define PORT 5005

int main(int argc, char *argv[]){

    int sock;

    struct sockaddr_in server_addr;

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("socket() failed\n");
        exit(1);
    }

    memset(&server_addr, '\0', sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER);
    server_addr.sin_port = htons(PORT);

    if(argc >= 2) server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    if(argc == 3) server_addr.sin_port = htons(atoi(argv[2]));

    printf("addr : %s, port : %d\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

    if(connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        printf("connect() failed\n");
        exit(1);
    }

    close(sock);

	return 0;
}