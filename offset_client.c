#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER "192.168.0.158" // SERVER �ּ� ���� �ʿ� �� ����
#define PORT 5010
#define ITERATION 1 // offset�� �ѹ��� ����
#define SLEEP_TIME 100000

int main(int argc, char *argv[]){

    int sock, iter, offset, delay, error; // offset�� delay�� double������ �������� �ϳ�?

    struct sockaddr_in server_addr, client_addr, temp_addr;

    struct timeval T1, T2, T3, T4, current;

    clock_t start;

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
        printf("SERVER, PORT ������ �Է��ϼ���\n");
        return 0;
    }

    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        printf("socket call error!\n");
        exit(1);
    }

<<<<<<< Updated upstream
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

    start = clock();

    while(1){ // Server�� ���� �ð� ������ ���� �� ���� ����
              // ������ ���� ���� �ƴϿ��� �ڽſ��� ������ udp ��ȣ�� �� ó���ϱ� ������
              // ��ȣ�� �����ؼ� �޾Ƶ��� �ʿ䰡 ����

        if(recvfrom(sock, T, sizeof(T), 0, (struct sockaddr*)&temp_addr, &len) > 0){
            error = ioctl(sock, SIOCGSTAMP, &T4); // ���� �������� ���� udp packet�� ��� ��..? ���� ��Ȯ�Ѱ�? �� ���̿� �ٸ� ��Ŷ�� ������ �� �ִٴ� ������ ��

            if(temp_addr.sin_addr.s_addr == server_addr.sin_addr.s_addr){
                T2.tv_sec = T[0]; T2.tv_usec = T[1]; T3.tv_sec = T[2]; T3.tv_usec = T[3];
                break;
            }
        }
=======
    //while(1){

        clock_gettime(CLOCK_REALTIME, &T1);

        if(send(sock, &binary, sizeof(binary), 0) == -1){
            printf("send() failed\n");
            exit(1);
        }

        if(recv(sock, T, sizeof(T), 0) == -1){ // code running stop until recv
            printf("recv() failed\n");
            exit(1);
        }

        clock_gettime(CLOCK_REALTIME, &T4);

        T2.tv_sec = T[0]; T2.tv_nsec = T[1]; T3.tv_sec = T[2]; T3.tv_nsec = T[3];

        offset[0] = ((T2.tv_sec - T1.tv_sec) - (T4.tv_sec - T3.tv_sec)) / 2;

        offset[1] = ((T2.tv_nsec - T1.tv_nsec) - (T4.tv_nsec - T3.tv_nsec)) / 2;
>>>>>>> Stashed changes

        printf("%f\n", (double)(clock() - start)/1000000);

<<<<<<< Updated upstream
        if((double)(clock() - start)/1000000 > 0.5) exit(1); // 0.5�� �Ѿ�� ��ٸ��� ���� while�� �ȿ��� ���ѷ����� ���� ���� ����
    }

    //offset ���
    offset += (((T2.tv_sec - T1.tv_sec) * 1000000 + T2.tv_usec - T1.tv_usec) -
               ((T4.tv_sec - T3.tv_sec) * 1000000 + T4.tv_usec - T3.tv_usec)) / 2;
=======
        printf("T1: %d.%d\nT4: %d.%d\noffset : %d.%d\n", T1.tv_sec, T1.tv_nsec, T4.tv_sec, T4.tv_nsec, offset[0], offset[1]);

    //}
>>>>>>> Stashed changes

    //printf("%d ", offset);

<<<<<<< Updated upstream
    usleep(SLEEP_TIME);

    printf("%dus", abs(offset));

    //printf("%dus", offset);

    close(sock);
}
=======
}
>>>>>>> Stashed changes
