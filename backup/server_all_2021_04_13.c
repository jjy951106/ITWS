/* Multi_Thread */

#include <stdio.h>
#include <stdlib.h> // exit(0) 'normal' exit(1) 'error'
#include <string.h>
#include <stdint.h> // int32_t int64_t
#include <assert.h>

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

#define BACKLOG 100

#define MEDIUM_TERM_SEC 0
#define MEDIUM_TERM_NSEC 10000000 // nanosecond between receive and transmit

#define PORT 5005 // default port

#define FC_COMPS_PORT_UDP 5006

typedef struct udp_thread_factor{

    int sock;
    struct msghdr msg;
    struct sockaddr_in from_addr;

}udp_thread_factor;

struct timespec s = { MEDIUM_TERM_SEC, MEDIUM_TERM_NSEC }; // time_t (long) sec long nsec for sleep

int Thread_t = 0; // total thread number played

double Compenstate_FC_MC = 0; // Offset between FC and MC 

double Compenstate_FC_MC_sync = 0;

int chage_comps = 0;

int64_t _atoi(char* cdata){
    int sign = 1;
    int64_t data = 0;
 
    if (*cdata == '\n')
        return 0;
    if (*cdata == '-'){
        sign = -1;
        cdata++;
    }
 
    while (*cdata){
        if (*cdata >= '0' && *cdata <= '9'){
            data = data * 10 + *cdata - '0';
        }
        else{
            break;
            //assert(0);
        }
        cdata++;
    }
    return data * sign;
}

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

    int32_t comps_sec;
    int32_t comps_nsec;

    while((close_ = recv_socket(sock, &msg, NULL)) != -1){

        if(close_ == 0) break; // close client socket recv return 0
 
        clock_gettime(CLOCK_REALTIME, &T[0]);

        for (cm = CMSG_FIRSTHDR(&msg); cm; cm = CMSG_NXTHDR(&msg, cm))
                if (SOL_SOCKET == cm->cmsg_level && SO_TIMESTAMPNS == cm->cmsg_type)
                    memcpy(&T[0], (struct timespec *)CMSG_DATA(cm), sizeof(struct timespec));

        comps_sec = (int32_t)(Compenstate_FC_MC / 1000);
        comps_nsec = (int32_t)((Compenstate_FC_MC - (int64_t)(Compenstate_FC_MC / 1000) * 1000) * 1000000);

        T_int[0] = T[0].tv_sec + (int32_t)(Compenstate_FC_MC / 1000); 
        T_int[1] = T[0].tv_nsec + (int32_t)((Compenstate_FC_MC - (int64_t)(Compenstate_FC_MC / 1000) * 1000) * 1000000);

        if(T_int[1] >= 1000000000){
            T_int[0] += 1;
            T_int[1] = T_int[1] % 1000000000;
        }

        if(T_int[1] < 0){
            T_int[0] -= 1;
            T_int[1] = 1000000000 + T_int[1];
        }
        
        nanosleep(&s, NULL);

        clock_gettime(CLOCK_REALTIME, &T[1]);

        T_int[2] = T[1].tv_sec + (int32_t)(Compenstate_FC_MC / 1000); 
        T_int[3] = T[1].tv_nsec + (int32_t)((Compenstate_FC_MC - (int64_t)(Compenstate_FC_MC / 1000) * 1000) * 1000000);

        if(T_int[3] >= 1000000000){
            T_int[2] += 1;
            T_int[3] = T_int[3] % 1000000000;
        }

        if(T_int[3] < 0){
            T_int[2] -= 1;
            T_int[3] = 1000000000 + T_int[3];
        }

        send(sock, T_int, sizeof(T_int), 0);

        printf("%d\nT2: %ld.%ld\nT3: %ld.%ld\n\n", ++n, T[0].tv_sec, T[0].tv_nsec, T[1].tv_sec, T[1].tv_nsec); // time_t (long) : %ld long: %ld

    }

    Thread_t--; // Thread crash caution

    close(sock);

    return 0;
}

void *UDP_Thread(void *args){

    udp_thread_factor utf = *((udp_thread_factor*)args);

    /* printpacket */
    struct cmsghdr *cm;

    struct timespec T[2];

    int32_t T_int[4]; // for compatiblility between 32bit and 64bit

    int32_t comps_sec = (int32_t)(Compenstate_FC_MC / 1000);
    int32_t comps_nsec = (int32_t)((Compenstate_FC_MC - (int64_t)(Compenstate_FC_MC / 1000) * 1000) * 1000000);

    printf("Client IP : %s Port : %d\n", inet_ntoa(utf.from_addr.sin_addr), ntohs(utf.from_addr.sin_port));

    clock_gettime(CLOCK_REALTIME, &T[0]);

    for (cm = CMSG_FIRSTHDR(&utf.msg); cm; cm = CMSG_NXTHDR(&utf.msg, cm))
        if (SOL_SOCKET == cm->cmsg_level && SO_TIMESTAMPNS == cm->cmsg_type)
            memcpy(&T[0], (struct timespec *)CMSG_DATA(cm), sizeof(struct timespec));

    T_int[0] = T[0].tv_sec + comps_sec; 
    T_int[1] = T[0].tv_nsec + comps_nsec;

    if(T_int[1] >= 1000000000){
        T_int[0] += 1;
        T_int[1] = T_int[1] % 1000000000;
    }

    if(T_int[1] < 0){
        T_int[0] -= 1;
        T_int[1] = 1000000000 + T_int[1];
    }

    nanosleep(&s, NULL);

    clock_gettime(CLOCK_REALTIME, &T[1]);

    T_int[2] = T[1].tv_sec + comps_sec; 
    T_int[3] = T[1].tv_nsec + comps_nsec;

    if(T_int[3] >= 1000000000){
        T_int[2] += 1;
        T_int[3] = T_int[3] % 1000000000;
    }

    if(T_int[3] < 0){
        T_int[2] -= 1;
        T_int[3] = 1000000000 + T_int[3];
    }

    sendto(utf.sock, T_int, sizeof(T_int), 0, (struct sockaddr*)&utf.from_addr, sizeof(utf.from_addr));

    printf("\nT2: %ld.%ld\nT3: %ld.%ld\n\n", T[0].tv_sec, T[0].tv_nsec, T[1].tv_sec, T[1].tv_nsec); // time_t (long) : %ld long: %ld

    printf("\nT2: %ld.%ld\nT3: %ld.%ld\n\n", T_int[0], T_int[1], T_int[2], T_int[3]); // time_t (long) : %ld long: %ld

    printf("Compenstate_FC_MC : %lfms, %ds, %dns\n", Compenstate_FC_MC, comps_sec, comps_nsec);

    return 0;

}

