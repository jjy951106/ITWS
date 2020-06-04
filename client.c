/* Single Thread */

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

#define ITERATION 5
#define SPLEEP_TIME 0 // nanosecond

#define BOUNDARY 200000 // plus
#define BOUNDARY_ -200000 // minus

int main(int argc, char *argv[]){

    int sock, iter;

    struct sockaddr_in server_addr;

    struct timespec T1, T2, T3, T4, C; // C (current)

    struct timespec s;

    int32_t tmp, offset[2], T[4], binary = 0x00;

    memset((int32_t *)&T, '\0', sizeof(T));

    s.tv_sec = 0; s.tv_nsec = SPLEEP_TIME;

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

    if(connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        printf("connect() failed\n");
        exit(1);
    }

    while(1){

        offset[0] = 0;
        offset[1] = 0;

        for(iter = 0; iter < ITERATION; iter++){

            clock_gettime(CLOCK_REALTIME, &T1);

            if(send(sock, &binary, sizeof(binary), 0) == -1){
                printf("%d : send() failed\n", iter);
                exit(1);
            }

            if(recv(sock, T, sizeof(T), 0) == -1){ // code running stop until recv
                printf("%d : recv() failed\n", iter);
                exit(1);
            }

            clock_gettime(CLOCK_REALTIME, &T4);

            T2.tv_sec = T[0]; T2.tv_nsec = T[1]; T3.tv_sec = T[2]; T3.tv_nsec = T[3];

            offset[0] += ((T2.tv_sec - T1.tv_sec) - (T4.tv_sec - T3.tv_sec));

            offset[1] += ((T2.tv_nsec - T1.tv_nsec) - (T4.tv_nsec - T3.tv_nsec));

            nanosleep(&s, NULL);
        }

        offset[0] /= (2 * ITERATION);
        offset[1] /= (2 * ITERATION);

        printf("offset : %d.%d\n", offset[0], offset[1]);

        if(abs(offset[0]) > 0){
            clock_settime(CLOCK_REALTIME, &T3);
            sleep(2);
            continue;
        }

        else if(offset[1] < BOUNDARY && offset[1] > BOUNDARY_){
            printf("%d ns\n", offset[1]);
            break;
        }

        clock_gettime(CLOCK_REALTIME, &C);

        if((tmp = C.tv_nsec + offset[1]) > 1000000000){
            C.tv_sec += 1;
            C.tv_nsec = tmp - 1000000000;
        }

        else
            C.tv_nsec = tmp;

        clock_settime(CLOCK_REALTIME, &C);

    }

    close(sock);

}