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

#define BACKLOG 100 // total client number
#define MEDIUM_TERM_SEC 0
#define MEDIUM_TERM_NSEC 5000000 // nanosecond between receive and transmit

#define PORT 5005 // default port

#define OFFSET_PORT_UDP 5006

#define UDP_OFFSET_THREAD 1 // 쓰래드 수

#define UDP_SYNC_THREAD 1 // 쓰래드 수

int Thread_t = 0; // total thread number played

static void err(const char *error){
    printf("%s: %s\n", error, strerror(errno));
    exit(1);
}

int recv_socket(int sock, struct msghdr *msg, struct sockaddr_in *from_addr){

    /* recvpacket */
    char data[256];
    struct iovec entry;
    struct {
        struct cmsghdr cm;
        char control[512];
    } control;
    memset(msg, 0, sizeof(*msg));
    msg->msg_iov = &entry;
    msg->msg_iovlen = 1;
    entry.iov_base = data;
    entry.iov_len = sizeof(data);
    msg->msg_name = (caddr_t)from_addr;
    msg->msg_namelen = sizeof(*from_addr);
    msg->msg_control = &control;
    msg->msg_controllen = sizeof(control);

    return recvmsg(sock, msg, 0);

}

void *Server_Socket_Thread(void *arg){

    struct msghdr msg;

    /* printpacket */
    struct cmsghdr *cm;

    int close_, n = 0;

    int sock = (int *)arg;

    struct timespec T[2];

    int32_t T_int[4]; // for compatiblility between 32bit and 64bit

    struct timespec s; // time_t (long) sec long nsec

    s.tv_sec = MEDIUM_TERM_SEC; s.tv_nsec = MEDIUM_TERM_NSEC;

    //sock = pth[0];

    while((close_ = recv_socket(sock, &msg, NULL)) != -1){

        if(close_ == 0) break; // close client socket recv return 0

        clock_gettime(CLOCK_REALTIME, &T[0]);

        for (cm = CMSG_FIRSTHDR(&msg); cm; cm = CMSG_NXTHDR(&msg, cm))
                if (SOL_SOCKET == cm->cmsg_level && SO_TIMESTAMPNS == cm->cmsg_type)
                    memcpy(&T[0], (struct timespec *)CMSG_DATA(cm), sizeof(struct timespec));

        T_int[0] = T[0].tv_sec; T_int[1] = T[0].tv_nsec;

        //nanosleep(&s, NULL);

        clock_gettime(CLOCK_REALTIME, &T[1]);

        T_int[2] = T[1].tv_sec; T_int[3] = T[1].tv_nsec;

        send(sock, T_int, sizeof(T_int), 0);

        printf("%d\nT2: %ld.%ld\nT3: %ld.%ld\n\n", ++n, T[0].tv_sec, T[0].tv_nsec, T[1].tv_sec, T[1].tv_nsec); // time_t (long) : %ld long: %ld

    }

    Thread_t--; // Thread crash caution

    close(sock);
}

void *UDP_OFFSET_Thread(void *arg){

    int sock = (int *)arg;

    struct msghdr msg;
    struct sockaddr_in from_addr;

    /* printpacket */
    struct cmsghdr *cm;

    struct timespec T[2];

    int32_t T_int[4]; // for compatiblility between 32bit and 64bit

    struct timespec s; // time_t (long) sec long nsec

    s.tv_sec = MEDIUM_TERM_SEC; s.tv_nsec = MEDIUM_TERM_NSEC;

    while(recv_socket(sock, &msg, &from_addr) > 0){

        printf("Client IP : %s Port : %d\n", inet_ntoa(from_addr.sin_addr), ntohs(from_addr.sin_port));

        clock_gettime(CLOCK_REALTIME, &T[0]);

        for (cm = CMSG_FIRSTHDR(&msg); cm; cm = CMSG_NXTHDR(&msg, cm))
                if (SOL_SOCKET == cm->cmsg_level && SO_TIMESTAMPNS == cm->cmsg_type)
                    memcpy(&T[0], (struct timespec *)CMSG_DATA(cm), sizeof(struct timespec));

        T_int[0] = T[0].tv_sec; T_int[1] = T[0].tv_nsec;

        //nanosleep(&s, NULL);

        clock_gettime(CLOCK_REALTIME, &T[1]);

        T_int[2] = T[1].tv_sec; T_int[3] = T[1].tv_nsec;

        sendto(sock, T_int, sizeof(T_int), 0, (struct sockaddr*)&from_addr, sizeof(from_addr));

        printf("\nT2: %ld.%ld\nT3: %ld.%ld\n\n", T[0].tv_sec, T[0].tv_nsec, T[1].tv_sec, T[1].tv_nsec); // time_t (long) : %ld long: %ld

    }

}

