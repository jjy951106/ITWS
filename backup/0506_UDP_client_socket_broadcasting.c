//CLIENT SOCKET
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#define PORTNUM 9005

int main(void) {
    char buf[256];
    struct sockaddr_in sin, brd;
    struct timeval tv_cli;
    int sd, n, i = 0, sinlen = sizeof(sin), broadcast = 1;
    int loc_time[1000], loc_utime[1000];

    if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){ //socket�� ����(���� ������ 0�� ���� ���н� -1�� ����)
        perror("socket"); //�����޽����� ����ϴ� �Լ�
        exit(1); //1�� ��ȯ�ϸ鼭 ���α׷� ����
    }

    if ((setsockopt(sd , SOL_SOCKET, SO_BROADCAST, (const char *)&broadcast,sizeof(broadcast))) == -1){ 
        perror("setsockopt");
        exit(1);
    }

    memset((char *)&sin, '\0', sizeof(sin)); //socket ����ü�� ���� ����(&ser-�޸��� ���� �ּ�, \0-�޸𸮿� ä����� �ϴ� ��, size-ä������ϴ� �޸��� ũ��)
    brd.sin_family = AF_INET; //socket family�� AF_INET���� ����
    brd.sin_port = htons(PORTNUM);
    brd.sin_addr.s_addr = htonl(INADDR_ANY); //���� �ּ� ����ü�� ������ �ּҸ� ����

    if (bind(sd, (struct sockaddr *)&brd, sizeof(brd))) { //17�࿡�� ������ ������ bind �Լ��� 22~25�࿡�� ������ IP, port ��ȣ�� ����(���н� -1�� ���� ������ 0�� ����)(sd-socket���� ������ ����ü, &ser-AF_INET�� ��� sockaddr_in AF_UNIX�� ��� sockaddr, len-������ ����ü�� ũ��)
        perror("bind");
        exit(1);
    }

    strcpy(buf, "I am a client.");

//    if ((sendto(sd, buf, strlen(buf)+1, 0, (struct sockaddr *)&sin, sizeof(sin))) == -1) { //Ŭ���̾�Ʈ���� sendto �Լ��� msg�� ����(sd, buf-������ msg�� ������ �޸� �ּ�, msg�� ũ��, 0-�����͸� �ְ�޴� ����� ������ �÷���, &cli-msg�� ������ ȣ��Ʈ�� �ּ�, &clientlen-&cli�� ũ��)
//            perror("sendto");
//            exit(1);
//        }

    memset((int *)&loc_time, '\0', sizeof(loc_time));
    memset((int *)&loc_utime, '\0', sizeof(loc_utime));

    while (1) {

        n = recvfrom(sd, buf, 255, 0, (struct sockaddr *)&sin, &sinlen); //������ ���� msg�� recvfrom �Լ��� ����(sd, buf-���۹��� msg�� ������ �޸� �ּ�, msg�� ũ��, 0-�����͸� �ְ�޴� ����� ������ �÷���, msg�� ������ ������ �ּ�, �ּ��� ũ��)

        if ((ioctl(sd, SIOCGSTAMP, &tv_cli)) == -1){
            perror("ioctl");
            exit(1);
        }

        loc_time[i] = tv_cli.tv_sec;
        loc_utime[i] = tv_cli.tv_usec;
        buf[n] = '\0';

        printf("** From Server : %s, %i packet\n", buf, i+1);
        printf("** Local clock : %i.%i\n", loc_time[i], loc_utime[i]);

        i++;
    }

    return 0;
}