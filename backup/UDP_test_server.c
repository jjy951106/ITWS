#include <stdio.h>
#include <stdlib.h> // exit(0) 'normal' exit(1) 'error'
#include <string.h>
#include <stdint.h> // int32_t int64_t

#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <pthread.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include "linux/errqueue.h"

#define PORT 5005

int main(int argc, char *argv[]){

    int sock, offset, error;

    int n = 0;

    struct sockaddr_in server_addr, client_addr;

    int len = sizeof(client_addr);

    char buf[256];

    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        printf("socket call error!\n");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if(bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        printf("bind error!\n");
        exit(1);
    }

    while(recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr*)&client_addr, &len) > 0){

        printf("%d : %s\n", n++, buf);

        print("%d\n", atoi(buf))

        memset(buf, '\0', sizeof(buf));

    }

    close(sock);
}