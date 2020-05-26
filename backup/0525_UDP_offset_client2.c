#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER "192.168.0.158" // SERVER 주소 변경 필요 시 변경
#define PORT 5005

int main(int argc, char *argv[]){

    int sock, iter, offset, delay, error; // offset과 delay를 double형으로 가져가야 하나?

    struct sockaddr_in server_addr, client_addr, temp_addr;

    struct timeval T1, T2, T3, T4, current;

    int32_t T[4];

    int32_t binary = {
        0x00
    };

    int len = sizeof(temp_addr);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER);
    server_addr.sin_port = htons(PORT);

    if(argc == 2){
        server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    }

    else if(argc == 3){
        server_addr.sin_addr.s_addr = inet_addr(argv[1]);
        server_addr.sin_port = htons(atoi(argv[2]));
    }

    else if(argc != 1){
        printf("SERVER, PORT 순으로 입력하세요\n");
        return 0;
    }

    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        printf("socket call error!\n");
        exit(1);
    }

    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    client_addr.sin_port = htons(0);

    if(bind(sock, (struct sockaddr*)&client_addr, sizeof(client_addr)) == -1){
        printf("bind error!\n");
        exit(1);
    }

    offset = 0;

    gettimeofday(&T1, 0);

    if(sendto(sock, &binary, sizeof(binary) + 1, 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        printf("sendto error!\n");
        exit(1);
    }

    while(1){ // Server로 부터 시간 정보를 받을 때 까지 수신
              // 연결을 맺은 것이 아니여서 자신에게 들어오는 udp 신호를 다 처리하기 때문에
              // 신호를 구분해서 받아들일 필요가 있음
        if(recvfrom(sock, T, sizeof(T), 0, (struct sockaddr*)&temp_addr, &len) > 0){
            error = ioctl(sock, SIOCGSTAMP, &T4); // 가장 마지막에 들어온 udp packet을 까보는 것..? 과연 정확한가? 그 사이에 다른 패킷이 도착할 수 있다는 가정이 듬

            if(temp_addr.sin_addr.s_addr == server_addr.sin_addr.s_addr){
                T2.tv_sec = T[0]; T2.tv_usec = T[1]; T3.tv_sec = T[2]; T3.tv_usec = T[3];
                break;
            }
        }
    }

    //offset 계산
    offset += (((T2.tv_sec - T1.tv_sec) * 1000000 + T2.tv_usec - T1.tv_usec) -
              ((T4.tv_sec - T3.tv_sec) * 1000000 + T4.tv_usec - T3.tv_usec)) / 2;


    printf("%d", offset / 3);

    close(sock);
}