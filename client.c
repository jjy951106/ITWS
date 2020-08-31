/* Single Thread */

#include <stdio.h>
#include <stdlib.h> // exit(0) 'normal' exit(1) 'error'
#include <string.h>
#include <stdint.h> // int32_t int64_t

#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <unistd.h>  // sleep usleep
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#include <errno.h>
#include "linux/errqueue.h"

#define SERVER "192.168.0.160" // test server
#define PORT 5005              // default port

#define ITERATION 10
#define MEDIUM_TERM_SEC 0
#define MEDIUM_TERM_NSEC 0 // nanosecond between receive and transmit

#define DEVIATION 3000000 // nanosecond

/* requirement : 5ms */

#define BOUNDARY 3000000 // plus(ns)
#define BOUNDARY_ -3000000 // minus(ns)

struct timespec T_;

static const unsigned char binary[] = {
    0x00, 0x01, 0x00, 0x01
};

static void err(const char *error){
    printf("%s: %s\n", error, strerror(errno));
    exit(1);
}

void offset_calculated(int sock, int *offset){

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
    struct sockaddr_in *p_from_addr = (struct sockaddr_in *)msg.msg_name;
    struct timespec *ts = (struct timespec *)msg.msg_iov->iov_base;

    struct timespec T[4], s;

    int temp1, temp2;

    s.tv_sec = MEDIUM_TERM_SEC; s.tv_nsec = MEDIUM_TERM_NSEC;

    for(int iter = 0; iter < ITERATION; iter++){

        clock_gettime(CLOCK_REALTIME, &T[0]);

        /* send */

        if(send(sock, binary, sizeof(binary), 0) < 0)
            err("send()");

        /* recvmsg */

        if(recvmsg(sock, &msg, 0) < 0)
            err("recv()");

        else{
            memcpy(&T[1], &ts[0], sizeof(struct timespec));
            memcpy(&T[2], &ts[1], sizeof(struct timespec));
            memcpy(&T_, &ts[1], sizeof(struct timespec));

            for (cm = CMSG_FIRSTHDR(&msg); cm; cm = CMSG_NXTHDR(&msg, cm))
                if (SOL_SOCKET == cm->cmsg_level && SO_TIMESTAMPNS == cm->cmsg_type)
                    memcpy(&T[3], (struct timespec *)CMSG_DATA(cm), sizeof(struct timespec));
        }

        temp1 = ((T[1].tv_sec - T[0].tv_sec) - (T[3].tv_sec - T[2].tv_sec));

        temp2 = ((T[1].tv_nsec - T[0].tv_nsec) - (T[3].tv_nsec - T[2].tv_nsec));

        //if(abs(temp1) < 1 && abs(temp2) <= DEVIATION){
            offset[0] += temp1;
            offset[1] += temp2;
        //}
        //else iter--;

        nanosleep(&s, NULL);
    }

    offset[0] /= (2 * ITERATION);
    offset[1] /= (2 * ITERATION);

}

int main(int argc, char *argv[]){

    int sock, temp1, temp2, enabled = 1;

    struct sockaddr_in server_addr;

    struct timespec C; // C (current)

    int32_t tmp, offset[2] = { 0, };

    /* TCP */

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        err("socket()");

    /* SO_TIMESTAMPNS */

    if(setsockopt(sock, SOL_SOCKET, SO_TIMESTAMPNS, &enabled, sizeof(enabled)) < 0)
        err("setsockopt()");

    /* filename server_ip port */

    memset(&server_addr, '\0', sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER /* 192.168.0.160 */);
    server_addr.sin_port = htons(PORT /* 5005 */);

    if(argc >= 2) server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    if(argc == 3) server_addr.sin_port = htons(atoi(argv[2]));

    if(connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
        err("connect()");

    while(1){

        offset_calculated(sock, offset);

        printf("offset : %d.%d\n", offset[0], offset[1]);

        if(abs(offset[0]) > 0){
            clock_settime(CLOCK_REALTIME, &T_);
            sleep(1);
            continue;
        }

        else if(offset[1] < BOUNDARY && offset[1] > BOUNDARY_){
            printf("%d ns\n", offset[1]);
            break;
        }

        /* offset compensation */

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