void *UDP_SYNC_Thread(void *arg){
    
    int sock = (int *)arg;

    struct msghdr msg;
    struct sockaddr_in from_addr;

    /* printpacket */
    struct cmsghdr *cm;

    struct timespec T[2];

    int32_t T_int[4]; // for compatiblility between 32bit and 64bit

    struct timespec s; // time_t (long) sec long nsec

    s.tv_sec = MEDIUM_TERM_SEC; s.tv_nsec = MEDIUM_TERM_NSEC;

    while(recv_socket(sock, &msg, &from_addr) > 0){

        printf("Client IP : %s Port : %d\n", inet_ntoa(from_addr.sin_addr), ntohs(from_addr.sin_port));

        clock_gettime(CLOCK_REALTIME, &T[0]);

        for (cm = CMSG_FIRSTHDR(&msg); cm; cm = CMSG_NXTHDR(&msg, cm))
                if (SOL_SOCKET == cm->cmsg_level && SO_TIMESTAMPNS == cm->cmsg_type)
                    memcpy(&T[0], (struct timespec *)CMSG_DATA(cm), sizeof(struct timespec));

        T_int[0] = T[0].tv_sec; T_int[1] = T[0].tv_nsec;

        //nanosleep(&s, NULL);

        clock_gettime(CLOCK_REALTIME, &T[1]);

        T_int[2] = T[1].tv_sec; T_int[3] = T[1].tv_nsec;

        sendto(sock, T_int, sizeof(T_int), 0, (struct sockaddr*)&from_addr, sizeof(from_addr));

        printf("\nT2: %ld.%ld\nT3: %ld.%ld\n\n", T[0].tv_sec, T[0].tv_nsec, T[1].tv_sec, T[1].tv_nsec); // time_t (long) : %ld long: %ld

    }

    Thread_t--;

}

int TCP_server(struct sockaddr_in *server_addr){

    pthread_t p_thread[BACKLOG]; // thread identifier

    struct sockaddr_in client_addr;

    int sock, new, client_len = sizeof(client_addr), enabled = 1; // enabled should be 1

    /* TCP */

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("TCP socket() failed\n");
        exit(1);
    }

    printf("TCP socket() success\n");

    /* SO_TIMESTAMPNS */

    if(setsockopt(sock, SOL_SOCKET, SO_TIMESTAMPNS, &enabled, sizeof(enabled)) < 0)
        err("setsockopt()");

    if(bind(sock, (struct sockaddr*)server_addr, sizeof(*server_addr)) < 0)
        err("bind()");

    printf("bind() success\n");

    if(listen(sock, BACKLOG) < 0)
        err("listen()");

    printf("listen() success\n");

    while((new = accept(sock, (struct sockaddr*)&client_addr, &client_len)) != -1){
        if(Thread_t < 0) Thread_t = 0; // additional consideration is demanded
        if(pthread_create(&p_thread[0], NULL, Server_Socket_Thread, (void *)new) == 0) Thread_t++; // thread success return 0 pthread overlap is ok but index -1 is not ok
        printf("%d : Client IP : %s Port : %d\n", Thread_t, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }

    close(sock);

    return 0;
}

int UDP_server(struct sockaddr_in *server_addr){

    struct sockaddr_in offset_server_addr;

    memset(&offset_server_addr, '\0', sizeof(offset_server_addr));

    offset_server_addr.sin_family = AF_INET;
    offset_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    offset_server_addr.sin_port = htons(OFFSET_PORT_UDP);

    int i, sock, offset_sock, enabled = 1;

    Thread_t = UDP_SYNC_THREAD;

    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        printf("UDP socket() failed\n");
        exit(1);
    }

    if((offset_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        printf("UDP offset_socket() failed\n");
        exit(1);
    }

    printf("UDP socket() success\n");

    if(setsockopt(sock, SOL_SOCKET, SO_TIMESTAMPNS, &enabled, sizeof(enabled)) < 0)
        err("setsockopt()");

    if(setsockopt(offset_sock, SOL_SOCKET, SO_TIMESTAMPNS, &enabled, sizeof(enabled)) < 0)
        err("offset_setsockopt()");

    if(bind(sock, (struct sockaddr*)server_addr, sizeof(*server_addr)) < 0)
        err("bind()");
    
    if(bind(offset_sock, (struct sockaddr*)server_addr, sizeof(*server_addr)) < 0)
        err("offset_bind()");

    printf("bind() success\n\n");

    for(i = 0; i < UDP_OFFSET_THREAD; i++){
        if(pthread_create(&p_thread[0], NULL, UDP_OFFSET_Thread, (void *)offset_sock) != 0)
            err("thread error");
    }

    for(i = 0; i < UDP_SYNC_THREAD; i++){
        if(pthread_create(&p_thread[0], NULL, UDP_SYNC_Thread, (void *)sock) != 0)
            err("thread error");
    }
    
    while(Thread_t != 0);

    close(sock);
    close(offset_sock);

    return 0;
}

int main(int argc, char *argv[]){

    struct sockaddr_in server_addr;

    int protocol = 0;

    /* filename port */

    memset(&server_addr, '\0', sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if(argc >= 2) server_addr.sin_port = htons(atoi(argv[1]));

    if(argc == 3 && atoi(argv[2]) == 1) protocol = 1;

    if (argc > 3) {
		printf("Input exceeded\n");
		return 0;
	}

    if (protocol == 0)
		UDP_server(&server_addr);

	else
		TCP_server(&server_addr);

    return 0;
}
