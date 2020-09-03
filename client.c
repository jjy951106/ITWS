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
#define MEDIUM_TERM_NSEC 0 // ns between receive and transmit

/* requirement : 5ms */

#define BOUNDARY 1000000 // plus(ns)
#define BOUNDARY_ -1000000 // minus(ns)

int32_t DEVIATION = 10000000; // ns

struct timespec T_;

static const unsigned char binary[] = {
    0x00, 0x01, 0x00, 0x01
};

static void err(const char *error){
    printf("%s: %s\n", error, strerror(errno));
    exit(1);
}

void initialized_T(int sock){

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

    int32_t *T_int = (int32_t *)msg.msg_iov->iov_base;
    
    struct timespec T;

    if(send(sock, binary, sizeof(binary), 0) < 0)
        err("send()");

    if(recvmsg(sock, &msg, 0) < 0)
        err("recv()");
    
    T.tv_sec = T_int[2]; T.tv_nsec = T_int[3];
        
    //printf("%ld.%ld\n", T.tv_sec, T.tv_nsec);

    clock_settime(CLOCK_REALTIME, &T);

}

void iterative_offset_calculated(int sock, int *offset){

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

    int32_t *T_int = (int32_t *)msg.msg_iov->iov_base;

    struct timespec T[4], s;

    int temp1, temp2, iter, iteration = 0;

    s.tv_sec = MEDIUM_TERM_SEC; s.tv_nsec = MEDIUM_TERM_NSEC;

    for(iter = 0; iter < ITERATION; iter++){

        clock_gettime(CLOCK_REALTIME, &T[0]);

        /* send */

        if(send(sock, binary, sizeof(binary), 0) < 0)
            err("send()");

        /* recvmsg */

        if(recvmsg(sock, &msg, 0) < 0)
            err("recv()");

        else{
            T[1].tv_sec = T_int[0]; T[1].tv_nsec = T_int[1];
            T[2].tv_sec = T_int[2]; T[2].tv_nsec = T_int[3];
            clock_gettime(CLOCK_REALTIME, &T[3]); // if no action SO_TIMESTAMPNS
            memcpy(&T_, &T[2], sizeof(struct timespec));
            for (cm = CMSG_FIRSTHDR(&msg); cm; cm = CMSG_NXTHDR(&msg, cm))
                if (SOL_SOCKET == cm->cmsg_level && SO_TIMESTAMPNS == cm->cmsg_type)
                    memcpy(&T[3], (struct timespec *)CMSG_DATA(cm), sizeof(struct timespec));
        }

        /* T1 : T[0], T2 : T[1], T3 : T[2], T4 : T[3] */
        temp1 = ((T[1].tv_sec - T[0].tv_sec) - (T[3].tv_sec - T[2].tv_sec));

        temp2 = ((T[1].tv_nsec - T[0].tv_nsec) - (T[3].tv_nsec - T[2].tv_nsec));

        /*printf("T1 : %ld.%ld\nT2 : %ld.%ld\nT3 : %ld.%ld\nT4 : %ld.%ld\n",
        	T[0].tv_sec, T[0].tv_nsec, T[1].tv_sec, T[1].tv_nsec,
        	T[2].tv_sec, T[2].tv_nsec, T[3].tv_sec, T[3].tv_nsec);*/

        /* DEVIATION */
        if(abs(temp1) < 1 && abs(temp2) <= DEVIATION){
            offset[0] += temp1;
            offset[1] += temp2;
            iteration++;
        }

        nanosleep(&s, NULL);
    }

    printf("iteration : %d \n", iteration);

    if(iteration >= 3){ // the minimum number of iterations that within the deviation is more than 5 in total 10
        offset[0] /= (2 * iteration);
        offset[1] /= (2 * iteration);
    }

    /* DEVIATION INCREASING */
    else{
        DEVIATION += 10000000;
        offset[0] = 0;
        offset[1] = 0;
    }

    if(iteration >= 7 && iteration <= ITERATION) // increase accuracy
        DEVIATION -= 5000000;
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

    int32_t *T_int = (int32_t *)msg.msg_iov->iov_base;

    struct timespec T[4];

    clock_gettime(CLOCK_REALTIME, &T[0]);

    /* send */

    if(send(sock, binary, sizeof(binary), 0) < 0)
        err("send()");

    /* recvmsg */

    if(recvmsg(sock, &msg, 0) < 0)
        err("recv()");

    else{
        T[1].tv_sec = T_int[0]; T[1].tv_nsec = T_int[1];
        T[2].tv_sec = T_int[2]; T[2].tv_nsec = T_int[3];
        clock_gettime(CLOCK_REALTIME, &T[3]); // if no action SO_TIMESTAMPNS
        for (cm = CMSG_FIRSTHDR(&msg); cm; cm = CMSG_NXTHDR(&msg, cm))
            if (SOL_SOCKET == cm->cmsg_level && SO_TIMESTAMPNS == cm->cmsg_type){
                printf("action SO_TIMESTAMPNS\n");
                memcpy(&T[3], (struct timespec *)CMSG_DATA(cm), sizeof(struct timespec));
            }
    }

    /* T1 : T[0], T2 : T_int[0], T_int[1], T3 : T_int[2], T_int[3], T4 : T[1] */
    offset[0] = ((T[1].tv_sec - T[0].tv_sec) - (T[3].tv_sec - T[2].tv_sec)) / 2;

    offset[1] = ((T[1].tv_nsec - T[0].tv_nsec) - (T[3].tv_nsec - T[2].tv_nsec)) / 2;

}

