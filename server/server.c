#include "server.h"

/* time_t (long) sec long nsec for sleep */
struct timespec s = { MEDIUM_TERM_SEC, MEDIUM_TERM_NSEC };

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

void UDP_Function(void *args){
    
    udp_thread_factor utf = *((udp_thread_factor*)args);

    /* printpacket */
    struct cmsghdr *cm;

    struct timespec T[2];

    int32_t T_int[4]; // for compatiblility between 32bit and 64bit

    /* recv ms measure */
    int32_t comps_sec = (int32_t)(utf.Compenstate_FC_MC / 1000);
    int32_t comps_nsec = (int32_t)((utf.Compenstate_FC_MC - (int64_t)(utf.Compenstate_FC_MC / 1000) * 1000) * 1000000);

    printf("\nClient IP : %s Port : %d\n", inet_ntoa(utf.from_addr.sin_addr), ntohs(utf.from_addr.sin_port));

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

    printf("T2: %ld.%ld\nT3: %ld.%ld\nCompenstate_FC_MC : %.0lfms\n", T[0].tv_sec, T[0].tv_nsec, T[1].tv_sec, T[1].tv_nsec, utf.Compenstate_FC_MC); // time_t (long) : %ld long: %ld

    // printf("Compenstate_FC_MC : %lfms, %ds, %dns\n", utf.Compenstate_FC_MC, comps_sec, comps_nsec);

    return 0;
}

void UDP_FC_COMPS_Fuction(void *args, fc_offset *fc, char *buf, double *Compenstate_FC_MC, struct timespec *interval_start){

    udp_thread_factor utf = *((udp_thread_factor*)args);
    
    int i;

    double tmp = 0, tmp2;

    int64_t offset_tmp = _atoi(buf);

    printf("(count, fc offset) : (%d, %lldms)\n", fc->count+1, offset_tmp);
    printf("--------------------------------------------------------\n");

    fc->fc_comps_buf[fc->count] = offset_tmp;
    fc->count++;

    if(fc->count >= fc->count_bound){

        /* find min and max values to compute mean value of fc offset */
        fc->max = fc->fc_comps_buf[fc->sync_during_ignored];
        fc->min = fc->fc_comps_buf[fc->sync_during_ignored];

        tmp2 = fc->fc_comps_buf[fc->sync_during_ignored];

        for(i = fc->sync_during_ignored + 1; i < fc->count_bound; i++){
            
            /* max */
            if(fc->fc_comps_buf[i] > fc->max)
                fc->max = fc->fc_comps_buf[i];
                
            /* min */
            if(fc->fc_comps_buf[i] < fc->min)
                fc->min = fc->fc_comps_buf[i];

            /* mean */
            tmp2 += fc->fc_comps_buf[i];
            
        }

        /* mean */
        tmp = (fc->max + fc->min) / 2.0;
        tmp2 /= (fc->count_bound - fc->sync_during_ignored);

        /* Ignore below 5ms */
        if(fabs(tmp2) > 5)
            /* need much consdiration */
            *Compenstate_FC_MC += tmp2;

        /* 5min 초과 시 0으로 초기화 */
        if(fabs(*Compenstate_FC_MC) >= 300000)
            *Compenstate_FC_MC = 0;

        printf("\n--------------------------------------------------------\n");
        printf("(max, min, mean) : (%lldms, %lldms, %.1lfms)\nCompenstate_FC_MC : %.1lfms\n", fc->max, fc->min, tmp2, *Compenstate_FC_MC);
        printf("--------------------------------------------------------\n");
        
        memset(fc->fc_comps_buf, '\0', sizeof(fc->fc_comps_buf));

        fc->count = 0;

        /* FC sync minimum interval */
        clock_gettime(CLOCK_REALTIME, interval_start);

    }

}

int UDP_server(struct sockaddr_in *server_addr){

    fc_offset fc = {0, 0, {0}, 0, 30, 7};

    int enabled = 1;

    char data[256];

    pthread_t p_thread; // thread identifier

    udp_thread_factor utf = {0,};

    struct timespec interval_start = {0,}, interval_end = {0,};
    
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

        /* same return 0 */
        if(!strcmp("offset", data))
            UDP_Function((void *)&utf);
        else{
            clock_gettime(CLOCK_REALTIME, &interval_end);
            if(interval_end.tv_sec - interval_start.tv_sec > MINIMUM_INTERVAL_SEC){
                printf("\n--------------------------------------------------------\n");
                printf("(end, start, interval) : (%ld, %ld, %ld)\n",
                interval_end.tv_sec, interval_start.tv_sec, interval_end.tv_sec - interval_start.tv_sec);
                UDP_FC_COMPS_Fuction((void *)&utf, &fc, data, &utf.Compenstate_FC_MC, &interval_start);
            }
        }
    }

    return 0;
}