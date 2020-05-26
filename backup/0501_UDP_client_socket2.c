#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>

#include <netdb.h>        //
#include <asm/sockios.h>  //

#include <sys/types.h>
#include <sys/socket.h> // socket 함수를 사용하기 위한 관련 정보가 포함된 파일 unix linux
                        // 기준으로 window 에서는 winsock, winsock2 헤더파일로 존재

#include <netinet/in.h>
#include <arpa/inet.h>  // inet_addr(), inet_ntoa() 함수를 쓰기 위함  32bit의 ip adress를 10진수 <-> 2진수로 변환 arpa/inet.h 에서 inet_addr 사용

#define SERVER "210.107.214.170"
#define PORT 5005

int main(){

    int client_sock, ret, error;
    struct sockaddr_in server_addr, client_addr;
    char buf[1024], temp[20];
    char *message = "Hellow me too!";

    struct timeval tv_server, tv_ioctl;

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER); // htonl 생략
    server_addr.sin_port = htons(PORT); // htons 주의

    if((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){ // 실패 시 -1 의 값을 디스크립터로 반환
        printf("socket call error!\n");
        exit(1);                                              // error 마다 호출할 함수를 만드는 것도 고려
    }

    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY); // htonl 생략
    client_addr.sin_port = htons(0); // htons 주의

    if(bind(client_sock, (struct sockaddr*)&client_addr, sizeof(client_addr)))
        exit(1);

    sendto(client_sock, message, strlen(message) + 1, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

    recvfrom(client_sock, buf, sizeof(buf), 0, (struct sockaddr*)0, (int*)0);

    error = ioctl(client_sock, SIOCGSTAMP, &tv_ioctl);

    tv_server.tv_sec = atoi(strtok(buf, "."));

    tv_server.tv_usec = atoi(strtok(NULL, ""));

    printf ("servertime: %d.%d\t ioctl: %d.%d\t delta: %d.%d\noffset: %d\n",
        tv_server.tv_sec, tv_server.tv_usec,
        tv_ioctl.tv_sec, tv_ioctl.tv_usec,
        tv_ioctl.tv_sec - tv_server.tv_sec,
        tv_ioctl.tv_usec - tv_server.tv_usec,
        (tv_ioctl.tv_sec - tv_server.tv_sec) * 1000000 + tv_ioctl.tv_usec - tv_server.tv_usec
    );

    close(client_sock);

    return 0;
}