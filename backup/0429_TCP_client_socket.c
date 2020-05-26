/* linux */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h> // socket 함수를 사용하기 위한 관련 정보가 포함된 파일 unix linux
                        // 기준으로 window 에서는 winsock, winsock2 헤더파일로 존재

#include <netinet/in.h>
#include <arpa/inet.h>  // inet_addr(), inet_ntoa() 함수를 쓰기 위함  32bit의 ip adress를 10진수 <-> 2진수로 변환

#define IP "210.107.214.170"
#define PORT 5005

int main(){

    int client_sock;
    struct sockaddr_in server_addr;
    char buf[1024];
    char *message = "Hellow me too!";

    if((client_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){ // 실패 시 -1 의 값을 디스크립터로 반환
        printf("socket call error!\n");
        exit(1);                                        // error 마다 호출할 함수를 만드는 것도 고려
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(IP); // htonl 생략
    server_addr.sin_port = htons(PORT); // htons 주의

    printf("1, server_addr.sin_port : %d\n",server_addr.sin_port);

    if(connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)))
        exit(1);

    printf("2, server_addr.sin_port : %d\n",server_addr.sin_port);

    if(recv(client_sock, buf, sizeof(buf), 0) == -1)
        exit(1);

    if(send(client_sock, message, strlen(message) + 1, 0) == -1)
        exit(1);

    printf("Server : %s\n", buf);

    close(client_sock);

    return 0;
}