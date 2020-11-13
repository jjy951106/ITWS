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

#include <netdb.h> // domain address

#define SERVER "192.168.0.160" // test server
#define PORT 5005              // default port
#define OFFSET_PORT_UDP 5006   // udp offset thread socket port

#define ITERATION 10
#define MEDIUM_TERM_SEC 0
#define MEDIUM_TERM_NSEC 0 // ns between receive and transmit

/* requirement : 5ms */

#define BOUNDARY 5000000 // plus(ns)
#define BOUNDARY_ -5000000 // minus(ns)

int32_t DEVIATION = 50000000; // ns

struct timespec T_, T_present;

int32_t delay[2] = { 0, };

int32_t thr = 5000000; // threshold default 5,000,000 ns

static const unsigned char binary[] = {
    0x00, 0x01, 0x00, 0x01
};

static void err(const char *error){
    printf("%s: %s\n", error, strerror(errno));
    exit(1);
}

int select_mode(int sock, int mode, struct sockaddr_in *server_addr, int protocol){

    if(mode == 1)
        mode_1(sock, server_addr, protocol);

    else if(mode == 2)
        mode_2(sock, server_addr, protocol);

    else if(mode == 3)
        mode_3(sock, server_addr, protocol);

    else
        printf("mode 1 : sync\nmode 2 : sync continuous\nmode 3 : offset print\n");

}

void send_socket(int sock, struct sockaddr_in *server_addr, int protocol){

    if(protocol == 0 /* UDP */)
        if(sendto(sock, binary, sizeof(binary), 0, (struct sockaddr*)server_addr, sizeof(*server_addr)) < 0)
            err("sendto()");

    if(protocol == 1 /* TCP */)
        if(send(sock, binary, sizeof(binary), 0) < 0)
            err("send()");

}

void recv_socket(int sock, struct msghdr *msg, struct sockaddr_in *server_addr, int protocol, struct timespec *T){

    int re;

    /* recvpacket */
    char data[256];
    struct iovec entry;
    struct sockaddr_in from_addr;
    struct {
        struct cmsghdr cm;
        char control[512];
    } control;
    memset(msg, 0, sizeof(*msg));
    msg->msg_iov = &entry;
    msg->msg_iovlen = 1;
    entry.iov_base = data;
    entry.iov_len = sizeof(data);
    msg->msg_name = (caddr_t)&from_addr;
    msg->msg_namelen = sizeof(from_addr);
    msg->msg_control = &control;
    msg->msg_controllen = sizeof(control);

    re = recvmsg(sock, msg, 0);

    while(re == -1){
        clock_gettime(CLOCK_REALTIME, T);
        //printf("retransmission : %d.%d\n", T->tv_sec, T->tv_nsec);//
        send_socket(sock, server_addr, protocol);
        re = recvmsg(sock, msg, 0);
    }

    if(re < -1)
        err("recv()");

}

void initialized_T(int sock, struct sockaddr_in *server_addr, int protocol){

    struct msghdr msg;

    struct timespec T, NULL_T;

    int32_t *T_int;

    send_socket(sock, server_addr, protocol);

    recv_socket(sock, &msg, server_addr, protocol, &NULL_T);

    T_int = (int32_t *)msg.msg_iov->iov_base;

    T.tv_sec = T_int[2]; T.tv_nsec = T_int[3];

    clock_settime(CLOCK_REALTIME, &T);

}

void offset_calculated(int sock, int *offset, struct sockaddr_in *server_addr, int protocol){

    struct msghdr msg;

    /* printpacket */
    struct cmsghdr *cm;

    int32_t *T_int;

    struct timespec T[4];

    clock_gettime(CLOCK_REALTIME, &T[0]);

    send_socket(sock, server_addr, protocol);

    recv_socket(sock, &msg, server_addr, protocol, &T[0]); // print critical offset calculation error

    clock_gettime(CLOCK_REALTIME, &T[3]); // if no action SO_TIMESTAMPNS

    T_int = (int32_t *)msg.msg_iov->iov_base;

    T[1].tv_sec = T_int[0]; T[1].tv_nsec = T_int[1];
    T[2].tv_sec = T_int[2]; T[2].tv_nsec = T_int[3];

    memcpy(&T_, &T[2], sizeof(struct timespec));

    for (cm = CMSG_FIRSTHDR(&msg); cm; cm = CMSG_NXTHDR(&msg, cm))
        if (SOL_SOCKET == cm->cmsg_level && SO_TIMESTAMPNS == cm->cmsg_type){
            //printf("SO_TIMESTAMPNS action\n");
            memcpy(&T[3], (struct timespec *)CMSG_DATA(cm), sizeof(struct timespec));
        }

    memcpy(&T_present, &T[3], sizeof(struct timespec));

    /* T1 : T[0], T2 : T_int[0], T_int[1], T3 : T_int[2], T_int[3], T4 : T[1] */
    offset[0] = ((T[1].tv_sec - T[0].tv_sec) - (T[3].tv_sec - T[2].tv_sec)) / 2;

    offset[1] = ((T[1].tv_nsec - T[0].tv_nsec) - (T[3].tv_nsec - T[2].tv_nsec)) / 2;

    //printf("T[0] : %d.%d\n", T[0].tv_sec, T[0].tv_nsec);//
    
    // delay add

    delay[0] = ((T[1].tv_sec - T[0].tv_sec) + (T[3].tv_sec - T[2].tv_sec)) / 2;

    delay[1] = ((T[1].tv_nsec - T[0].tv_nsec) + (T[3].tv_nsec - T[2].tv_nsec)) / 2;
}

