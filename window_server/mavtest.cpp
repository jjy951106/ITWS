#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <Windows.h>

#include "C:\Users\junyong\Desktop\MAVLINK\include\common\mavlink.h"

using namespace std;

#define PORT 14550

#define BUFFER_LENGTH 2041

static void err(const char* error) {
	cout << error << " failed" << WSAGetLastError << endl;
	exit(1);
}

int main(void) {

	WSADATA wsaDATA;
	SOCKADDR_IN server_addr;

	int sock, len, bytes_sent;

	uint8_t buf[BUFFER_LENGTH];
	
	mavlink_message_t msg;

	if (WSAStartup(MAKEWORD(2, 2), &wsaDATA) == INVALID_SOCKET /* INVALID_SOCKET == -1 */) { // firs parameter means that the version of using socket is 2.2  
		WSACleanup();
		err("WSAStartup()");
	}

	memset(&server_addr, 0, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORT);

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
		err("UDP socket()");

	if (bind(sock, (SOCKADDR*)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR /* SOCKET_ERROR == -1 */)
		err("bind()");

	for (;;) {
		mavlink_msg_heartbeat_pack(1, 200, &msg, MAV_TYPE_HELICOPTER, MAV_AUTOPILOT_GENERIC, MAV_MODE_GUIDED_ARMED, 0, MAV_STATE_ACTIVE);
		len = mavlink_msg_to_send_buffer(buf, &msg);

		//cout << "test" << endl;
		
		bytes_sent = sendto(sock, buf, len, 0, (SOCKADDR*)&server_addr, sizeof(SOCKADDR));
	}

	return 0;
}