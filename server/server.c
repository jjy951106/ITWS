#include "server.h"

/* time_t (long) sec long nsec for sleep */
struct timespec s = { MEDIUM_TERM_SEC, MEDIUM_TERM_NSEC };

/* total thread number played to use in TCP protocol */
int Thread_t = 0;

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

    while((close_ = recv_socket(sock, &msg, NULL)) != -1){

        if(close_ == 0) break; // close client socket recv return 0
 
        clock_gettime(CLOCK_REALTIME, &T[0]);

        for (cm = CMSG_FIRSTHDR(&msg); cm; cm = CMSG_NXTHDR(&msg, cm))
            if (SOL_SOCKET == cm->cmsg_level && SO_TIMESTAMPNS == cm->cmsg_type)
                memcpy(&T[0], (struct timespec *)CMSG_DATA(cm), sizeof(struct timespec));

        T_int[0] = T[0].tv_sec; 
        T_int[1] = T[0].tv_nsec;

        nanosleep(&s, NULL);

        clock_gettime(CLOCK_REALTIME, &T[1]);

        T_int[2] = T[1].tv_sec;
        T_int[3] = T[1].tv_nsec;

        send(sock, T_int, sizeof(T_int), 0);

        printf("%d\nT2: %ld.%ld\nT3: %ld.%ld\n\n", ++n, T[0].tv_sec, T[0].tv_nsec, T[1].tv_sec, T[1].tv_nsec); // time_t (long) : %ld long: %ld

    }

    Thread_t--; // Thread crash caution

    close(sock);

    return 0;
}

void UDP_Function(void *args){
    
    udp_thread_factor utf = *((udp_thread_factor*)args);

    /* printpacket */
    struct cmsghdr *cm;

    struct timespec T[2];

    int32_t T_int[4]; // for compatiblility between 32bit and 64bit

    /* recv ms measure */
    int32_t comps_sec = (int32_t)(utf.Compenstate_FC_MC / 1000);
    int32_t comps_nsec = (int32_t)((utf.Compenstate_FC_MC - (int64_t)(utf.Compenstate_FC_MC / 1000) * 1000) * 1000000);

    printf("Client IP : %s Port : %d\n", inet_ntoa(utf.from_addr.sin_addr), ntohs(utf.from_addr.sin_port));

    /* T2 : T[0] server
       T3 : T[1] server */

    clock_gettime(CLOCK_REALTIME, &T[0]);

    for (cm = CMSG_FIRSTHDR(&utf.msg); cm; cm = CMSG_NXTHDR(&utf.msg, cm))
        if (SOL_SOCKET == cm->cmsg_level && SO_TIMESTAMPNS == cm->cmsg_type)
            memcpy(&T[0], (struct timespec *)CMSG_DATA(cm), sizeof(struct timespec));

    /* add compensation time */
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

    printf("\nT2: %ld.%ld\nT3: %ld.%ld\n\nCompenstate_FC_MC : %lfms, %ds, %dns\n", T[0].tv_sec, T[0].tv_nsec, T[1].tv_sec, T[1].tv_nsec, utf.Compenstate_FC_MC, comps_sec, comps_nsec); // time_t (long) : %ld long: %ld

    // printf("Compenstate_FC_MC : %lfms, %ds, %dns\n", utf.Compenstate_FC_MC, comps_sec, comps_nsec);

    return 0;
}

void *UDP_Thread(void *args){

    udp_thread_factor utf = *((udp_thread_factor*)args);
    
    /* printpacket */
    struct cmsghdr *cm;

    struct timespec T[2];

    int32_t T_int[4]; // for compatiblility between 32bit and 64bit

    /* recv ms measure */
    int32_t comps_sec = (int32_t)(utf.Compenstate_FC_MC / 1000);
    int32_t comps_nsec = (int32_t)((utf.Compenstate_FC_MC - (int64_t)(utf.Compenstate_FC_MC / 1000) * 1000) * 1000000);

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

    printf("\nT2: %ld.%ld\nT3: %ld.%ld\n\nCompenstate_FC_MC : %lfms, %ds, %dns\n", T[0].tv_sec, T[0].tv_nsec, T[1].tv_sec, T[1].tv_nsec, utf.Compenstate_FC_MC, comps_sec, comps_nsec); // time_t (long) : %ld long: %ld

    // printf("Compenstate_FC_MC : %lfms, %ds, %dns\n", utf.Compenstate_FC_MC, comps_sec, comps_nsec);

    return 0;

}

