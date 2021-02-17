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

    int n = 0, temp[2], i;

    float Compenstate_FC_MC;

    struct sockaddr_in server_addr, client_addr;

    int len = sizeof(client_addr);

    char buf[256];

    int *fc_comps_buf = (int *)malloc(sizeof(int) * 10);

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

        printf("%d\n", atoi(buf));

        fc_comps_buf[n] = atoi(buf);

        n++;

        memset(buf, '\0', sizeof(buf));

        if(n > 10){

            for(i = 1; i < 10; i++){
                temp[0] = fc_comps_buf[0]; // max
                temp[1] = fc_comps_buf[0]; // min

                if(fc_comps_buf[i] > temp[0])
                    temp[0] = fc_comps_buf[i];

                if(fc_comps_buf[i] < temp[1])
                    temp[1] = fc_comps_buf[i];
            }

            Compenstate_FC_MC = (temp[0] + temp[1]) / 2.0;

            printf("max : %d, min : %d\nCompenstate_FC_MC : %f\n", temp[0], temp[1], Compenstate_FC_MC);

            memset(fc_comps_buf, '\0', sizeof(fc_comps_buf));

            n = 0;

        }

    }

    free(fc_comps_buf);

    close(sock);
}