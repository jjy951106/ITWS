//SERVER SOCKET
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PORTNUM 9005

int main(void) {
    char buf[256];
    struct sockaddr_in sin, cli;
    float beacon_interval = 0.001;
    int sd, clientlen = sizeof(cli), broadcast = 1; // 1���� broadcast option

    if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){ //socket�� ����(���� ������ 0�� ���� ���н� -1�� ����)
        perror("socket"); //�����޽����� ����ϴ� �Լ�
        exit(1); //1�� ��ȯ�ϸ鼭 ���α׷� ����
    }

    if ((setsockopt(sd , SOL_SOCKET, SO_BROADCAST, (const char *)&broadcast , sizeof(broadcast))) == -1){
        perror("setsockopt"); //�����޽����� ����ϴ� �Լ�
        exit(1); //1�� ��ȯ�ϸ鼭 ���α׷� ����
    }

    memset((char *)&sin, '\0', sizeof(sin)); //socket ����ü�� ���� ����(&ser-�޸��� ���� �ּ�, \0-�޸𸮿� ä����� �ϴ� ��, size-ä������ϴ� �޸��� ũ��)
    sin.sin_family = AF_INET; //socket family�� AF_INET���� ����
    sin.sin_port = htons(PORTNUM);
    sin.sin_addr.s_addr = inet_addr("192.168.0.255"); //���� �ּ� ����ü�� ������ �ּҸ� ����

    if (bind(sd, (struct sockaddr *)&sin, sizeof(sin))) { //17�࿡�� ������ ������ bind �Լ��� 22~25�࿡�� ������ IP, port ��ȣ�� ����(���н� -1�� ���� ������ 0�� ����)(sd-socket���� ������ ����ü, &ser-AF_INET�� ��� sockaddr_in AF_UNIX�� ��� sockaddr, len-������ ����ü�� ũ��)
        perror("bind");
        exit(1);
    }

    for (int n = 1; n <= 1000; n++) {
//        if ((recvfrom(sd, buf, 255, 0, (struct sockaddr *)&cli, &clientlen)) == -1) { //Ŭ���̾�Ʈ�� ���� msg�� recvfrom �Լ��� ����(sd, buf-���۹��� msg�� ������ �޸� �ּ�, msg�� ũ��, 0-�����͸� �ְ�޴� ����� ������ �÷���, &cli-msg�� ������ ȣ��Ʈ�� �ּ�, &clientlen-&cli�� ũ��)
//            perror("recvfrom");
//            exit(1);
//        }

//        printf("** From Client : %s\n", buf);
        strcpy(buf, "Hello Client");

        if ((sendto(sd, buf, strlen(buf)+1, 0, (struct sockaddr *)&sin, sizeof(sin))) == -1) { //Ŭ���̾�Ʈ���� sendto �Լ��� msg�� ����(sd, buf-������ msg�� ������ �޸� �ּ�, msg�� ũ��, 0-�����͸� �ְ�޴� ����� ������ �÷���, &cli-msg�� ������ ȣ��Ʈ�� �ּ�, &clientlen-&cli�� ũ��)
            perror("sendto");
            exit(1);
        }

        printf("send to client %i packet\n", n);

        sleep(beacon_interval);

    }

    return 0;
}