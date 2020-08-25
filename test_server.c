/* Multi_Thread */

#include <stdio.h>
#include <stdlib.h> // exit(0) 'normal' exit(1) 'error'
#include <errno.h>
#include <string.h>
#include <stdint.h> // int32_t int64_t

#include <sys/time.h>
#include <sys/socket.h> // msghdr cmsghdr
#include <sys/types.h>
#include <sys/ioctl.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>


// Header about Timestamp
#include "linux/errqueue.h"


#define BACKLOG 5 // total client number

#define PORT 5005

static void err(const char *error){
    printf("%s: %s\n", error, strerror(errno));
    exit(1);
}

static const unsigned char sync[] = {
    0x00, 0x01, 0x00, 0x01
};


int main(int argc, char *argv[]){

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
    entry.iov_base = data; // 데이터 전송
    entry.iov_len = sizeof(data);
    msg.msg_name = (caddr_t)&from_addr;
    msg.msg_namelen = sizeof(from_addr);
    msg.msg_control = &control;
    msg.msg_controllen = sizeof(control);

    /* printpacket */
    struct cmsghdr *cm;
    struct sockaddr_in *p_from_addr = (struct sockaddr_in *)msg.msg_name;
    struct timespec *ts;
    struct timespec *T = (struct timespec *)msg.msg_iov->iov_base;

    int res, client_len;
    int sock, new;
    struct sockaddr_in server_addr, client_addr;
    int enabled = 1;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
        err("socket");

    if (setsockopt(sock, SOL_SOCKET, SO_TIMESTAMPNS, &enabled, sizeof(enabled)) < 0)
        printf("ERROR: setsockopt SO_TIMESTAMPING\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT /* SYNC default */);

    if(argc == 2) server_addr.sin_port = htons(atoi(argv[1]));

    if(bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        printf("bind() failed\n");
        exit(1);
    }

    if(listen(sock, BACKLOG) == -1){
        printf("listen() failed\n");
        exit(1);
    }

    if((new = accept(sock, (struct sockaddr*)&client_addr, &client_len)) == -1){
        printf("Client IP : %s Port : %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        printf("accept error\n");
        exit(1);
    }

    if((recvmsg(new, &msg, 0)) != -1){
        printf("T1 : %ld.%09ld\n", T[0].tv_sec, T[0].tv_nsec);
        printf("T2 : %ld.%09ld\n", T[1].tv_sec, T[1].tv_nsec);
            for (cm = CMSG_FIRSTHDR(&msg); cm; cm = CMSG_NXTHDR(&msg, cm))
            {
                if (SOL_SOCKET == cm->cmsg_level && SO_TIMESTAMPNS == cm->cmsg_type) {
                    printf("from %s\n", inet_ntoa(p_from_addr->sin_addr));
                    //memcpy(ts, CMSG_DATA(cm), sizeof(struct timespec));
                    ts = (struct timespec *)CMSG_DATA(cm);
                    printf("SW TIMESTAMP %ld.%09ld\n", (long)ts->tv_sec, (long)ts->tv_nsec);
                }
            }
    }


    return 0;
}