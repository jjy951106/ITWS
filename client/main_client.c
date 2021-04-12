#include "client.h"

int main(int argc, char *argv[]){

    int mode = 2 /*default mode 2*/, protocol = 0;

    int32_t thr = 5000000; // threshold default 5,000,000 ns

    struct sockaddr_in server_addr;

    char *IPbuffer;
    struct hostent *host_entry;

    memset(&server_addr, '\0', sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER /* 192.168.0.160 */);
    server_addr.sin_port = htons(PORT /* 5005 */);

    if(argc >= 2) mode = atoi(argv[1]);   

    if(argc >= 3){
        // To retrieve host information
        host_entry = gethostbyname(argv[2]);

        // To convert an Internet network
        // address into ASCII string
        IPbuffer = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));

        server_addr.sin_addr.s_addr = inet_addr(IPbuffer);
    }

    if(argc >= 4) server_addr.sin_port = htons(atoi(argv[3]));

    if(argc >= 5 && atoi(argv[4]) == 1) protocol = 1; // default 0 : UDP

    if(argc == 6) thr = atoi(argv[5]) * 1000000; // threshold default 5,000,000 ns // input ms

    if (argc > 6) {
		printf("Input exceeded\n");
		return 0;
	}

    if (protocol == 0){
        //printf("UDP client\n\n");
		UDP_socket(&server_addr, mode, protocol);
    }

	else{
        //printf("TCP client\n\n");
		TCP_socket(&server_addr, mode, protocol);
    }

    return 0;

}
