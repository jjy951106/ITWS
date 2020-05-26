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

	if (WSAStartup(MAKEWORD(2, 2), &wsaDATA) == INVALID_SOCKET) { // 첫 번째 parameter 는 사용할 소켓이 2.2 버전이라는 것을 의미
		printf("WSAStartup() failed : %d", WSAGetLastError());	  // 두 번째 parameter 는 WSADATA 구조체 변수의 주소값을 넘겨주면 해당 변수에 초기화 된 라이브러리 정보가 채워짐
		WSACleanup();
		exit(1);
	}

	if ((Server_Sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("socket() failed : %d", WSAGetLastError);
		exit(1);
	}

	memset(&serverAddr, 0, sizeof(serverAddr)); // serverAddr의 주소를 0으로 초기화
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
		if ((Client_Sock = accept(Server_Sock, (SOCKADDR*)&clientAddr, &len)) == INVALID_SOCKET) {	  // Server Socket으로 listen하여 대기하고 있던 accept요청을
			printf("accept() failed : %d", WSAGetLastError);										  // accept 함수를 이용하여 Client_Sock에 배정하는 것
			exit(1);																				  // 실제로는 Client_Sock을 통해 통신하게 됨 새로운 socket call
		}																							  // Client_Sock에 연결을 요청한 클러이언트의 소켓주소를 반환
		send(Client_Sock, message, strlen(message) + 1, 0);
		recv(Client_Sock, buf, sizeof(buf), 0);
		printf("Client : %s\n", buf);
	}

	closesocket(Client_Sock);
	closesocket(Server_Sock);

	//printf("success! %d\n", Sock);
	
	WSACleanup(); // winsock 종료 시 반드시 WSACleanup을 호출

	return 0;
}

/*int main() {

	char a[20];

	// 1바이트마다 모두 65로 초기화
	// 배열을 채울 때는 sizeof()함수를 사용하면 됩니다.
	// sizeof 함수 - 배열의 전체 바이트 크기를 반환합니다.
	memset(a, 65, sizeof(a));

	// 출력을 통해 확인
	for (int i = 0; i < (sizeof(a) / sizeof(char)); i++) {
		printf("%c\n", a[i]);
	}

}*/