#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h>

#define PORT 5005

int main() {

	WSADATA wsaDATA;
	SOCKET Server_Sock;
	SOCKADDR_IN serverAddr, clientAddr;
	int client_len = sizeof(clientAddr); // sizeof(client_len)�� �������� clientAddr �� ����
	int buf_len;

	char buf[1024];
	char* message = "Hellow World!";

	//printf("%d\n", INVALID_SOCKET); // INVALID_SOCKET == SOCKET_ERROR == -1

	if (WSAStartup(MAKEWORD(2, 2), &wsaDATA) == INVALID_SOCKET) { // ù ��° parameter �� ����� ������ 2.2 �����̶�� ���� �ǹ�
		printf("WSAStartup() failed : %d", WSAGetLastError());	  // �� ��° parameter �� WSADATA ����ü ������ �ּҰ��� �Ѱ��ָ� �ش� ������ �ʱ�ȭ �� ���̺귯�� ������ ä����
		WSACleanup();
		exit(1);
	}

	if ((Server_Sock = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
		printf("socket() failed : %d", WSAGetLastError);
		exit(1);
	}

	memset(&serverAddr, 0, sizeof(serverAddr)); // serverAddr�� 0���� �ʱ�ȭ
	memset(buf, 0, sizeof(buf));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(PORT);

	if (bind(Server_Sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		printf("bind() failed : %d", WSAGetLastError);
		exit(1);
	}

	while (1) {
		buf_len = recvfrom(Server_Sock, buf, 1025, 0, (SOCKADDR*)&clientAddr, &client_len);
		if (buf_len > 0) printf("Client : %s\n", buf);
		sendto(Server_Sock, message, strlen(message) + 1, 0, (SOCKADDR*)&clientAddr, sizeof(clientAddr));
	}

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