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
        caddr_t msg_name;       // �ּ�(�ɼ�)
        u_int   msg_namelen;    // �ּ��� ������
        struct  iovec *msg_iov; // ��ĳŸ/���� �迭
        u_int   msg_iovlen;     // msg_iov �� ��Ҽ�
        caddr_t msg_control;    // ���� ������, �ļ�
        u_int   msg_controllen; // ���� �������� ������
        int     msg_flags;      // ���ŵ� �޼������� �÷���
};

struct cmsghdr {
        u_int   cmsg_len;       // ������ ����Ʈ ī��Ʈ, hdr �� �����Ѵ�
        int     cmsg_level;     // �޼����� ������ ��������
        int     cmsg_type;      // �������ݿ� ������ Ÿ��
        // u_char  cmsg_data[]; �� �Ŀ� ��ӵȴ�
};
*/