void UDP_FC_COMPS_Fuction(void *args, fc_offset *fc, char *buf, double *Compenstate_FC_MC){

    udp_thread_factor utf = *((udp_thread_factor*)args);
    
    int i;

    double tmp = 0, tmp2 = 0;

    int64_t offset_tmp = _atoi(buf);

    printf("%d : %lld\n", fc->count, offset_tmp);

    /* below 1s */
    if(offset_tmp < 1000){

        fc->fc_comps_buf[fc->count] = offset_tmp;

        fc->count++;
    
    }

    if(fc->count >= fc->count_bound){

        /* find min and max values to compute mean value of fc offset */
        fc->max = fc->fc_comps_buf[fc->sync_during_ignored];
        fc->min = fc->fc_comps_buf[fc->sync_during_ignored];

        for(i = fc->sync_during_ignored + 1; i < fc->count_bound; i++){
            
            /* max */
            //if(fc->fc_comps_buf[i] > fc->max)
            //    fc->max = fc->fc_comps_buf[i];
                
            /* min */
            //if(fc->fc_comps_buf[i] < fc->min)
            //    fc->min = fc->fc_comps_buf[i];

            /* mean */
            tmp2 += fc->fc_comps_buf[i];
            
        }

        /* mean */
        //tmp = (fc->max + fc->min) / 2.0;
        tmp2 /= (fc->count_bound - fc->sync_during_ignored);
        printf("%d\n", abs(utf.Compenstate_FC_MC - tmp2));

        if(abs(tmp2) >= 1000)
            *Compenstate_FC_MC = 0;

        /* Ignore below 5ms && The difference from the previous value must be more than 5*/
        if(abs(tmp2) > 5 && abs(utf.Compenstate_FC_MC - tmp2) > 5)
            /* need much consdiration */
            *Compenstate_FC_MC += tmp2;

        printf("max : %lld, min : %lld\nCompenstate_FC_MC : %lf\n", fc->max, fc->min, *Compenstate_FC_MC);

        memset(fc->fc_comps_buf, '\0', sizeof(fc->fc_comps_buf));

        fc->count = 0;

    }

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

    fc_offset fc = {0, 0, {0}, 0, 30, 7};

    int enabled = 1;

    char data[256];

    pthread_t p_thread; // thread identifier

    udp_thread_factor utf = {0,};
    
    if((utf.sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        printf("UDP socket() failed\n");
        exit(1);
    }

    printf(" UDP socket() success\n ");

    if(setsockopt(utf.sock, SOL_SOCKET, SO_TIMESTAMPNS, &enabled, sizeof(enabled)) < 0)
        err("setsockopt()");

    if(bind(utf.sock, (struct sockaddr*)server_addr, sizeof(*server_addr)) < 0)
        err("bind()");

    printf("bind() success\n\n");

    /* msg data initialization should be needed */
    sendto(utf.sock, data, sizeof(data), 0, (struct sockaddr*)server_addr, sizeof(*server_addr));

    while(recv_socket(utf.sock, &utf.msg, &utf.from_addr) > 0){
        
        strcpy(data, utf.msg.msg_iov->iov_base);

        if(!strcmp("offset", data)){ // same return 0

            /* Thread
            if(pthread_create(&p_thread, NULL, UDP_Thread, (void *)&utf) != 0)
                err("thread error");

            // return resource 
            pthread_detach(p_thread); */

            /* Function */
            UDP_Function((void *)&utf);
        }
        else
            UDP_FC_COMPS_Fuction((void *)&utf, &fc, data, &utf.Compenstate_FC_MC);

    }

    return 0;
}