void mode_1(int sock){

    int32_t tmp, offset[2] = { 0, };

    struct timespec C; // C (current)

    initialized_T(sock);

    while(1){

        iterative_offset_calculated(sock, offset);

        printf("offset : %d.%d\n", offset[0], offset[1]);

        if(offset[0] == 0 && offset[1] == 0) // Not enough samples
            continue;

        else if(abs(offset[0]) > 0){
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

        if((tmp = C.tv_nsec + offset[1]) >= 1000000000){
            C.tv_sec += 1;
            C.tv_nsec = tmp - 1000000000;
        }

        else
            C.tv_nsec = tmp;

        clock_settime(CLOCK_REALTIME, &C);

    }

}

void mode_2(int sock){

    int32_t tmp, offset[2] = { 0, }, offset_check = 0;

    struct timespec C; // C (current)

    initialized_T(sock);

    while(1){

        offset_calculated(sock, offset);

        sleep(1);

        if(abs(offset[0]) > 1 || abs(offset[1]) > 5000000){
            printf("offset_check : %d\n", ++offset_check);
        }

        offset[0] = 0;
        offset[1] = 0;

        if(offset_check >= 3){

            offset_check = 0;

            iterative_offset_calculated(sock, offset);

            printf("offset : %d.%d\n", offset[0], offset[1]);

            if(offset[0] == 0 && offset[1] == 0) // Not enough samples
                continue;

            else if(abs(offset[0]) > 0){
                clock_settime(CLOCK_REALTIME, &T_);
                sleep(1);
                continue;
            }

            else if(offset[1] < BOUNDARY && offset[1] > BOUNDARY_){
                printf("%d ns\n", offset[1]);
                continue;
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

            sleep(3);

        }

    }

}

void mode_3(int sock){

    int32_t offset[2] = { 0, };

    offset_calculated(sock, offset);

    printf("offset : %d.%d\n", offset[0], offset[1]);

}

int main(int argc, char *argv[]){

    int sock, temp1, temp2, enabled = 1, mode = 1;

    struct sockaddr_in server_addr;

    /* TCP */

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        err("socket()");

    /* SO_TIMESTAMPNS */

    if(setsockopt(sock, SOL_SOCKET, SO_TIMESTAMPNS, &enabled, sizeof(enabled)) < 0)
        err("setsockopt()");

    /* filename server_ip port mode(default 1) */

    memset(&server_addr, '\0', sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER /* 192.168.0.160 */);
    server_addr.sin_port = htons(PORT /* 5005 */);

    if(argc >= 2) server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    if(argc == 3) server_addr.sin_port = htons(atoi(argv[2]));

    if(argc == 4) mode = atoi(argv[3]);

    if(connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
        err("connect()");

    if(mode == 1)
        mode_1(sock);

    else if(mode == 2)
        mode_2(sock);

    else if(mode == 3)
        mode_3(sock);

    else
    {
        printf("mode 1 : sync\nmode 2 : sync continuous\nmode 3 : offset print\n");
    }

    close(sock);

}
