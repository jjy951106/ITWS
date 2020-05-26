#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h> // sleep 함수 사용 window 에서는 windows.h

#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdint.h>

#define SERVER "192.168.0.158" // SERVER 주소 변경 필요 시 변경
#define PORT 5005
#define ITERATION 5
#define SPLEEP_TIME 100000 // us
#define LIMIT 500
#define BOUNDARY 400 // 양수
#define BOUNDARY_ -100 // 음수

int offset = 0;
int delay = 0;

int main(int argc, char *argv[]){

    int sock, iter, error; // offset과 delay를 double형으로 가져가야 하나?

    struct sockaddr_in server_addr, client_addr, temp_addr; // temp_addr 은 server의 주소를 비교하기 위함 어떤 주소든 받기 때문에

    struct timeval T1, T2, T3, T4, current;

    int32_t T[4];

    int32_t binary = {
        0x00
    };

    int len = sizeof(temp_addr);

    int temp;

    double interval = 0; // 동기화 간격

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
        printf("You need to input SERVER, PORT in order!\n");
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

    memset((int32_t *)&T, '\0', sizeof(T));

    while(1){

        for(iter = 0; iter < ITERATION; iter++){

            gettimeofday(&T1, 0);

            if(sendto(sock, &binary, sizeof(binary) + 1, 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
                printf("sendto error!\n");
                exit(1);
            }

            while(1){ // 신호를 구분해서 받아들일 필요가 있음

                if(recvfrom(sock, T, sizeof(T) + 1, 0, (struct sockaddr*)&temp_addr, &len) > 0){

                    error = ioctl(sock, SIOCGSTAMP, &T4); // 가장 마지막에 들어온 udp packet을 까보는 것..? 과연 정확한가? 그 사이에 다른 패킷이 도착할 수 있다는 가정이 듬

                    if(temp_addr.sin_addr.s_addr == server_addr.sin_addr.s_addr){
                        T2.tv_sec = T[0]; T2.tv_usec = T[1]; T3.tv_sec = T[2]; T3.tv_usec = T[3];
                        break;
                    }

                }
                //else break; 수신 그대로 중단되버리는 경우가 있음

            }

            offset += (((T2.tv_sec - T1.tv_sec) * 1000000 + T2.tv_usec - T1.tv_usec) -
                      ((T4.tv_sec - T3.tv_sec) * 1000000 + T4.tv_usec - T3.tv_usec));

            delay += (((T2.tv_sec - T1.tv_sec) * 1000000 + T2.tv_usec - T1.tv_usec) +
                      ((T4.tv_sec - T3.tv_sec) * 1000000 + T4.tv_usec - T3.tv_usec));

            usleep(SPLEEP_TIME);
        }

        offset /= (2 * ITERATION);

        delay /= 2;

        printf("offset : %d\n", offset);

        interval += SPLEEP_TIME * ITERATION + delay;

        if(abs(offset) > 1000000){
            settimeofday(&T3, 0);
            sleep(2);
            continue;
        }

        else if(offset < BOUNDARY && offset > BOUNDARY_){
            printf("%dus %.3f", offset, interval / 1000000);
            break;
        }

        gettimeofday(&current, 0);

        if((temp = current.tv_usec + offset) > 1000000){
            current.tv_sec += 1;
            current.tv_usec = temp - 1000000;
        }

        else
            current.tv_usec = temp;

        settimeofday(&current, 0); // 관리자 권한 필요

    }

    close(sock);
}