void iterative_offset_calculated(int sock, int32_t *offset, struct sockaddr_in *server_addr, int protocol){

    int32_t temp[2] = { 0, };

    struct timespec s;

    int iter, iteration = 0;

    s.tv_sec = MEDIUM_TERM_SEC; s.tv_nsec = MEDIUM_TERM_NSEC;

    for(iter = 0; iter < ITERATION; iter++){

        offset_calculated(sock, temp, server_addr, protocol);

        /* DEVIATION */
        if(abs(temp[0]) < 1 && abs(temp[1]) <= DEVIATION){
            offset[0] += temp[0];
            offset[1] += temp[1];
            iteration++;
        }

        nanosleep(&s, NULL);
    }

    //printf("iteration : %d \n", iteration);

    if(iteration >= 3){ // the minimum number of iterations that within the deviation is more than 5 in total 10
        offset[0] /= iteration;
        offset[1] /= iteration;
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

void mode_1(int sock, struct sockaddr_in *server_addr, int protocol){

    int32_t tmp, offset[2];

    struct timespec C; // C (current)

    initialized_T(sock, server_addr, protocol);

    while(1){

        offset[0] = 0;
        offset[1] = 0;

        iterative_offset_calculated(sock, offset, server_addr, protocol);

        //printf("offset : %d.%d\n", offset[0], offset[1]);

        if(offset[0] == 0 && offset[1] == 0) // Not enough samples
            continue;

        else if(abs(offset[0]) > 0){
            clock_settime(CLOCK_REALTIME, &T_);
            sleep(1);
            continue;
        }

        else if(offset[1] < BOUNDARY && offset[1] > BOUNDARY_){
            //printf("\nsuccess : %d ns\n\n", offset[1]);
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

void mode_2(int sock, struct sockaddr_in *server_addr, int protocol){

    int32_t offset[2], offset_check = 0;

    struct timespec C; // C (current)

    int offset_interval = 3; // offset measurament interval

    initialized_T(sock, server_addr, protocol);

    while(1){

        offset[0] = 0;
        offset[1] = 0;

        offset_calculated(sock, offset, server_addr, protocol);

        //printf("offset : %ds %dns\n", offset[0], offset[1]);

        sleep(offset_interval);

        if(abs(offset[0]) > 1 || abs(offset[1]) > thr) offset_check++;

        if(offset_check >= 3){

            offset_check = 0;

            mode_1(sock, server_addr, protocol);

            sleep(3);

        }

    }

}

void mode_3(int sock, struct sockaddr_in *server_addr, int protocol){

    struct tm *server_date, *drone_date;

    int drone_ms, server_ms;

    int32_t offset[2] = { 0, };

//    if(protocol == 0) // if udp
//        server_addr->sin_port = htons(OFFSET_PORT_UDP); // udp offset thread socket port

    offset_calculated(sock, offset, server_addr, protocol);

    // delay reward

    T_.tv_sec += delay[0];

    T_.tv_nsec += delay[1];

    const time_t t1 = T_present.tv_sec;
    const time_t t2 = T_.tv_sec;

    drone_date = localtime(&t1);
    server_date = localtime(&t2);

    drone_ms = T_present.tv_nsec / 1000000;

    server_ms = T_.tv_nsec / 1000000;
    
    // Server Time
    if(server_ms < 100){
        if(server_date->tm_hour < 10)
            printf("%d%d%dT0%d%d%d0%d+", server_date->tm_year + 1900 , server_date->tm_mon + 1 , server_date->tm_mday , server_date->tm_hour , server_date->tm_min , server_date->tm_sec, server_ms);
        else
            printf("%d%d%dT%d%d%d0%d+", server_date->tm_year + 1900 , server_date->tm_mon + 1 , server_date->tm_mday , server_date->tm_hour , server_date->tm_min , server_date->tm_sec, server_ms);
    }
    else{
        if(server_date->tm_hour < 10)
            printf("%d%d%dT0%d%d%d%d+", server_date->tm_year + 1900 , server_date->tm_mon + 1 , server_date->tm_mday , server_date->tm_hour , server_date->tm_min , server_date->tm_sec, server_ms);
        else
            printf("%d%d%dT%d%d%d%d+", server_date->tm_year + 1900 , server_date->tm_mon + 1 , server_date->tm_mday , server_date->tm_hour , server_date->tm_min , server_date->tm_sec, server_ms);
    }
    // MC_Drone Time
    if(drone_ms < 100){
        if(drone_date->tm_hour < 10)
            printf("%d%d%dT0%d%d%d0%d+", drone_date->tm_year + 1900 , drone_date->tm_mon + 1 , drone_date->tm_mday , drone_date->tm_hour , drone_date->tm_min , drone_date->tm_sec, drone_ms);
        else
            printf("%d%d%dT%d%d%d0%d+", drone_date->tm_year + 1900 , drone_date->tm_mon + 1 , drone_date->tm_mday , drone_date->tm_hour , drone_date->tm_min , drone_date->tm_sec, drone_ms);
    }
    else{
        if(drone_date->tm_hour < 10)
            printf("%d%d%dT0%d%d%d%d+", drone_date->tm_year + 1900 , drone_date->tm_mon + 1 , drone_date->tm_mday , drone_date->tm_hour , drone_date->tm_min , drone_date->tm_sec, drone_ms);
        else
            printf("%d%d%dT%d%d%d%d+", drone_date->tm_year + 1900 , drone_date->tm_mon + 1 , drone_date->tm_mday , drone_date->tm_hour , drone_date->tm_min , drone_date->tm_sec, drone_ms);
    }
    
    // Offset
    printf("%d+", (offset[0] * 1000) + (offset[1] / 1000000)); // ms
    
}

int TCP_socket(struct sockaddr_in *server_addr, int mode, int protocol){

    int sock, enabled = 1;

    struct timeval tv;

    tv.tv_sec = 3;
    tv.tv_usec = 0;

    /* TCP */

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        err("TCP socket()");

    //printf("TCP socket() success\n");

    /* SO_TIMESTAMPNS */

    if(setsockopt(sock, SOL_SOCKET, SO_TIMESTAMPNS, &enabled, sizeof(enabled)) < 0)
        err("SO_TIMESTAMPNS setsockopt()");

    if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval)) < 0)
        err("SO_RCVTIMEO setsockopt()");

    if(connect(sock, (struct sockaddr*)server_addr, sizeof(*server_addr)) < 0)
        err("connect()");

    //printf("conect() success\n\n");

    select_mode(sock, mode, server_addr, protocol);

    close(sock);

    return 0;

}

int UDP_socket(struct sockaddr_in *server_addr, int mode, int protocol){

    int sock, enabled = 1;

    struct timeval tv;

    tv.tv_sec = 3;
    tv.tv_usec = 0;

    struct sockaddr_in client_addr;

    int client_len = sizeof(client_addr);

    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    client_addr.sin_port = htons(0);

    /* UDP */

    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        err("UDP socket()");

    //printf("UDP socket() success\n");

    /* SO_TIMESTAMPNS */

    if(setsockopt(sock, SOL_SOCKET, SO_TIMESTAMPNS, &enabled, sizeof(enabled)) < 0)
        err("SO_TIMESTAMPNS setsockopt()");

    if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval)) < 0)
        err("SO_RCVTIMEO setsockopt()");

    if(bind(sock, (struct sockaddr*)&client_addr, client_len) < 0)
        err("bind()");

    //printf("bind() success\n\n");

    select_mode(sock, mode, server_addr, protocol);

    close(sock);

    return 0;

}

