/* linux */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h> // socket �Լ��� ����ϱ� ���� ���� ������ ���Ե� ���� unix linux
                        // �������� window ������ winsock, winsock2 ������Ϸ� ����

#include <netinet/in.h>
#include <arpa/inet.h>  // inet_addr(), inet_ntoa() �Լ��� ���� ����  32bit�� ip adress�� 10���� <-> 2������ ��ȯ

#define IP "210.107.214.170"
#define PORT 5005

int main(){

    int client_sock;
    struct sockaddr_in server_addr;
    char buf[1024];
    char *message = "Hellow me too!";

    if((client_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){ // ���� �� -1 �� ���� ��ũ���ͷ� ��ȯ
        printf("socket call error!\n");
        exit(1);                                        // error ���� ȣ���� �Լ��� ����� �͵� ���
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(IP); // htonl ����
    server_addr.sin_port = htons(PORT); // htons ����

    printf("1, server_addr.sin_port : %d\n",server_addr.sin_port);

    if(connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)))
        exit(1);

    printf("2, server_addr.sin_port : %d\n",server_addr.sin_port);

    if(recv(client_sock, buf, sizeof(buf), 0) == -1)
        exit(1);

    if(send(client_sock, message, strlen(message) + 1, 0) == -1)
        exit(1);

    printf("Server : %s\n", buf);

    close(client_sock);

    return 0;
}