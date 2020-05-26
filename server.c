#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <unistd.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdint.h>

#include <pthread.h>

#define PORT 5005
#define OFFSET_PORT 5010

typedef struct Thread_Arg{
    char p[20];
    int sock;
}Thread_Arg;

void *T_func(void *arg_){

    Thread_Arg *arg = (Thread_Arg *)arg_;

    int error;

    int n = 0;

    struct sockaddr_in client_addr;

    struct timeval T2, T3;

    int32_t T[4], binary = 0x00;

    int len = sizeof(client_addr);

    memset((int32_t *)&T, '\0', sizeof(T));

    while(1){

        if(recvfrom(arg->sock, &binary, sizeof(binary) + 1, 0, (struct sockaddr*)&client_addr, &len) > 0){
            error = ioctl(arg->sock, SIOCGSTAMP, &T2);
            n++;

            //usleep(300000); // medium term 을 줘버리면 client_addr가 sleep 동안 수정되어버리는 문제 발생 client는 while문 안에서 더 많은 수신을 하게 됨

            gettimeofday(&T3, 0);

            T[0] = T2.tv_sec; T[1] = T2.tv_usec; T[2] = T3.tv_sec; T[3] = T3.tv_usec;

            sendto(arg->sock, T, sizeof(T) + 1, 0, (struct sockaddr*)&client_addr, sizeof(client_addr));

            printf("%s : %d\nT2: %d.%d\nT3: %d.%d\n\n", arg->p, n, T2.tv_sec, T2.tv_usec, T3.tv_sec, T3.tv_usec);
        }

    }
}

int main(int argc, char *argv[]){

    pthread_t p_thread[2];
    int status;

    Thread_Arg *synchronization, *offset;

    synchronization = (Thread_Arg *)malloc(sizeof(Thread_Arg));
    offset = (Thread_Arg *)malloc(sizeof(Thread_Arg));

    struct sockaddr_in server_addr, offset_server_addr;

    struct timeval T2, T3;

    int offset_len = sizeof(offset_len);

    strcpy(synchronization->p, "synchronization");
    strcpy(offset->p, "offset");

    if((synchronization->sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        printf("socket call error!\n");
        exit(1);
    }

    if((offset->sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        printf("offset_socket call error!\n");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if(argc == 2)
        server_addr.sin_port = htons(atoi(argv[1]));

    offset_server_addr.sin_family = AF_INET;
    offset_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    offset_server_addr.sin_port = htons(OFFSET_PORT);

    if(bind(synchronization->sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        printf("bind error1!\n");
        exit(1);
    }

    if(bind(offset->sock, (struct sockaddr*)&offset_server_addr, sizeof(offset_server_addr)) == -1){
        printf("bind error2!\n");
        exit(1);
    }

    if((pthread_create(&p_thread[0], NULL, T_func, (void *)synchronization)) < 0){
        printf("thread create error1!");
        exit(1);
    }

    sleep(1);

    if((pthread_create(&p_thread[1], NULL, T_func, (void *)offset)) < 0){
        printf("thread create error2!");
        exit(1);
    }

    pthread_join(p_thread[0], (void **)&status);
    pthread_join(p_thread[1], (void **)&status);

    close(synchronization->sock);
    close(offset->sock);
}