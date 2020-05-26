#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5005

int main(){

    int server_sock, error;
    struct sockaddr_in serveraddr, clientaddr;
    struct timeval tv_server, tv_client;
    int client_len = sizeof(clientaddr);

    char buf[1024];
    char time_temp[50];

    if((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        printf("socket call error!\n");
        exit(1);
    }

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(PORT);

    if(bind(server_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1){
        printf("bind error!\n");
        exit(1);
    }

    while(1){
        if(recvfrom(server_sock, buf, sizeof(buf), 0, (struct sockaddr*)&clientaddr, &client_len) > 0){
            //gettimeofday(&tv_server, NULL);                                                                           // 이것이 더 정확할 수도 있다는 걸 참고
            error = ioctl(server_sock, SIOCGSTAMP, &tv_server);
            printf("Client_Send_Time: %s\nSocket_Receive_Time: %d.%d\n\n", buf, tv_server.tv_sec, tv_server.tv_usec);
        }

        gettimeofday(&tv_server, NULL);
        sprintf(time_temp ,"%d.%d", tv_server.tv_sec, tv_server.tv_usec);
        sendto(server_sock, time_temp, strlen(time_temp) + 1, 0, (struct sockaddr*)&clientaddr, sizeof(clientaddr));
    }

    close(server_sock);

    return 0;
}