void *UDP_FC_COMPS_Thread(void *arg){

    int sock = (int *)arg;

    struct sockaddr_in client_addr;

    int len = sizeof(client_addr), count = 0, i;
    
    int64_t temp[2];

    int64_t *fc_comps_buf=(int64_t *)malloc(sizeof(int64_t) * 15); //동적할당

    int sync_during_ignored = 5;

    int count_bound = 10;

    char buf[256];

    while(recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr*)&client_addr, &len) > 0){

        printf("%d : %lld\n", count, _atoi(buf));

        fc_comps_buf[count] = _atoi(buf);

        count++;

        memset(buf, '\0', sizeof(buf));

        if(count > count_bound - 1){

            chage_comps = 1;

            for(i = sync_during_ignored + 1; i < count_bound; i++){
                temp[0] = fc_comps_buf[sync_during_ignored]; // max
                temp[1] = fc_comps_buf[sync_during_ignored]; // min

                if(fc_comps_buf[i] > temp[0])
                    temp[0] = fc_comps_buf[i];

                if(fc_comps_buf[i] < temp[1])
                    temp[1] = fc_comps_buf[i];
            }

            if((temp[0] + temp[1]) / 2.0 > 5 || (temp[0] + temp[1]) / 2.0 < -5){ // 5 아래는 무시하겠다

                Compenstate_FC_MC += (temp[0] + temp[1]) / 2.0; // 오프셋이 축척되는 것이 문제임
            
            }

            printf("max : %lld, min : %lld\nCompenstate_FC_MC : %lf\n", temp[0], temp[1], Compenstate_FC_MC);

            memset(fc_comps_buf, '\0', sizeof(fc_comps_buf));

            count = 0;

        }

        chage_comps = 0;

    }

    free(fc_comps_buf);

    close(sock);

    return 0;

}

int TCP_server(struct sockaddr_in *server_addr){

    pthread_t p_thread; // thread identifier

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

        if(pthread_create(&p_thread, NULL, Server_Socket_Thread, (void *)new) == 0) Thread_t++; // thread success return 0 pthread overlap is ok but index -1 is not ok

        pthread_detach(p_thread); // 자원 반납

        printf("%d : Client IP : %s Port : %d\n", Thread_t, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }

    close(sock);

    return 0;
}

int UDP_server(struct sockaddr_in *server_addr){

    udp_thread_factor utf;

    pthread_t p_thread; // thread identifier

    int fc_comps_sock, enabled = 1;

    struct sockaddr_in fc_comps_server_addr;

    memset(&fc_comps_server_addr, '\0', sizeof(fc_comps_server_addr));

    fc_comps_server_addr.sin_family = AF_INET;
    fc_comps_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    fc_comps_server_addr.sin_port = htons(FC_COMPS_PORT_UDP);

    if((utf.sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        printf("UDP socket() failed\n");
        exit(1);
    }

    if((fc_comps_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        printf("fc_comp_sock UDP socket() failed\n");
        exit(1);
    }

    printf("UDP socket() success\n "); // 이 부분에서 끝에 띄어쓰기가 없으면 segmentation 오류 발생

    if(setsockopt(utf.sock, SOL_SOCKET, SO_TIMESTAMPNS, &enabled, sizeof(enabled)) < 0)
        err("setsockopt()");

    if(setsockopt(fc_comps_sock, SOL_SOCKET, SO_TIMESTAMPNS, &enabled, sizeof(enabled)) < 0)
        err("fc_comps_setsockopt()");

    if(bind(utf.sock, (struct sockaddr*)server_addr, sizeof(*server_addr)) < 0)
        err("bind()");

    if(bind(fc_comps_sock, (struct sockaddr*)&fc_comps_server_addr, sizeof(fc_comps_server_addr)) < 0)
        err("fc_comps_bind()");

    printf("bind() success\n\n");

    if(pthread_create(&p_thread, NULL, UDP_FC_COMPS_Thread, (void *)fc_comps_sock) != 0)
            err("thread error");

    pthread_detach(p_thread); // 자원 반납

    while(recv_socket(utf.sock, &utf.msg, &utf.from_addr) > 0){

        if(pthread_create(&p_thread, NULL, UDP_Thread, (void *)&utf) != 0)
            err("thread error");
            
        pthread_detach(p_thread);  // 자원 반납
    }

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
