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

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

#define PORT 5005

struct timezone
{
    int  tz_minuteswest; /* minutes W of Greenwich */
    int  tz_dsttime;     /* type of dst correction */
};

int GetTimeOfDay(struct timeval* tv, struct timezone* tz)
{
    FILETIME ft;
    uint64_t tmpres = 0;
    static int tzflag;

    if (NULL != tv)
    {
        // system time�� ���ϱ�
        GetSystemTimeAsFileTime(&ft);

        // unsigned 64 bit�� �����
        tmpres |= ft.dwHighDateTime;
        tmpres <<= 32;
        tmpres |= ft.dwLowDateTime;

        // 100nano�� 1micro�� ��ȯ�ϱ�
        tmpres /= 10;

        // epoch time���� ��ȯ�ϱ�
        tmpres -= DELTA_EPOCH_IN_MICROSECS;

        // sec�� micorsec���� ���߱�
        tv->tv_sec = (tmpres / 1000000UL);
        tv->tv_usec = (tmpres % 1000000UL);
    }

    // timezone ó��
    if (NULL != tz)
    {
        if (!tzflag)
        {
            _tzset();
            tzflag++;
        }
        tz->tz_minuteswest = _timezone / 60;
        tz->tz_dsttime = _daylight;
    }

    return 0;
}

int main() {

	WSADATA wsaDATA;
	SOCKET Server_Sock;
	SOCKADDR_IN serverAddr, clientAddr;
	int client_len = sizeof(clientAddr); // sizeof(client_len)�� �������� clientAddr �� ����
	int buf_len;

	char buf[1024];
	char temp[20];
	char time_[20];
	char* message = "Hellow World!";

	struct timeval tv_tod;

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

		strcpy(temp, inet_ntoa(clientAddr.sin_addr)); // IP Address�� �˾Ƴ��� ��

		printf("Server : %s client connected.\n", temp);

		GetTimeOfDay(&tv_tod, NULL);

		sprintf(time_, "%d.%d", tv_tod.tv_sec, tv_tod.tv_usec);

		sendto(Server_Sock, time_, strlen(time_) + 1, 0, (SOCKADDR*)&clientAddr, sizeof(clientAddr));

		printf("%d.%d\n", tv_tod.tv_sec, tv_tod.tv_usec);
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