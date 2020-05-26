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
        // system time을 구하기
        GetSystemTimeAsFileTime(&ft);

        // unsigned 64 bit로 만들기
        tmpres |= ft.dwHighDateTime;
        tmpres <<= 32;
        tmpres |= ft.dwLowDateTime;

        // 100nano를 1micro로 변환하기
        tmpres /= 10;

        // epoch time으로 변환하기
        tmpres -= DELTA_EPOCH_IN_MICROSECS;

        // sec와 micorsec으로 맞추기
        tv->tv_sec = (tmpres / 1000000UL);
        tv->tv_usec = (tmpres % 1000000UL);
    }

    // timezone 처리
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
	int client_len = sizeof(clientAddr); // sizeof(client_len)이 문제였음 clientAddr 이 맞음
	int buf_len;

	char buf[1024];
	char temp[20];
	char time_[20];
	char* message = "Hellow World!";

	struct timeval tv_tod;

	//printf("%d\n", INVALID_SOCKET); // INVALID_SOCKET == SOCKET_ERROR == -1

	if (WSAStartup(MAKEWORD(2, 2), &wsaDATA) == INVALID_SOCKET) { // 첫 번째 parameter 는 사용할 소켓이 2.2 버전이라는 것을 의미
		printf("WSAStartup() failed : %d", WSAGetLastError());	  // 두 번째 parameter 는 WSADATA 구조체 변수의 주소값을 넘겨주면 해당 변수에 초기화 된 라이브러리 정보가 채워짐
		WSACleanup();
		exit(1);
	}

	if ((Server_Sock = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
		printf("socket() failed : %d", WSAGetLastError);
		exit(1);
	}

	memset(&serverAddr, 0, sizeof(serverAddr)); // serverAddr을 0으로 초기화
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

		strcpy(temp, inet_ntoa(clientAddr.sin_addr)); // IP Address를 알아내는 것

		printf("Server : %s client connected.\n", temp);

		GetTimeOfDay(&tv_tod, NULL);

		sprintf(time_, "%d.%d", tv_tod.tv_sec, tv_tod.tv_usec);

		sendto(Server_Sock, time_, strlen(time_) + 1, 0, (SOCKADDR*)&clientAddr, sizeof(clientAddr));

		printf("%d.%d\n", tv_tod.tv_sec, tv_tod.tv_usec);
	}

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