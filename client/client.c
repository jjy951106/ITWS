#include "client.h"

int32_t DEVIATION = 50000000; /* initial DEVIATION is 50ms */

struct timespec T_, T_present;

int32_t thr = 5000000; /* threshold default 5ms to use in mode_2 */

static const unsigned char binary[] = {
    "offset"
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

    /* If a packet is not received within a certain period of time, it is retransmitted */
    while(re == -1){

        /* T1 update */
        clock_gettime(CLOCK_REALTIME, T);
  
        /* retransmission */
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

    /* T3 server send time */
    T.tv_sec = T_int[2]; T.tv_nsec = T_int[3];

    clock_settime(CLOCK_REALTIME, &T);

}

void offset_calculated(int sock, int *offset, struct sockaddr_in *server_addr, int protocol, int *delay){

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

    /* T3 is saved to use in mode_1 and mode_3 */
    memcpy(&T_, &T[2], sizeof(struct timespec));

    for (cm = CMSG_FIRSTHDR(&msg); cm; cm = CMSG_NXTHDR(&msg, cm))
        if (SOL_SOCKET == cm->cmsg_level && SO_TIMESTAMPNS == cm->cmsg_type){
            //printf("SO_TIMESTAMPNS action\n");
            memcpy(&T[3], (struct timespec *)CMSG_DATA(cm), sizeof(struct timespec));
        }

    /* T4 is saved to use in mode_3 */
    memcpy(&T_present, &T[3], sizeof(struct timespec));

    /* T1 : T[0] client
       T2 : T[1] server
       T3 : T[2] server
       T4 : T[3] client */

    /* offset (+ or -) */
    offset[0] = ((T[1].tv_sec - T[0].tv_sec) - (T[3].tv_sec - T[2].tv_sec)) / 2;

    offset[1] = ((T[1].tv_nsec - T[0].tv_nsec) - (T[3].tv_nsec - T[2].tv_nsec)) / 2;

    if(delay != NULL){
        /* delay (+ or -) */
        delay[0] = ((T[1].tv_sec - T[0].tv_sec) + (T[3].tv_sec - T[2].tv_sec)) / 2;

        delay[1] = ((T[1].tv_nsec - T[0].tv_nsec) + (T[3].tv_nsec - T[2].tv_nsec)) / 2;
    }
}

void iterative_offset_calculated(int sock, int32_t *offset, struct sockaddr_in *server_addr, int protocol){

    int32_t temp[2] = { 0, };

    struct timespec s;

    int iter, iteration = 0;

    s.tv_sec = MEDIUM_TERM_SEC; s.tv_nsec = MEDIUM_TERM_NSEC;

    for(iter = 0; iter < ITERATION; iter++){

        offset_calculated(sock, temp, server_addr, protocol, NULL);

        if(temp[0] > 5 /* second (5s or 10s) */){
            initialized_T(sock, server_addr, protocol);
            return 0;
        }

        /* DEVIATION */
        if(abs(temp[0]) < 1 /* must be less than 1 second */ && abs(temp[1]) <= DEVIATION /* |ns| < DEVIATION */){
            /* accumulated offset */
            offset[0] += temp[0];
            offset[1] += temp[1];
            iteration++;
        }

        nanosleep(&s, NULL);
    }
    //printf("iteration : %d \n", iteration);

    /* minimum of iterations within adatative deviations is more than 5 in total 10 */
    if(iteration >= 3){ 
        offset[0] /= iteration;
        offset[1] /= iteration;
    }

    /* DEVIATION increasing and not sync */
    else{
        DEVIATION += 10000000; /* 10ms */
        offset[0] = 0;
        offset[1] = 0;
    }

    /* DEVIATION decreasing to increase accuracy */
    if(iteration >= 7 && iteration <= ITERATION) 
        DEVIATION -= 5000000;  /* 5ms */
}

void mode_1(int sock, struct sockaddr_in *server_addr, int protocol){

    int32_t tmp, offset[2];

    struct timespec C; // C (current)
    
    while(1){

        offset[0] = 0;
        offset[1] = 0;

        iterative_offset_calculated(sock, offset, server_addr, protocol);

        /* Not enough samples */
        if(offset[0] == 0 && offset[1] == 0)  
            continue;

        /* inner offset initialization algorithm */
        else if(abs(offset[0]) > 0){
            clock_settime(CLOCK_REALTIME, &T_);
            sleep(1);
            continue;
        }

        /* SUCCESS */
        else if(offset[1] < BOUNDARY && offset[1] > BOUNDARY_) 
            break;

        /* offset compensation */
        clock_gettime(CLOCK_REALTIME, &C);

        tmp = C.tv_nsec + offset[1];

        if(tmp >= 1000000000){
            C.tv_sec += 1;
            C.tv_nsec = tmp - 1000000000;
        }
        else if(tmp < 0){
            C.tv_sec -= 1;
            C.tv_nsec = tmp + 1000000000;
        }
        else
            C.tv_nsec = tmp;

        clock_settime(CLOCK_REALTIME, &C);

    }

}

void mode_2(int sock, struct sockaddr_in *server_addr, int protocol){

    int32_t offset[2], offset_check = 0;

    struct timespec C; // C (current)

    int i;

    initialized_T(sock, server_addr, protocol);

    while(1){

        offset[0] = 0;
        offset[1] = 0;

        /* need to add this code in python */
        /* It is related to synch problem that sleep term is more than 1.5 and iteration is pretty large as 10 */
        for(i=0; i<5; i++){ // test needed
            offset_calculated(sock, offset, server_addr, protocol, NULL);
            if(abs(offset[1]) <= BOUNDARY)
                break;
            sleep(0.7); // test needed
        }

        if(abs(offset[0]) > 1 || abs(offset[1]) > BOUNDARY) 
            mode_1(sock, server_addr, protocol);
    }

}

void mode_3(int sock, struct sockaddr_in *server_addr, int protocol){

    struct tm *server_date, *drone_date;

    int drone_ms, server_ms, i;

    int32_t tmp, offset[2] = { 0, }, delay[2] = { 0, };

    /* need to add this code in python */
    /* It is related to synch problem that sleep term is more than 1.5 and iteration is pretty large as 10 */
    for(i=0; i<10; i++){ // test needed
        offset_calculated(sock, offset, server_addr, protocol, delay);
        if(abs(offset[1]) <= BOUNDARY)
            break;
        sleep(0.5); // test needed
    }

    /* delay reward in T3 server */
    T_.tv_sec += delay[0];

    tmp = T_.tv_nsec + delay[1];

    if(tmp > 1000000000){
        T_.tv_sec += 1;
        T_.tv_nsec = tmp - 1000000000;
    }
    else if(tmp < 0){
        T_.tv_sec -= 1;
        T_.tv_nsec = tmp + 1000000000;
    }
    else
        T_.tv_nsec = tmp;

    /* print time */
    /* server_s.server_us+client.s+client.us+offset(ms)+ */

    /* not omit 0 (ex 10000us -> print 010000us) */
    if(T_.tv_nsec >= 100000000 /* 100,000,000 */)
        printf("%d.%d+", T_.tv_sec, T_.tv_nsec / 1000);

    else if(T_.tv_nsec >= 10000000 /* 10,000,000 */)
        printf("%d.0%d+", T_.tv_sec, T_.tv_nsec / 1000);

    else if(T_.tv_nsec >= 1000000 /* 1,000,000 */)
        printf("%d.00%d+", T_.tv_sec, T_.tv_nsec / 1000);

    else if(T_.tv_nsec >= 100000 /* 100,000 */)
        printf("%d.000%d+", T_.tv_sec, T_.tv_nsec / 1000);

    else if(T_.tv_nsec >= 10000 /* 10,000 */)
        printf("%d.0000%d+", T_.tv_sec, T_.tv_nsec / 1000);

    else if(T_.tv_nsec >= 1000 /* 1,000 */)
        printf("%d.00000%d+", T_.tv_sec, T_.tv_nsec / 1000);

    else if(T_.tv_nsec >= 100)
        printf("%d.000000%d+", T_.tv_sec, T_.tv_nsec / 1000);

    if(T_present.tv_nsec >= 100000000 /* 100,000,000 */)
        printf("%d.%d+", T_present.tv_sec, T_present.tv_nsec / 1000);

    else if(T_present.tv_nsec >= 10000000 /* 10,000,000 */)
        printf("%d.0%d+", T_present.tv_sec, T_present.tv_nsec / 1000);

    else if(T_present.tv_nsec >= 1000000 /* 1,000,000 */)
        printf("%d.00%d+", T_present.tv_sec, T_present.tv_nsec / 1000);

    else if(T_present.tv_nsec >= 100000 /* 100,000 */)
        printf("%d.000%d+", T_present.tv_sec, T_present.tv_nsec / 1000);

    else if(T_present.tv_nsec >= 10000 /* 10,000 */)
        printf("%d.0000%d+", T_present.tv_sec, T_present.tv_nsec / 1000);

    else if(T_present.tv_nsec >= 1000 /* 1,000 */)
        printf("%d.00000%d+", T_present.tv_sec, T_present.tv_nsec / 1000);

    else if(T_present.tv_nsec >= 100)
        printf("%d.000000%d+", T_present.tv_sec, T_present.tv_nsec / 1000);
    
    /* ms */
    printf("%d+", (offset[0] * 1000) + (offset[1] / 1000000));

}

int TCP_socket(struct sockaddr_in *server_addr, int mode, int protocol, int thr_tmp){

    int sock, enabled = 1;

    struct timeval tv;

    tv.tv_sec = 3;
    tv.tv_usec = 0;

    thr = thr_tmp;

    /* TCP */

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        err("TCP socket()");

    /* SO_TIMESTAMPNS */

    if(setsockopt(sock, SOL_SOCKET, SO_TIMESTAMPNS, &enabled, sizeof(enabled)) < 0)
        err("SO_TIMESTAMPNS setsockopt()");

    if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval)) < 0)
        err("SO_RCVTIMEO setsockopt()");

    if(connect(sock, (struct sockaddr*)server_addr, sizeof(*server_addr)) < 0)
        err("connect()");

    select_mode(sock, mode, server_addr, protocol);

    close(sock);

    return 0;

}

int UDP_socket(struct sockaddr_in *server_addr, int mode, int protocol, int thr_tmp){

    int sock, enabled = 1;

    struct timeval tv;

    tv.tv_sec = 3;
    tv.tv_usec = 0;

    thr = thr_tmp;

    struct sockaddr_in client_addr;

    int client_len = sizeof(client_addr);

    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    client_addr.sin_port = htons(0);

    /* UDP */

    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        err("UDP socket()");

    /* SO_TIMESTAMPNS */

    if(setsockopt(sock, SOL_SOCKET, SO_TIMESTAMPNS, &enabled, sizeof(enabled)) < 0)
        err("SO_TIMESTAMPNS setsockopt()");

    if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval)) < 0)
        err("SO_RCVTIMEO setsockopt()");

    if(bind(sock, (struct sockaddr*)&client_addr, client_len) < 0)
        err("bind()");

    select_mode(sock, mode, server_addr, protocol);

    close(sock);

    return 0;

}