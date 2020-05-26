//CLIENT SOCKET
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#define PORTNUM 9005

int main(void) {
    char buf[256];
    struct sockaddr_in sin, brd;
    struct timeval tv_cli;
    int sd, n, i = 0, sinlen = sizeof(sin), broadcast = 1;
    int loc_time[1000], loc_utime[1000];

    if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){ //socket을 생성(연결 성공시 0을 리턴 실패시 -1을 리턴)
        perror("socket"); //에러메시지를 출력하는 함수
        exit(1); //1을 반환하면서 프로그램 종료
    }

    if ((setsockopt(sd , SOL_SOCKET, SO_BROADCAST, (const char *)&broadcast,sizeof(broadcast))) == -1){ 
        perror("setsockopt");
        exit(1);
    }

    memset((char *)&sin, '\0', sizeof(sin)); //socket 구조체에 값을 지정(&ser-메모리의 시작 주소, \0-메모리에 채우고자 하는 값, size-채우고자하는 메모리의 크기)
    brd.sin_family = AF_INET; //socket family를 AF_INET으로 지정
    brd.sin_port = htons(PORTNUM);
    brd.sin_addr.s_addr = htonl(INADDR_ANY); //소켓 주소 구조체에 서버의 주소를 지정

    if (bind(sd, (struct sockaddr *)&brd, sizeof(brd))) { //17행에서 생성한 소켓을 bind 함수로 22~25행에서 설정한 IP, port 번호와 연결(실패시 -1을 리턴 성공시 0을 리턴)(sd-socket에서 선언한 구조체, &ser-AF_INET의 경우 sockaddr_in AF_UNIX의 경우 sockaddr, len-선언한 구조체의 크기)
        perror("bind");
        exit(1);
    }

    strcpy(buf, "I am a client.");

//    if ((sendto(sd, buf, strlen(buf)+1, 0, (struct sockaddr *)&sin, sizeof(sin))) == -1) { //클라이언트에게 sendto 함수로 msg를 전송(sd, buf-전송할 msg를 저장한 메모리 주소, msg의 크기, 0-데이터를 주고받는 방법을 지정한 플래그, &cli-msg를 전송할 호스트의 주소, &clientlen-&cli의 크기)
//            perror("sendto");
//            exit(1);
//        }

    memset((int *)&loc_time, '\0', sizeof(loc_time));
    memset((int *)&loc_utime, '\0', sizeof(loc_utime));

    while (1) {

        n = recvfrom(sd, buf, 255, 0, (struct sockaddr *)&sin, &sinlen); //서버가 보낸 msg를 recvfrom 함수로 수신(sd, buf-전송받은 msg를 저장할 메모리 주소, msg의 크기, 0-데이터를 주고받는 방법을 지정한 플래그, msg를 보내는 서버의 주소, 주소의 크기)

        if ((ioctl(sd, SIOCGSTAMP, &tv_cli)) == -1){
            perror("ioctl");
            exit(1);
        }

        loc_time[i] = tv_cli.tv_sec;
        loc_utime[i] = tv_cli.tv_usec;
        buf[n] = '\0';

        printf("** From Server : %s, %i packet\n", buf, i+1);
        printf("** Local clock : %i.%i\n", loc_time[i], loc_utime[i]);

        i++;
    }

    return 0;
}