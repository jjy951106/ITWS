#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <Windows.h>
#include <process.h> // thread

#define BACKLOG 100

#define PORT 5005

#define DEFAULT_BUFLEN 512

int Thread_t = 0; // total thread number played

static void err(const char* error) {
	printf("%s failed : %d\n", error, WSAGetLastError /* errno */);
	exit(1);
}

unsigned int WINAPI Server_Socket_Thread(void* arg) {

	SOCKET sock = (SOCKET*)arg, close_, n = 0;

	struct timespec T[2];

	int32_t T_int[4]; // for compatiblility between 32bit and 64bit 

	char recv_buf[DEFAULT_BUFLEN];

	int recv_buflen = sizeof(recv_buf);

	while ((close_ = recv(sock, recv_buf, recv_buflen + 1, 0)) != SOCKET_ERROR) {

		if (close_ == 0) break; // close client socket recv return 0

		timespec_get(&T[0], TIME_UTC);

		T_int[0] = T[0].tv_sec; T_int[1] = T[0].tv_nsec;

		Sleep(1); // millisecs term

		timespec_get(&T[1], TIME_UTC);

		T_int[2] = T[1].tv_sec; T_int[3] = T[1].tv_nsec;

		send(sock, T_int, sizeof(T_int), 0);

		printf("%d\nT2: %ld", ++n, T[0].tv_sec); // time_t (long) : %ld long: %ld
		printf(".%ld\n", T[0].tv_nsec);
		printf("T3: %ld", T[1].tv_sec);
		printf(".%ld\n\n", T[1].tv_nsec);
	}

	Thread_t--; // Thread crash caution

	closesocket(sock);

	return 0;

}

int TCP_server(SOCKADDR_IN *server_addr) {

	/* thread variables */
	HANDLE h_thread[BACKLOG] = { NULL, };
	DWORD thread_id = NULL;

	SOCKADDR_IN client_addr;

	int client_len = sizeof(client_addr);

	SOCKET sock, new;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
		err("TCP socket()");

	printf("TCP socket() success\n");

	if (bind(sock, (SOCKADDR*)server_addr, sizeof(*server_addr)) == SOCKET_ERROR /* SOCKET_ERROR == -1 */)
		err("bind()");

	printf("bind() success\n");

	if (listen(sock, BACKLOG) == SOCKET_ERROR)
		err("listen()");

	printf("listen() success\n");

	while ((new = accept(sock, (SOCKADDR*)&client_addr, &client_len)) != SOCKET_ERROR) {

		printf("accept() success\n");

		if (Thread_t < 0) Thread_t = 0; // additional consideration is demanded

		if ((h_thread[0] = (HANDLE)_beginthreadex(NULL, 0, Server_Socket_Thread, (void*)new, 0, (unsigned*)&thread_id)) == 0) // if thread create is failed, return 0
			printf("thread id : %d create error\n", thread_id);

		else Thread_t++;

		printf("%d : Client IP : %s Port : %d\n", Thread_t, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
	}

	closesocket(sock); // Unlike using close in Linux, use closesocket function to close socket in Window

	return 0;
}

int UDP_server(SOCKADDR_IN* server_addr) {

	SOCKET sock;

	SOCKADDR_IN client_addr;

	int client_len = sizeof(client_addr);

	struct timespec T[2];

	int32_t T_int[4]; // for compatiblility between 32bit and 64bit 

	char recv_buf[DEFAULT_BUFLEN];

	int recv_buflen = sizeof(recv_buf);

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
		err("UDP socket()");

	printf("UDP socket() success\n");

	if (bind(sock, (SOCKADDR*)server_addr, sizeof(*server_addr)) == SOCKET_ERROR /* SOCKET_ERROR == -1 */)
		err("bind()");

	printf("bind() success\n");

	while (recvfrom(sock, recv_buf, recv_buflen, 0, (SOCKADDR*)&client_addr, &client_len) != SOCKET_ERROR) {

		printf("Client IP : %s Port : %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

		timespec_get(&T[0], TIME_UTC);

		T_int[0] = T[0].tv_sec; T_int[1] = T[0].tv_nsec;

		Sleep(1); // millisecs term

		timespec_get(&T[1], TIME_UTC);

		T_int[2] = T[1].tv_sec; T_int[3] = T[1].tv_nsec;

		sendto(sock, T_int, sizeof(T_int), 0, (SOCKADDR *)&client_addr, client_len);

		printf("T2: %ld", T[0].tv_sec); // time_t (long) : %ld long: %ld
		printf(".%ld\n", T[0].tv_nsec);
		printf("T3: %ld", T[1].tv_sec);
		printf(".%ld\n\n", T[1].tv_nsec);
	}

	closesocket(sock); // Unlike using close in Linux, use closesocket function to close socket in Window

	return 0;
}

int main(int argc, char* argv[]) {

	WSADATA wsaDATA;
	SOCKADDR_IN server_addr;

	int protocol = 0; // TCP : 0 (defalut), UDP : 1

	if (WSAStartup(MAKEWORD(2, 2), &wsaDATA) == INVALID_SOCKET /* INVALID_SOCKET == -1 */) { // firs parameter means that the version of using socket is 2.2  
		WSACleanup();
		err("WSAStartup()");
	}

	printf("TCP : 0 (default), UDP : 1\n\nport, protocol\n\n");

	/* filename port */

	memset(&server_addr, 0, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORT);

	if (argc >= 2) server_addr.sin_port = htons(atoi(argv[1]));

	if (argc == 3 && atoi(argv[2]) == 1) protocol = 1;

	if (argc > 3) {
		printf("Input exceeded\n");
		return 0;
	}

	if (protocol == 0)
		TCP_server(&server_addr);

	else
		UDP_server(&server_addr);

	WSACleanup(); // when winsock is closed, WSACleanup should be called

	return 0;
}
