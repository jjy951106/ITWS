#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h> // sleep 함수 사용 window 에서는 windows.h

#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER "192.168.0.158" // SERVER 주소 변경 필요 시 변경
#define PORT 5005
#define ITERATION 5
#define SPLEEP_TIME 0.5
#define LIMIT 500
#define BOUNDARY 100
#define OFFSET_INTERVAL 2


int main(int argc, char *argv[]){

    int sock, iter, offset, delay, error; // offset과 delay를 double형으로 가져가야 하나?

    struct sockaddr_in server_addr, client_addr, temp_addr;

    struct timeval T1, T2, T3, T4, current;

    int len = sizeof(temp_addr);

    int temp;

    int interval = 0; // 동기화 간격

    char buf[256];

    int sync = 1; // 1 on, 0 off

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

    offset = 0;
    //delay = 0;

    while(1){

        for(iter = 0; iter < ITERATION; iter++){

            gettimeofday(&T1, 0);

            sprintf(buf, "%d %d", T1.tv_sec, T1.tv_usec);

            if(sendto(sock, buf, strlen(buf) + 1, 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
                printf("sendto error!\n");
                exit(1);
            }

            while(1){ // 신호를 구분해서 받아들일 필요가 있음

                if(recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr*)&temp_addr, &len) > 0){

                    error = ioctl(sock, SIOCGSTAMP, &T4); // 가장 마지막에 들어온 udp packet을 까보는 것..? 과연 정확한가? 그 사이에 다른 패킷이 도착할 수 있다는 가정이 듬

                    if(temp_addr.sin_addr.s_addr == server_addr.sin_addr.s_addr){
                        T2.tv_sec = atoi(strtok(buf, " "));
                        T2.tv_usec = atoi(strtok(NULL, " "));
                        T3.tv_sec = atoi(strtok(NULL, " "));
                        T3.tv_usec = atoi(strtok(NULL, " "));
                        break;
                    }

                };

            }

            //offset 계산
            offset += (((T2.tv_sec - T1.tv_sec) * 1000000 + T2.tv_usec - T1.tv_usec) -
                      ((T4.tv_sec - T3.tv_sec) * 1000000 + T4.tv_usec - T3.tv_usec));

            //delay 계산
            //delay += (((T2.tv_sec - T1.tv_sec) * 1000000 + T2.tv_usec - T1.tv_usec) +
            //         ((T4.tv_sec - T3.tv_sec) * 1000000 + T4.tv_usec - T3.tv_usec));
            sleep(SPLEEP_TIME);
        }

        offset /= (2 * ITERATION);

        if(sync == 1){

            if(abs(offset) > 1000000){
                settimeofday(&T3, 0);
                continue;
            }

            else if(offset < BOUNDARY && offset > 0) sync = 0;

            //delay /= (2 * ITERATION);

            gettimeofday(&current, 0);

            if((temp = current.tv_usec + offset) > 1000000){
                current.tv_sec += 1;
                current.tv_usec = temp - 1000000;
            }

            else
                current.tv_usec = temp;

            settimeofday(&current, 0); // 관리자 권한 필요
        }
        else{

            printf("%dus %d\n", offset, interval);

            interval += OFFSET_INTERVAL;

            if(abs(offset) > LIMIT) {
                sync = 1;
                interval = 0; // interval 초기화
                printf("\n");
            }

            sleep(OFFSET_INTERVAL);
        }

    }

    close(sock);
}