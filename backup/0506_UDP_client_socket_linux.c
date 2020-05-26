#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <sys/time.h>

#include <netdb.h>        //
#include <asm/sockios.h>  //

#include <sys/types.h>
#include <sys/socket.h> // socket �Լ��� ����ϱ� ���� ���� ������ ���Ե� ���� unix linux
                        // �������� window ������ winsock, winsock2 ������Ϸ� ����

#include <netinet/in.h>
#include <arpa/inet.h>  // inet_addr(), inet_ntoa() �Լ��� ���� ����  32bit�� ip adress�� 10���� <-> 2������ ��ȯ arpa/inet.h ���� inet_addr ���

#define SERVER "192.168.0.158"
#define PORT 5005

int main(){

    int client_sock, ret, error, offset, tmp;
    struct sockaddr_in server_addr, client_addr;
    char buf[1024], temp[50], send_Time[50];

    struct timeval tv_server, tv_ioctl, tv_client, current;

    int initial = 0;

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER); // htonl ����
    server_addr.sin_port = htons(PORT); // htons ����

    if((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){ // ���� �� -1 �� ���� ��ũ���ͷ� ��ȯ
        printf("socket call error!\n");
        exit(1);                                              // error ���� ȣ���� �Լ��� ����� �͵� ���
    }

    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY); // htonl ����
    client_addr.sin_port = htons(0); // htons ����

    if(bind(client_sock, (struct sockaddr*)&client_addr, sizeof(client_addr)))
        exit(1);

    gettimeofday(&tv_client, NULL);  // client sendtime T1

    sprintf(send_Time, "%d.%d", tv_client.tv_sec, tv_client.tv_usec);

    sendto(client_sock, send_Time, strlen(send_Time) + 1, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

    recvfrom(client_sock, buf, sizeof(buf), 0, (struct sockaddr*)0, (int*)0);

    printf("server_send_time: %s\n", buf);

    //gettimeofday(&tv_ioctl, NULL);
    error = ioctl(client_sock, SIOCGSTAMP, &tv_ioctl); // client receivetime T4

    tv_server.tv_sec = atoi(strtok(buf, "."));

    tv_server.tv_usec = atoi(strtok(NULL, "")); // server sendtime T2 T3

    if(!initial){
        settimeofday(&tv_server, NULL);
        initial++;
    }

    offset = (((tv_server.tv_sec - tv_client.tv_sec) * 1000000 + tv_server.tv_usec - tv_client.tv_usec) -
        ((tv_ioctl.tv_sec - tv_server.tv_sec) * 1000000 + tv_ioctl.tv_usec - tv_server.tv_usec)) / 2;

    printf("offset: %d usec\n", offset); // ((T2 - T1) - (T4 - T3)) / 2

    gettimeofday(&current, NULL);

    printf("%d.%d\n", current.tv_sec, current.tv_usec);

    if((tmp = current.tv_usec + offset) > 1000000){
        current.tv_sec += 1;
        current.tv_usec = tmp - 1000000;
    }

    else
        current.tv_usec = tmp;

    settimeofday(&current, NULL);

    printf("%d.%d\n", current.tv_sec, current.tv_usec);

    close(client_sock);

    return 0;
}