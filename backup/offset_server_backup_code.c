#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5005

int main(int argc, char *argv[]){

    int sock, offset, error;

    struct sockaddr_in server_addr, client_addr;

    struct timeval T2, T3;

    int len = sizeof(client_addr);

    char buf[256];

    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        printf("socket call error!\n");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if(argc == 2)
        server_addr.sin_port = htons(atoi(argv[1]));

    if(bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        printf("bind error!\n");
        exit(1);
    }

    while(1){ //gettimeofday가 더 정확할 수도 있는 점을 참고
        if(recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr*)&client_addr, &len) > 0){
            error = ioctl(sock, SIOCGSTAMP, &T2);
        }

        gettimeofday(&T3, 0);
        sprintf(buf ,"%d %d %d %d", T2.tv_sec, T2.tv_usec, T3.tv_sec, T3.tv_usec);
        sendto(sock, buf, strlen(buf) + 1, 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
    }

    close(sock);
}