int main(int argc, char *argv[]){

    int mode = 2 /*default mode 2*/, protocol = 0;;

    struct sockaddr_in server_addr;

    char *IPbuffer;
    struct hostent *host_entry;

    memset(&server_addr, '\0', sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER /* 192.168.0.160 */);
    server_addr.sin_port = htons(PORT /* 5005 */);

    if(argc >= 2) mode = atoi(argv[1]);   

    if(argc >= 3){
        // To retrieve host information
        host_entry = gethostbyname(argv[2]);

        // To convert an Internet network
        // address into ASCII string
        IPbuffer = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));

        server_addr.sin_addr.s_addr = inet_addr(IPbuffer);
    }

    if(argc >= 4) server_addr.sin_port = htons(atoi(argv[3]));

    if(argc >= 5 && atoi(argv[4]) == 1) protocol = 1; // default 0 : UDP

    if(argc == 6) thr = atoi(argv[5]) * 1000000; // threshold default 5,000,000 ns // input ms

    if (argc > 6) {
		printf("Input exceeded\n");
		return 0;
	}

    if (protocol == 0){
        //printf("UDP client\n\n");
		UDP_socket(&server_addr, mode, protocol);
    }

	else{
        //printf("TCP client\n\n");
		TCP_socket(&server_addr, mode, protocol);
    }

    return 0;

}
