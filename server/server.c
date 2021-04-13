#include "server.h"

/* time_t (long) sec long nsec for sleep */
struct timespec s = { MEDIUM_TERM_SEC, MEDIUM_TERM_NSEC };

/* total thread number played to use in TCP protocol */
int Thread_t = 0;

/* Offset between FC and MC */
double Compenstate_FC_MC = 0;

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

    /* T2 : T[0] server
       T3 : T[1] server */

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
    else if(T_int[1] < 0){
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
    else if(T_int[3] < 0){
        T_int[2] -= 1;
        T_int[3] = 1000000000 + T_int[3];
    }

    sendto(utf.sock, T_int, sizeof(T_int), 0, (struct sockaddr*)&utf.from_addr, sizeof(utf.from_addr));

    printf("\nT2: %ld.%ld\nT3: %ld.%ld\n\n", T[0].tv_sec, T[0].tv_nsec, T[1].tv_sec, T[1].tv_nsec); // time_t (long) : %ld long: %ld

    printf("Compenstate_FC_MC : %lfms, %ds, %dns\n", Compenstate_FC_MC, comps_sec, comps_nsec);

    return 0;

}

void *UDP_FC_COMPS_Thread(void *arg){

    int sock = (int *)arg;

    struct sockaddr_in client_addr;

    int len = sizeof(client_addr), count = 0, tmp = 0, i;
    
    int64_t max, min;

    int64_t *fc_comps_buf=(int64_t *)malloc(sizeof(int64_t) * 30); //동적할당

    /* Ignore the first 5 */
    int sync_during_ignored = 5;

    int count_bound = 15;

    char buf[256];

    while(recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr*)&client_addr, &len) > 0){

        printf("%d : %lld\n", count, _atoi(buf));

        fc_comps_buf[count] = _atoi(buf);

        count++;

        memset(buf, '\0', sizeof(buf));

        if(count >= count_bound){

            for(i = sync_during_ignored + 1; i < count_bound; i++){
        
                max = fc_comps_buf[sync_during_ignored];
                min = fc_comps_buf[sync_during_ignored];
                
                /* max */
                if(fc_comps_buf[i] > temp[0])
                    max = fc_comps_buf[i];
                
                /* min */
                if(fc_comps_buf[i] < temp[1])
                    min = fc_comps_buf[i];
            
            }

            tmp = (max + min) / 2.0;

            /* Ignore below 5ms */
            if(abs(tmp) > 5)
                /* need much consdiration */
                Compenstate_FC_MC += tmp;

            printf("max : %lld, min : %lld\nCompenstate_FC_MC : %lf\n", temp[0], temp[1], Compenstate_FC_MC);

            memset(fc_comps_buf, '\0', sizeof(fc_comps_buf));

            count = 0;

        }

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

        pthread_detach(p_thread);

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
        printf("UDP socket() failed\n");
        exit(1);
    }

    printf(" UDP socket() success\n ");

    if(setsockopt(utf.sock, SOL_SOCKET, SO_TIMESTAMPNS, &enabled, sizeof(enabled)) < 0)
        err("setsockopt()");

    if(bind(utf.sock, (struct sockaddr*)server_addr, sizeof(*server_addr)) < 0)
        err("bind()");

    if(setsockopt(fc_comps_sock, SOL_SOCKET, SO_TIMESTAMPNS, &enabled, sizeof(enabled)) < 0)
        err("fc_comps_setsockopt()");

    if(bind(fc_comps_sock, (struct sockaddr*)&fc_comps_server_addr, sizeof(fc_comps_server_addr)) < 0)
        err("fc_comps_bind()");

    printf("bind() success\n\n");

    /* FC offset compansation */
    if(pthread_create(&p_thread, NULL, UDP_FC_COMPS_Thread, (void *)fc_comps_sock) != 0)
            err("thread error");
    
    /* return resource */
    pthread_detach(p_thread);

    while(recv_socket(utf.sock, &utf.msg, &utf.from_addr) > 0){

        if(pthread_create(&p_thread, NULL, UDP_Thread, (void *)&utf) != 0)
            err("thread error");
        
        /* return resource */
        pthread_detach(p_thread);
    }

    return 0;
}