/* Multi_Thread */

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

#define BACKLOG 10 // total client number
#define MEDIUM_TERM_SEC 0
#define MEDIUM_TERM_NSEC 5000000 // nanosecond between receive and transmit

#define PORT 5005 // default port

int Thread_t = 0; // total thread number played

static void err(const char *error){
    printf("%s: %s\n", error, strerror(errno));
    exit(1);
}

void *Server_Socket_Thread(void *arg){

    /* recvpacket */
    char data[256];
    struct msghdr msg;
    struct iovec entry;
    struct sockaddr_in from_addr;
    struct {
        struct cmsghdr cm;
        char control[512];
    } control;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = &entry;
    msg.msg_iovlen = 1;
    entry.iov_base = data;
    entry.iov_len = sizeof(data);
    msg.msg_name = (caddr_t)&from_addr;
    msg.msg_namelen = sizeof(from_addr);
    msg.msg_control = &control;
    msg.msg_controllen = sizeof(control);

    /* printpacket */
    struct cmsghdr *cm;

    int close_, sock = (int *)arg, n = 0;

    struct timespec T[2];

    struct timespec s; // time_t (long) sec long nsec

    s.tv_sec = MEDIUM_TERM_SEC; s.tv_nsec = MEDIUM_TERM_NSEC;
    while((close_ = recvmsg(sock, &msg, 0)) != -1){

        if(close_ == 0) break; // close client socket recv return 0

        clock_gettime(CLOCK_REALTIME, &T[0]);

        for (cm = CMSG_FIRSTHDR(&msg); cm; cm = CMSG_NXTHDR(&msg, cm))
                if (SOL_SOCKET == cm->cmsg_level && SO_TIMESTAMPNS == cm->cmsg_type){
                    printf("action SO_TIMESTAMPNS\n");
                    memcpy(&T[0], (struct timespec *)CMSG_DATA(cm), sizeof(struct timespec));
                }

        nanosleep(&s, NULL);

        clock_gettime(CLOCK_REALTIME, &T[1]);

        send(sock, T, sizeof(T), 0);

        printf("%d\nT2: %ld.%ld\nT3: %ld.%ld\n\n", ++n, T[0].tv_sec, T[0].tv_nsec, T[1].tv_sec, T[1].tv_nsec); // time_t (long) : %ld long: %ld

    }

    Thread_t--;

    close(sock);
}

int main(int argc, char *argv[]){

    int sock, new, client_len, enabled = 1; // enabled should be 1

    struct sockaddr_in server_addr, client_addr;

    pthread_t p_thread[BACKLOG];

    /* TCP */

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("socket() failed\n");
        exit(1);
    }

    /* SO_TIMESTAMPNS */

    if(setsockopt(sock, SOL_SOCKET, SO_TIMESTAMPNS, &enabled, sizeof(enabled)) < 0)
        err("setsockopt()");

    /* filename port */

    memset(&server_addr, '\0', sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if(argc == 2) server_addr.sin_port = htons(atoi(argv[1]));

    if(bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
        err("bind()");

    if(listen(sock, BACKLOG) < 0)
        err("listen()");

    while((new = accept(sock, (struct sockaddr*)&client_addr, &client_len)) != -1){
        if(pthread_create(&p_thread[Thread_t], NULL, Server_Socket_Thread, (void *)new) == 0) Thread_t++; // thread success return 0
        printf("%d : Client IP : %s Port : %d\n", Thread_t, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }

    close(sock);

    return 0;
}