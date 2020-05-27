/* Multi_Thread */

#include <stdio.h>
#include <stdlib.h> // exit(0) 'normal' exit(1) 'error'
#include <string.h>
#include <stdint.h>

#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <pthread.h>
#include <unistd.h>  // int32_t
#include <netinet/in.h>
#include <arpa/inet.h>

#define BACKLOG 5 // total client number
#define MEDIUM_TERM 0 // nanosecond

#define PORT 5005

int Thread_t = 0; // total thread number played

void *Server_Socket_Thread(void *arg){

    int sock = (int *)arg;

    int n = 0;

    struct timespec T2, T3;

    struct timespec s;

    int32_t T[4], binary;

    memset((int32_t *)T, '\0', sizeof(T));

    s.tv_sec = 0; s.tv_nsec = MEDIUM_TERM;

    while(recv(sock, &binary, sizeof(binary), 0) != -1){

        clock_gettime(CLOCK_REALTIME, &T2);

        T[0] = T2.tv_sec; T[1] = T2.tv_nsec;

        nanosleep(&s, NULL);

        clock_gettime(CLOCK_REALTIME, &T3);

        T[2] = T3.tv_sec; T[3] = T3.tv_nsec;

        send(sock, T, sizeof(T) + 1, 0);

        printf("%d\nT2: %d.%d\nT3: %d.%d\n\n", ++n, T2.tv_sec, T2.tv_nsec, T3.tv_sec, T3.tv_nsec);

    }

    close(sock);
}

int main(int argc, char *argv[]){

    int sock, new, client_len;

    struct sockaddr_in server_addr, client_addr;

    pthread_t p_thread[BACKLOG];

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("socket() failed\n");
        exit(1);
    }

    memset(&server_addr, '\0', sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if(bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        printf("bind() failed\n");
        exit(1);
    }

    if(listen(sock, BACKLOG) == -1){
        printf("listen() failed\n");
        exit(1);
    }

    while((new = accept(sock, (struct sockaddr*)&client_addr, &client_len)) != -1)
        if(pthread_create(&p_thread[Thread_t], NULL, Server_Socket_Thread, (void *)new) == 0) Thread_t++; // thread success return 0

    close(sock);

    return 0;
}