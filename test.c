/* Single Thread */

#include <stdio.h>
#include <stdlib.h> // exit(0) 'normal' exit(1) 'error'
#include <string.h>
#include <stdint.h> // int32_t int64_t
#include <stdlib.h>

#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <pthread.h>
#include <unistd.h>  // sleep usleep
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER "192.168.0.160"
#define PORT 5005

int main(int argc, char *argv[]){

    int res ,sock;

    struct sockaddr_in server_addr;

    struct timespec T[2];

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("socket() failed\n");
        exit(1);
    }

    memset(&server_addr, '\0', sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER);
    server_addr.sin_port = htons(PORT);

    if(argc >= 2) server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    if(argc == 3) server_addr.sin_port = htons(atoi(argv[2]));

    if(connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        printf("connect() failed\n");
        exit(1);
    }

    printf("server_addr : %s, port : %d\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    /* recvpacket */

    clock_gettime(CLOCK_REALTIME, &T[0]);

    clock_gettime(CLOCK_REALTIME, &T[1]);

    printf("T1 : %ld.%09ld\n", T[0].tv_sec, T[0].tv_nsec);

    printf("T2 : %ld.%09ld\n", T[1].tv_sec, T[1].tv_nsec);

    //res = sendto(sock, T, sizeof(T), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
    res = send(sock, T, sizeof(T), 0);
    if(res < 0)
        printf("send error\n");


	return 0;
}

/*
struct msghdr {qq
        caddr_t msg_name;       // 주소(옵션)
        u_int   msg_namelen;    // 주소의 사이즈
        struct  iovec *msg_iov; // 스캐타/개더 배열
        u_int   msg_iovlen;     // msg_iov 의 요소수
        caddr_t msg_control;    // 보조 데이터, 후술
        u_int   msg_controllen; // 보조 데이터의 버퍼장
        int     msg_flags;      // 수신된 메세지상의 플래그
};

struct cmsghdr {
        u_int   cmsg_len;       // 데이터 바이트 카운트, hdr 를 포함한다
        int     cmsg_level;     // 메세지를 생성한 프로토콜
        int     cmsg_type;      // 프로토콜에 고유의 타입
        // u_char  cmsg_data[]; 가 후에 계속된다
};
*/