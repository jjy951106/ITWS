/* window */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h>

#define PORT 5005

int main() {

	WSADATA wsaDATA;
	SOCKET Server_Sock, Client_Sock;
	SOCKADDR_IN serverAddr, clientAddr;
	int len;

	char buf[1024];
	char* message = "Hellow World!";

	//printf("%d\n", INVALID_SOCKET); // INVALID_SOCKET == SOCKET_ERROR == -1

	if (WSAStartup(MAKEWORD(2, 2), &wsaDATA) == INVALID_SOCKET) { // ù ��° parameter �� ����� ������ 2.2 �����̶�� ���� �ǹ�
		printf("WSAStartup() failed : %d", WSAGetLastError());	  // �� ��° parameter �� WSADATA ����ü ������ �ּҰ��� �Ѱ��ָ� �ش� ������ �ʱ�ȭ �� ���̺귯�� ������ ä����
		WSACleanup();
		exit(1);
	}

	if ((Server_Sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("socket() failed : %d", WSAGetLastError);
		exit(1);
	}

	memset(&serverAddr, 0, sizeof(serverAddr)); // serverAddr�� �ּҸ� 0���� �ʱ�ȭ
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(PORT);

	if (bind(Server_Sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		printf("bind() failed : %d", WSAGetLastError);
		exit(1);
	}

	if (listen(Server_Sock, 5) == SOCKET_ERROR) {
		printf("listen() failed : %d", WSAGetLastError);
		exit(1);
	}

	len = sizeof(clientAddr);

	while (1) {
		if ((Client_Sock = accept(Server_Sock, (SOCKADDR*)&clientAddr, &len)) == INVALID_SOCKET) {	  // Server Socket���� listen�Ͽ� ����ϰ� �ִ� accept��û��
			printf("accept() failed : %d", WSAGetLastError);										  // accept �Լ��� �̿��Ͽ� Client_Sock�� �����ϴ� ��
			exit(1);																				  // �����δ� Client_Sock�� ���� ����ϰ� �� ���ο� socket call
		}																							  // Client_Sock�� ������ ��û�� Ŭ���̾�Ʈ�� �����ּҸ� ��ȯ
		send(Client_Sock, message, strlen(message) + 1, 0);
		recv(Client_Sock, buf, sizeof(buf), 0);
		printf("Client : %s\n", buf);
	}

	closesocket(Client_Sock);
	closesocket(Server_Sock);

	//printf("success! %d\n", Sock);
	
	WSACleanup(); // winsock ���� �� �ݵ�� WSACleanup�� ȣ��

	return 0;
}

/*int main() {

	char a[20];

	// 1����Ʈ���� ��� 65�� �ʱ�ȭ
	// �迭�� ä�� ���� sizeof()�Լ��� ����ϸ� �˴ϴ�.
	// sizeof �Լ� - �迭�� ��ü ����Ʈ ũ�⸦ ��ȯ�մϴ�.
	memset(a, 65, sizeof(a));

	// ����� ���� Ȯ��
	for (int i = 0; i < (sizeof(a) / sizeof(char)); i++) {
		printf("%c\n", a[i]);
	}

}*/